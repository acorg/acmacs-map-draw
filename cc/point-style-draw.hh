#pragma once

#include "acmacs-chart/point-style.hh"

// ----------------------------------------------------------------------

class Surface;
namespace rjson { class object; }

// ----------------------------------------------------------------------

class PointStyleDraw : public PointStyle
{
 public:
    inline PointStyleDraw() : PointStyle() {}
    inline PointStyleDraw(enum Empty e) : PointStyle(e) {}
    inline PointStyleDraw& operator = (const PointStyle& aPS) { PointStyle::operator=(aPS); return *this; }

    void draw(Surface& aSurface, const Coordinates& aCoord) const;

}; // class PointStyleDraw

// ----------------------------------------------------------------------

PointStyle point_style_from_json(const rjson::object& aSource);

// ----------------------------------------------------------------------

enum class PointDrawingOrder { NoChange, Raise, Lower };

PointDrawingOrder drawing_order_from_json(const rjson::object& aSource);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
