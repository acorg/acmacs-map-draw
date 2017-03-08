#include "acmacs-draw/surface-cairo.hh"
#include "acmacs-draw/geographic-map.hh"
#include "geographic-map.hh"

// ----------------------------------------------------------------------

void GeographicMapDraw::prepare()
{

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

} // GeographicMapDraw::draw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
