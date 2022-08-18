#include "acmacs-base/log.hh"
#include "acmacs-base/rjson-v2.hh"
#include "acmacs-draw/surface.hh"
#include "acmacs-map-draw/point-style-draw.hh"

// ----------------------------------------------------------------------

void draw_point(acmacs::surface::Surface& aSurface, const acmacs::PointStyle& aStyle, const acmacs::PointCoordinates& aCoord)
{
      // obsolete
    if (aStyle.shown() && aCoord.exists()) {
        switch (aStyle.shape()) {
          case acmacs::PointShape::Circle:
              aSurface.circle_filled(aCoord, aStyle.size(), aStyle.aspect(), aStyle.rotation(), aStyle.outline(), aStyle.outline_width(), acmacs::surface::Dash::NoDash, aStyle.fill());
              break;
          case acmacs::PointShape::Egg:
              aSurface.egg_filled(aCoord, aStyle.size(), aStyle.aspect(), aStyle.rotation(), aStyle.outline(), aStyle.outline_width(), acmacs::surface::Dash::NoDash, aStyle.fill());
              break;
          case acmacs::PointShape::UglyEgg:
              aSurface.ugly_egg_filled(aCoord, aStyle.size(), aStyle.aspect(), aStyle.rotation(), aStyle.outline(), aStyle.outline_width(), acmacs::surface::Dash::NoDash, aStyle.fill());
              break;
          case acmacs::PointShape::Box:
              aSurface.square_filled(aCoord, aStyle.size(), aStyle.aspect(), aStyle.rotation(), aStyle.outline(), aStyle.outline_width(), aStyle.fill());
              break;
          case acmacs::PointShape::Triangle:
              aSurface.triangle_filled(aCoord, aStyle.size(), aStyle.aspect(), aStyle.rotation(), aStyle.outline(), aStyle.outline_width(), aStyle.fill());
              break;
        }
    }

} // draw_point

// ----------------------------------------------------------------------

acmacs::PointStyleModified point_style_from_json(const rjson::value& aSource, Color passage_color)
{
    acmacs::PointStyleModified style;
    rjson::for_each(aSource, [&style,passage_color](std::string_view key, const rjson::value& val) {
        if (key == "fill") {
            if (const std::string_view val_s{val.to<std::string_view>()}; val_s == "passage")
                style.fill(acmacs::color::Modifier{Color(passage_color)});
            else
                style.fill(acmacs::color::Modifier{Color(val_s)});
        }
        else if (key == "fill_saturation") {
            AD_WARNING("\"fill_saturation\" not implemented");
            // style.fill = acmacs::color::Modifier{acmacs::color::adjust_saturation{val.to<double>()}};
        }
        else if (key == "fill_brightness") {
            AD_WARNING("\"fill_brightness not implemented");
            // style.fill = acmacs::color::Modifier{acmacs::color::adjust_brightness{val.to<double>()}};
        }
        else if (key == "outline") {
            if (const std::string_view val_s{val.to<std::string_view>()}; val_s == "passage")
                style.outline(acmacs::color::Modifier{Color(passage_color)});
            else
                style.outline(acmacs::color::Modifier{Color(val_s)});
        }
        else if (key == "outline_saturation") {
            AD_WARNING("\"outline_saturation\" not implemented");
            // style.outline = acmacs::color::Modifier{acmacs::color::adjust_saturation{val.to<double>()}};
        }
        else if (key == "outline_brightness") {
            AD_WARNING("\"outline_brightness\" not implemented");
            // style.outline = acmacs::color::Modifier{acmacs::color::adjust_brightness{val.to<double>()}};
        }
        else if (key == "show") {
            style.shown(val.to<bool>());
        }
        else if (key == "hide") {
            style.shown(! val.to<bool>());
        }
        else if (key == "shape") {
            style.shape(val.to<std::string_view>());
        }
        else if (key == "size") {
            style.size(Pixels{val.to<double>()});
        }
        else if (key == "outline_width") {
            style.outline_width(Pixels{val.to<double>()});
        }
        else if (key == "aspect") {
            style.aspect(Aspect{val.to<double>()});
        }
        else if (key == "rotation") {
            style.rotation(Rotation{val.to<double>()});
        }
    });
    return style;

} // point_style_from_json

// ----------------------------------------------------------------------

PointDrawingOrder drawing_order_from(std::string_view order)
{
    if (order == "raise")
        return PointDrawingOrder::Raise;
    if (order == "lower")
        return PointDrawingOrder::Lower;
    if (!order.empty())
        AD_WARNING("unrecognized drawing order: {}", order);
    return PointDrawingOrder::NoChange;

} // drawing_order_from

// ----------------------------------------------------------------------

PointDrawingOrder drawing_order_from(const rjson::value& aSource)
{
    if (rjson::get_or(aSource, "raise_", false))
        return PointDrawingOrder::Raise;
    if (rjson::get_or(aSource, "lower", false))
        return PointDrawingOrder::Lower;
    return drawing_order_from(rjson::get_or(aSource, "order", std::string_view{}));

} // drawing_order_from

// ----------------------------------------------------------------------
