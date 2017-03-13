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

class GeographicMapColoring
{
 public:
    virtual ~GeographicMapColoring() {}

    virtual Color color(const hidb::AntigenData& aAntigen) const = 0;
};

// ----------------------------------------------------------------------

class ColoringByContinent : public GeographicMapColoring
{
 public:
    inline ColoringByContinent(const std::map<std::string, std::string>& aContinentColor, const LocDb& aLocDb)
        : mColors{aContinentColor.begin(), aContinentColor.end()}, mLocDb(aLocDb) {}

    virtual Color color(const hidb::AntigenData& aAntigen) const
        {
            try {
                return mColors.at(mLocDb.continent(virus_name::location(aAntigen.data().name())));
            }
            catch (...) {
                return "grey50";
            }
        }

 private:
    std::map<std::string, Color> mColors;
    const LocDb& mLocDb;

}; // class ColoringByContinent

// ----------------------------------------------------------------------

class ColoringByClade : public GeographicMapColoring
{
 public:
    inline ColoringByClade(const std::map<std::string, std::string>& aCladeColor, const seqdb::Seqdb& aSeqdb)
        : mColors{aCladeColor.begin(), aCladeColor.end()}, mSeqdb(aSeqdb) {}

    virtual Color color(const hidb::AntigenData& aAntigen) const
        {
            Color result("grey50");
            try {
                const auto* entry_seq = mSeqdb.find_hi_name(aAntigen.full_name());
                if (entry_seq) {
                    for (const auto& clade: entry_seq->seq().clades()) {
                        try {
                            result = mColors.at(clade); // find first clade that has corresponding entry in mColors and use it
                        }
                        catch (...) {
                        }
                    }
                }
            }
            catch (...) {
            }
            return result;
        }

 private:
    std::map<std::string, Color> mColors;
    const seqdb::Seqdb& mSeqdb;

}; // class ColoringByClade

// ----------------------------------------------------------------------

class ColoringByLineage : public GeographicMapColoring
{
 public:
    inline ColoringByLineage(const std::map<std::string, std::string>& aLineageColor)
        : mColors{aLineageColor.begin(), aLineageColor.end()} {}

    virtual Color color(const hidb::AntigenData& aAntigen) const
        {
            try {
                return mColors.at(aAntigen.data().lineage());
            }
            catch (...) {
                return "grey50";
            }
        }

 private:
    std::map<std::string, Color> mColors;

}; // class ColoringByLineage

// ----------------------------------------------------------------------

void GeographicMapWithPointsFromHidb::add_points_from_hidb_colored_by_continent(const std::map<std::string, std::string>& aContinentColor, std::string aStartDate, std::string aEndDate)
{
    add_points_from_hidb(ColoringByContinent(aContinentColor, mLocDb), aStartDate, aEndDate);

} // GeographicMapWithPointsFromHidb::add_points_from_hidb_colored_by_continent

// ----------------------------------------------------------------------

void GeographicMapWithPointsFromHidb::add_points_from_hidb_colored_by_clade(const std::map<std::string, std::string>& aCladeColor, const seqdb::Seqdb& aSeqdb, std::string aStartDate, std::string aEndDate)
{
    add_points_from_hidb(ColoringByClade(aCladeColor, aSeqdb), aStartDate, aEndDate);

} // GeographicMapWithPointsFromHidb::add_points_from_hidb_colored_by_clade

// ----------------------------------------------------------------------

void GeographicMapWithPointsFromHidb::add_points_from_hidb_colored_by_lineage(const std::map<std::string, std::string>& aLineageColor, std::string aStartDate, std::string aEndDate)
{
    add_points_from_hidb(ColoringByLineage(aLineageColor), aStartDate, aEndDate);

} // GeographicMapWithPointsFromHidb::add_points_from_hidb_colored_by_lineage

// ----------------------------------------------------------------------

void GeographicMapWithPointsFromHidb::add_points_from_hidb(const GeographicMapColoring& aColoring, std::string aStartDate, std::string aEndDate)
{
    auto antigens = mHiDb.all_antigens();
    antigens.date_range(aStartDate, aEndDate);
    std::cerr << "Antigens selected: " << antigens.size() << std::endl;
    for (auto& antigen: antigens) {
        mPoints.add(virus_name::location(antigen->data().name()), aColoring.color(*antigen));
    }
    std::cerr << "Locations: " << mPoints.size() << std::endl;

} // GeographicMapWithPointsFromHidb::add_points_from_hidb

// ----------------------------------------------------------------------

void GeographicTimeSeriesMonthly::draw_colored_by_continent(std::string aFilenamePrefix, const std::map<std::string, std::string>& aContinentColor, double aImageWidth)
{
    for (auto ts_iter = mTS.begin(); ts_iter != mTS.end(); ++ts_iter) {
        GeographicMapWithPointsFromHidb map = mMap;
        const Date start{*ts_iter}, end{Date{*ts_iter}.increment_month(1)};
        map.add_points_from_hidb_colored_by_continent(aContinentColor, start, end);
        map.title().add_line(start.monthtext_year());
        map.draw(aFilenamePrefix + start.year4_month2() + ".pdf", aImageWidth);
    }

} // GeographicTimeSeriesMonthly::draw_colored_by_continent

// ----------------------------------------------------------------------

void GeographicTimeSeriesMonthly::draw_colored_by_clade(std::string aFilenamePrefix, const std::map<std::string, std::string>& aCladeColor, const seqdb::Seqdb& aSeqdb, double aImageWidth)
{
    for (auto ts_iter = mTS.begin(); ts_iter != mTS.end(); ++ts_iter) {
        GeographicMapWithPointsFromHidb map = mMap;
        const Date start{*ts_iter}, end{Date{*ts_iter}.increment_month(1)};
        map.add_points_from_hidb_colored_by_clade(aCladeColor, aSeqdb, start, end);
        map.title().add_line(start.monthtext_year());
        map.draw(aFilenamePrefix + start.year4_month2() + ".pdf", aImageWidth);
    }

} // GeographicTimeSeriesMonthly::draw_colored_by_clade

// ----------------------------------------------------------------------

void GeographicTimeSeriesMonthly::draw_colored_by_lineage(std::string aFilenamePrefix, const std::map<std::string, std::string>& aLineageColor, double aImageWidth)
{
    for (auto ts_iter = mTS.begin(); ts_iter != mTS.end(); ++ts_iter) {
        GeographicMapWithPointsFromHidb map = mMap;
        const Date start{*ts_iter}, end{Date{*ts_iter}.increment_month(1)};
        map.add_points_from_hidb_colored_by_lineage(aLineageColor, start, end);
        map.title().add_line(start.monthtext_year());
        map.draw(aFilenamePrefix + start.year4_month2() + ".pdf", aImageWidth);
    }

} // GeographicTimeSeriesMonthly::draw_colored_by_lineage

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
