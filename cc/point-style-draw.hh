#pragma once

#include "acmacs-chart/point-style.hh"

// ----------------------------------------------------------------------

class Surface;

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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
