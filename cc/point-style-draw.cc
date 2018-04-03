#include "acmacs-base/rjson.hh"
#include "acmacs-draw/surface.hh"
#include "acmacs-map-draw/point-style-draw.hh"

// ----------------------------------------------------------------------

void draw_point(acmacs::surface::Surface& aSurface, const acmacs::PointStyle& aStyle, const acmacs::Coordinates& aCoord)
{
      // obsolete
    if (*aStyle.shown && aCoord.not_nan()) {
        switch (*aStyle.shape) {
          case acmacs::PointShape::Circle:
              aSurface.circle_filled(aCoord, Pixels{*aStyle.size}, *aStyle.aspect, *aStyle.rotation, *aStyle.outline, *aStyle.outline_width, *aStyle.fill);
              break;
          case acmacs::PointShape::Box:
              aSurface.square_filled(aCoord, Pixels{*aStyle.size}, *aStyle.aspect, *aStyle.rotation, *aStyle.outline, *aStyle.outline_width, *aStyle.fill);
              break;
          case acmacs::PointShape::Triangle:
              aSurface.triangle_filled(aCoord, Pixels{*aStyle.size}, *aStyle.aspect, *aStyle.rotation, *aStyle.outline, *aStyle.outline_width, *aStyle.fill);
              break;
        }
    }

} // draw_point

// ----------------------------------------------------------------------

acmacs::PointStyle point_style_from_json(const rjson::object& aSource)
{
    acmacs::PointStyle style;
    for (auto [key, value]: aSource) {
        if (key == "fill")
            style.fill = Color(value);
        else if (key == "fill_saturation")
            style.fill = Color(Color::type::adjust_saturation, value);
        else if (key == "fill_brightness")
            style.fill = Color(Color::type::adjust_brightness, value);
        else if (key == "outline")
            style.outline = Color(value);
        else if (key == "outline_saturation")
            style.outline = Color(Color::type::adjust_saturation, value);
        else if (key == "outline_brightness")
            style.outline = Color(Color::type::adjust_brightness, value);
        else if (key == "show")
            style.shown = value;
        else if (key == "hide")
            style.shown = !value;
        else if (key == "shape")
            style.shape = value.str();
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
