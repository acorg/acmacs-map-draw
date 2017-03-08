#include "acmacs-draw/surface-cairo.hh"
#include "acmacs-draw/geographic-map.hh"
#include "geographic-map.hh"

// ----------------------------------------------------------------------

void GeographicMapPoints::draw(Surface& aSurface) const
{
    auto& surface = aSurface.subsurface(aSurface.viewport().origin, Scaled{aSurface.viewport().size.width}, geographic_map_viewport(), false);
    for (const auto& point: *this) {
        point.draw(surface);
    }

} // GeographicMapPoints::draw

// ----------------------------------------------------------------------

void GeographicMapDraw::prepare()
{
    add_point(0, 0, "pink", Pixels{5});
    add_point(-33.87, 151.21, "pink", Pixels{5}); // sydney
    add_point( 40.71, -74.01, "pink", Pixels{5}); // new york
    add_point( 51.51,   0.13, "pink", Pixels{5}); // london
    add_point( 36.13,  -5.37, "pink", Pixels{5}); // gibraltar
    add_point( 12.42, -71.72, "pink", Pixels{5}); // top of south america
    add_point( 34.29, 126.52, "pink", Pixels{5}); // south-west of south korea

} // GeographicMapDraw::prepare

// ----------------------------------------------------------------------

void GeographicMapDraw::draw(Surface& aSurface)
{
    geographic_map_draw(aSurface, "black", Pixels{1});

} // GeographicMapDraw::draw

// ----------------------------------------------------------------------

void GeographicMapDraw::draw(std::string aFilename)
{
    const Size size = geographic_map_size();
    PdfCairo surface(aFilename, size.width, size.height, size.width);
    draw(surface);
    mPoints.draw(surface);

} // GeographicMapDraw::draw

// ----------------------------------------------------------------------

void GeographicMapDraw::add_point(double aLat, double aLong, Color aFill, Pixels aSize)
{
    mPoints.emplace_back(Coordinates{aLong, -aLat});
    mPoints.back().shape(PointStyle::Shape::Circle).fill(aFill).outline_width(Pixels{0}).size(aSize);

} // GeographicMapDraw::add_point

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
