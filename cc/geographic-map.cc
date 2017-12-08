#include "acmacs-base/virus-name.hh"
#include "acmacs-base/range.hh"
#include "acmacs-draw/surface-cairo.hh"
#include "acmacs-draw/geographic-map.hh"
#include "geographic-map.hh"

// ----------------------------------------------------------------------

void GeographicMapPoints::draw(Surface& aSurface) const
{
    for (const auto& point: *this) {
        point.draw(aSurface);
    }

} // GeographicMapPoints::draw

// ----------------------------------------------------------------------

GeographicMapDraw::~GeographicMapDraw()
{
} // GeographicMapDraw::~GeographicMapDraw

// ----------------------------------------------------------------------

void GeographicMapDraw::prepare(Surface&)
{
} // GeographicMapDraw::prepare

// ----------------------------------------------------------------------

void GeographicMapDraw::draw(Surface& aOutlineSurface, Surface& aPointSurface) const
{
    geographic_map_draw(aOutlineSurface, mOutline, mOutlineWidth);
    mPoints.draw(aPointSurface);
    mTitle.draw(aOutlineSurface);

} // GeographicMapDraw::draw

// ----------------------------------------------------------------------

void GeographicMapDraw::draw(std::string aFilename, double aImageWidth)
{
    const acmacs::Size size = geographic_map_size();
    PdfCairo outline_surface(aFilename, aImageWidth, aImageWidth / size.width * size.height, size.width);
    auto& point_surface = outline_surface.subsurface(outline_surface.viewport().origin, Scaled{outline_surface.viewport().size.width}, geographic_map_viewport(), false);
    prepare(point_surface);
    draw(outline_surface, point_surface);

} // GeographicMapDraw::draw

// ----------------------------------------------------------------------

void GeographicMapDraw::add_point(long aPriority, double aLat, double aLong, Color aFill, Pixels aSize, Color aOutline, Pixels aOutlineWidth)
{
    mPoints.emplace_back(LongLat{aLong, -aLat}, aPriority);
    auto& style = mPoints.back();
    style.shape = acmacs::PointShape::Circle;
    style.fill = aFill;
    style.outline = aOutline;
    style.outline_width = aOutlineWidth;
    style.size = aSize;

} // GeographicMapDraw::add_point

// ----------------------------------------------------------------------

GeographicMapColoring::~GeographicMapColoring()
{
}

// ----------------------------------------------------------------------

ColorOverride::TagColor ColoringByClade::color(const hidb::Antigen& aAntigen) const
{
    ColoringData result("grey50");
    std::string tag{"UNKNOWN"};
    try {
        const auto* entry_seq = seqdb::get(report_time::Yes).find_hi_name(aAntigen.full_name());
        if (entry_seq) {
            for (const auto& clade: entry_seq->seq().clades()) {
                try {
                    result = mColors.at(clade); // find first clade that has corresponding entry in mColors and use it
                    tag = clade;
                    break;
                }
                catch (...) {
                }
            }
        }
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: ColoringByClade " << aAntigen.full_name() << ": " << err.what() << '\n';
    }
    catch (...) {
    }
      // std::cerr << "INFO: ColoringByClade " << aAntigen.full_name() << ": " << tag << '\n';
    return {tag, result};

} // ColoringByClade::color

// ----------------------------------------------------------------------

void GeographicMapWithPointsFromHidb::prepare(Surface& aSurface)
{
    GeographicMapDraw::prepare(aSurface);

    const double point_scaled = aSurface.convert(mPointSize).value();
    for (const auto& location_color: mPointsAtLocation) {
        try {
            const auto location = get_locdb().find(location_color.first);
            const double center_lat = location.latitude(), center_long = location.longitude();
            auto iter = location_color.second.iterator();
            auto [coloring_data, priority] = *iter;
            add_point(priority, center_lat, center_long, coloring_data.fill, mPointSize, coloring_data.outline, coloring_data.outline_width);
            ++iter;
            for (size_t circle_no = 1; iter; ++circle_no) {
                const double distance = point_scaled * mDensity * circle_no;
                const size_t circle_capacity = static_cast<size_t>(M_PI * 2.0 * distance * circle_no / (point_scaled * mDensity));
                const size_t points_on_circle = std::min(circle_capacity, iter.left());
                const double step = 2.0 * M_PI / points_on_circle;
                for (auto index: acmacs::incrementer(0UL, points_on_circle)) {
                    std::tie(coloring_data, priority) = *iter;
                    add_point(priority, center_lat + distance * std::cos(index * step), center_long + distance * std::sin(index * step), coloring_data.fill, mPointSize, coloring_data.outline, coloring_data.outline_width);
                    ++iter;
                }
            }
        }
        catch (LocationNotFound&) {
        }
    }

    sort_points();

} // GeographicMapWithPointsFromHidb::prepare

// ----------------------------------------------------------------------

void GeographicMapWithPointsFromHidb::add_points_from_hidb_colored_by(const GeographicMapColoring& aColoring, const ColorOverride& aColorOverride, const std::vector<std::string>& aPriority, std::string aStartDate, std::string aEndDate)
{
      // std::cerr << "add_points_from_hidb_colored_by" << '\n';
    const auto& hidb = hidb::get(mVirusType);
    auto antigens = hidb.antigens()->date_range(aStartDate, aEndDate);
    std::cerr << "INFO: dates: " << aStartDate << ".." << aEndDate << "  antigens: " << antigens.size() << std::endl;
    if (!aPriority.empty())
        std::cerr << "INFO: priority: " << aPriority << " (the last in this list to be drawn on top of others)\n";
    for (auto antigen: antigens) {
        auto [tag, coloring_data] = aColorOverride.color(*antigen);
        if (coloring_data.fill.empty())
            std::tie(tag, coloring_data) = aColoring.color(*antigen);
        try {
            auto location = virus_name::location(antigen->name());
            if (location == "GEORGIA" && hidb.tables()->most_recent(antigen->tables())->lab() == "CDC")
                location = "GEORGIA STATE"; // somehow disambiguate
            const auto found = std::find(std::begin(aPriority), std::end(aPriority), tag);
            mPointsAtLocation.add(location, found == std::end(aPriority) ? 0 : (found - std::begin(aPriority) + 1), coloring_data);
        }
        catch (virus_name::Unrecognized&) {
        }
    }
      // std::transform(mPoints.begin(), mPoints.end(), std::ostream_iterator<std::string>(std::cerr, "\n"), [](const auto& e) -> std::string { return e.first; });

} // GeographicMapWithPointsFromHidb::add_points_from_hidb_colored_by

// ----------------------------------------------------------------------

GeographicTimeSeriesBase::~GeographicTimeSeriesBase()
{
} // GeographicTimeSeriesBase::~GeographicTimeSeriesBase

// ----------------------------------------------------------------------

void GeographicTimeSeriesBase::draw(std::string aFilenamePrefix, TimeSeriesIterator& aBegin, const TimeSeriesIterator& aEnd, const std::vector<std::string>& aPriority, const GeographicMapColoring& aColoring, const ColorOverride& aColorOverride, double aImageWidth) const
{
    for (; aBegin != aEnd; ++aBegin) {
        auto map = mMap;        // make a copy!
        map.add_points_from_hidb_colored_by(aColoring, aColorOverride, aPriority, *aBegin, aBegin.next());
        map.title().add_line(aBegin.text_name());
        map.draw(aFilenamePrefix + aBegin.numeric_name() + ".pdf", aImageWidth);
    }

} // GeographicTimeSeriesBase::draw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
