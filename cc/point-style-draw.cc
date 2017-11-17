#include "acmacs-base/rjson.hh"
#include "acmacs-draw/surface.hh"
#include "acmacs-map-draw/point-style-draw.hh"

// ----------------------------------------------------------------------

void PointStyleDraw::draw(Surface& aSurface, const acmacs::chart::Coordinates& aCoord) const
{
    if (*shown && aCoord.not_nan()) {
        switch (*shape) {
          case acmacs::PointShape::Circle:
              aSurface.circle_filled(aCoord, Pixels{*size}, *aspect, *rotation, *outline, *outline_width, *fill);
              break;
          case acmacs::PointShape::Box:
              aSurface.square_filled(aCoord, Pixels{*size}, *aspect, *rotation, *outline, *outline_width, *fill);
              break;
          case acmacs::PointShape::Triangle:
              aSurface.triangle_filled(aCoord, Pixels{*size}, *aspect, *rotation, *outline, *outline_width, *fill);
              break;
        }
    }

} // PointStyleDraw::draw

// ----------------------------------------------------------------------

acmacs::PointStyle point_style_from_json(const rjson::object& aSource)
{
    acmacs::PointStyle style;
    for (auto [key, value]: aSource) {
        if (key == "fill")
            style.fill = static_cast<std::string>(value);
        else if (key == "outline")
            style.outline = static_cast<std::string>(value);
        else if (key == "show")
            style.shown = value;
        else if (key == "hide")
            style.shown = !value;
        else if (key == "shape")
            style.shape = static_cast<std::string>(value);
        else if (key == "size")
            style.size = Pixels{value};
        else if (key == "outline_width")
            style.outline_width = Pixels{value};
        else if (key == "aspect")
            style.aspect = Aspect{value};
        else if (key == "rotation")
            style.rotation = Rotation{value};
    }
    return style;

} // point_style_from_json

// ----------------------------------------------------------------------

PointDrawingOrder drawing_order_from_json(const rjson::object& aSource)
{
    PointDrawingOrder result{PointDrawingOrder::NoChange};
    if (aSource.get_or_default("raise_", false)) {
        result = PointDrawingOrder::Raise;
    }
    else if (aSource.get_or_default("lower", false)) {
        result = PointDrawingOrder::Lower;
    }
    else {
        const std::string order = aSource.get_or_default("order", "");
        if (order == "raise")
            result = PointDrawingOrder::Raise;
        else if (order == "lower")
            result = PointDrawingOrder::Lower;
    }
    return result;

} // drawing_order_from_json

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
