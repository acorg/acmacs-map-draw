#include "acmacs-draw/surface.hh"
#include "acmacs-map-draw/point-style-draw.hh"

// ----------------------------------------------------------------------

void PointStyleDraw::draw(Surface& aSurface, const Coordinates& aCoord) const
{
    if (shown_raw() == Shown::Shown && !aCoord.empty()) {
        switch (shape()) {
          case Shape::NoChange:
              THROW_OR_CERR(std::runtime_error("Invalid point shape NoChange"));
          case Shape::Circle:
              aSurface.circle_filled(aCoord, size(), aspect(), rotation(), outline_raw(), outline_width(), fill_raw());
              break;
          case Shape::Box:
              aSurface.square_filled(aCoord, size(), aspect(), rotation(), outline_raw(), outline_width(), fill_raw());
              break;
          case Shape::Triangle:
              aSurface.triangle_filled(aCoord, size(), aspect(), rotation(), outline_raw(), outline_width(), fill_raw());
              break;
        }
    }
    else if (shown_raw() == Shown::NoChange)
        THROW_OR_CERR(std::runtime_error("Invalid shown value NoChange"));

} // PointStyleDraw::draw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
