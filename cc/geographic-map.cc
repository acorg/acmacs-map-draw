#include "acmacs-base/virus-name.hh"
#include "acmacs-base/range.hh"
#include "acmacs-draw/surface-cairo.hh"
#include "acmacs-draw/geographic-map.hh"
#include "locationdb/locdb.hh"
#include "hidb/hidb.hh"
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
    // add_point(0, 0, "pink", Pixels{5});
    // add_point(-33.87, 151.21, "pink", Pixels{5}); // sydney
    // add_point( 40.71, -74.01, "pink", Pixels{5}); // new york
    // add_point( 51.51,   0.13, "pink", Pixels{5}); // london
    // add_point( 36.13,  -5.37, "pink", Pixels{5}); // gibraltar
    // add_point( 12.42, -71.72, "pink", Pixels{5}); // top of south america
    // add_point( 34.29, 126.52, "pink", Pixels{5}); // south-west of south korea

} // GeographicMapDraw::prepare

// ----------------------------------------------------------------------

void GeographicMapDraw::draw(Surface& aOutlineSurface, Surface& aPointSurface) const
{
    geographic_map_draw(aOutlineSurface, "black", Pixels{1});
    mPoints.draw(aPointSurface);

} // GeographicMapDraw::draw

// ----------------------------------------------------------------------

void GeographicMapDraw::draw(std::string aFilename)
{
    const Size size = geographic_map_size();
    PdfCairo outline_surface(aFilename, size.width, size.height, size.width);
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
};


// ----------------------------------------------------------------------


void GeographicMapWithPointsFromHidb::add_points_from_hidb_colored_by_continent(const std::map<std::string, std::string>& aContinentColor, std::string aStartDate, std::string aEndDate)
{
    add_points_from_hidb(ColoringByContinent(aContinentColor, mLocDb), aStartDate, aEndDate);

} // GeographicMapWithPointsFromHidb::add_points_from_hidb_colored_by_continent

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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
