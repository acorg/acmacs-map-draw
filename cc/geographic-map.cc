#include "acmacs-base/virus-name.hh"
#include "acmacs-base/range.hh"
#include "acmacs-draw/surface-cairo.hh"
#include "acmacs-draw/geographic-map.hh"
#include "locationdb/locdb.hh"
#include "hidb/hidb.hh"
#include "seqdb/seqdb.hh"
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
    const Size size = geographic_map_size();
    PdfCairo outline_surface(aFilename, aImageWidth, aImageWidth / size.width * size.height, size.width);
    auto& point_surface = outline_surface.subsurface(outline_surface.viewport().origin, Scaled{outline_surface.viewport().size.width}, geographic_map_viewport(), false);
    prepare(point_surface);
    draw(outline_surface, point_surface);

} // GeographicMapDraw::draw

// ----------------------------------------------------------------------

void GeographicMapDraw::add_point(double aLat, double aLong, Color aFill, Pixels aSize)
{
    mPoints.emplace_back(Coordinates{aLong, -aLat});
    mPoints.back().shape(PointStyle::Shape::Circle).fill(aFill).outline_width(Pixels{0}).size(aSize);

} // GeographicMapDraw::add_point

// ----------------------------------------------------------------------

GeographicMapColoring::~GeographicMapColoring()
{
}

// ----------------------------------------------------------------------

void GeographicMapWithPointsFromHidb::prepare(Surface& aSurface)
{
    GeographicMapDraw::prepare(aSurface);

    const double point_scaled = aSurface.convert(mPointSize).value();
    for (const auto& location_color: mPoints) {
        try {
            const auto location = mLocDb.find(location_color.first);
            const double center_lat = location.latitude(), center_long = location.longitude();
            auto iter = location_color.second.iterator();
            add_point(center_lat, center_long, *iter, mPointSize);
            ++iter;
            for (size_t circle_no = 1; iter; ++circle_no) {
                const double distance = point_scaled * mDensity * circle_no;
                const size_t circle_capacity = static_cast<size_t>(M_PI * 2.0 * distance * circle_no / (point_scaled * mDensity));
                const size_t points_on_circle = std::min(circle_capacity, iter.left());
                const double step = 2.0 * M_PI / points_on_circle;
                for (auto index = Range<size_t>::begin(points_on_circle); index != Range<size_t>::end(); ++index) {
                    add_point(center_lat + distance * std::cos(*index * step), center_long + distance * std::sin(*index * step), *iter, mPointSize);
                    ++iter;
                }
            }
        }
        catch (LocationNotFound&) {
        }
    }

} // GeographicMapWithPointsFromHidb::prepare

// ----------------------------------------------------------------------

void GeographicMapWithPointsFromHidb::add_points_from_hidb_colored_by(const GeographicMapColoring& aColoring, std::string aStartDate, std::string aEndDate)
{
    auto antigens = mHiDb.all_antigens();
    antigens.date_range(aStartDate, aEndDate);
      // std::cerr << "Antigens selected: " << antigens.size() << std::endl;
    for (auto& antigen: antigens) {
        mPoints.add(virus_name::location(antigen->data().name()), aColoring.color(*antigen));
    }
      // std::cerr << "Locations: " << mPoints.size() << std::endl;

} // GeographicMapWithPointsFromHidb::add_points_from_hidb_colored_by

// ----------------------------------------------------------------------

GeographicTimeSeriesBase::~GeographicTimeSeriesBase()
{
} // GeographicTimeSeriesBase::~GeographicTimeSeriesBase

// ----------------------------------------------------------------------

void GeographicTimeSeriesBase::draw(std::string aFilenamePrefix, TimeSeriesIterator& aBegin, const TimeSeriesIterator& aEnd, const GeographicMapColoring& aColoring, double aImageWidth) const
{
    for (; aBegin != aEnd; ++aBegin) {
        GeographicMapWithPointsFromHidb map = mMap;
        map.add_points_from_hidb_colored_by(aColoring, *aBegin, aBegin.next());
        map.title().add_line(aBegin.text_name());
        map.draw(aFilenamePrefix + aBegin.numeric_name() + ".pdf", aImageWidth);
    }

} // GeographicTimeSeriesBase::draw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
