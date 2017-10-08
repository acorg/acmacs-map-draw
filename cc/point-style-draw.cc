#include "acmacs-base/rjson.hh"
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

PointStyle point_style_from_json(const rjson::object& aSource)
{
    auto style{PointStyleEmpty()};
    for (auto [key, value]: aSource) {
        if (key == "fill")
            style.fill(value);
        else if (key == "outline")
            style.outline(value);
        else if (key == "show")
            style.show(value ? PointStyle::Shown::Shown : PointStyle::Shown::Hidden);
        else if (key == "hide")
            style.show(value ? PointStyle::Shown::Hidden : PointStyle::Shown::Shown);
        else if (key == "shape")
            style.shape(value);
        else if (key == "size")
            style.size(Pixels{value});
        else if (key == "outline_width")
            style.outline_width(Pixels{value});
        else if (key == "aspect")
            style.aspect(value);
        else if (key == "rotation")
            style.rotation(value);
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
