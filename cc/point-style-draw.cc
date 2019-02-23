#include "acmacs-base/rjson.hh"
#include "acmacs-draw/surface.hh"
#include "acmacs-map-draw/point-style-draw.hh"

// ----------------------------------------------------------------------

void draw_point(acmacs::surface::Surface& aSurface, const acmacs::PointStyle& aStyle, const acmacs::PointCoordinates& aCoord)
{
      // obsolete
    if (*aStyle.shown && aCoord.exists()) {
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

acmacs::PointStyle point_style_from_json(const rjson::value& aSource)
{
    acmacs::PointStyle style;
    rjson::for_each(aSource, [&style](const std::string& key, const rjson::value& val) {
        if (key == "fill")
            style.fill = Color(static_cast<std::string_view>(val));
        else if (key == "fill_saturation")
            style.fill = Color(Color::type::adjust_saturation, val);
        else if (key == "fill_brightness")
            style.fill = Color(Color::type::adjust_brightness, val);
        else if (key == "outline")
            style.outline = Color(static_cast<std::string_view>(val));
        else if (key == "outline_saturation")
            style.outline = Color(Color::type::adjust_saturation, val);
        else if (key == "outline_brightness")
            style.outline = Color(Color::type::adjust_brightness, val);
        else if (key == "show")
            style.shown = static_cast<bool>(val);
        else if (key == "hide")
            style.shown = ! static_cast<bool>(val);
        else if (key == "shape")
            style.shape = static_cast<std::string_view>(val);
        else if (key == "size")
            style.size = Pixels{val};
        else if (key == "outline_width")
            style.outline_width = Pixels{val};
        else if (key == "aspect")
            style.aspect = Aspect{val};
        else if (key == "rotation")
            style.rotation = Rotation{val};
    });
    return style;

} // point_style_from_json

// ----------------------------------------------------------------------

PointDrawingOrder drawing_order_from_json(const rjson::value& aSource)
{
    PointDrawingOrder result{PointDrawingOrder::NoChange};
    if (rjson::get_or(aSource, "raise_", false)) {
        result = PointDrawingOrder::Raise;
    }
    else if (rjson::get_or(aSource, "lower", false)) {
        result = PointDrawingOrder::Lower;
    }
    else {
        const auto order = rjson::get_or(aSource, "order", "");
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
