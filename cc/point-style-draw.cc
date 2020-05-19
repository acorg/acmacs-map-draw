#include "acmacs-base/debug.hh"
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
                style.fill(Color(passage_color));
            else
                style.fill(Color(val_s));
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
                style.outline(Color(passage_color));
            else
                style.outline(Color(val_s));
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
