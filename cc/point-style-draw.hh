#pragma once

#include "acmacs-base/point-style.hh"
#include "acmacs-map-draw/coordinates.hh"

// ----------------------------------------------------------------------

class Surface;
namespace rjson { class object; }

// ----------------------------------------------------------------------

class PointStyleDraw : public acmacs::PointStyle
{
 public:
    inline PointStyleDraw() = default;
    inline PointStyleDraw& operator = (const acmacs::PointStyle& aPS) { acmacs::PointStyle::operator=(aPS); return *this; }

    void draw(Surface& aSurface, const acmacs::Coordinates& aCoord) const;

}; // class PointStyleDraw

// ----------------------------------------------------------------------

acmacs::PointStyle point_style_from_json(const rjson::object& aSource);

// ----------------------------------------------------------------------

enum class PointDrawingOrder { NoChange, Raise, Lower };

PointDrawingOrder drawing_order_from_json(const rjson::object& aSource);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
