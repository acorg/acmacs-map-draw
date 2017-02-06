#pragma once

#include <string>
#include <vector>

#include "acmacs-draw/viewport.hh"

#include "layout.hh"
#include "point-style.hh"
#include "map-elements.hh"

class Chart;
class Surface;
class IndexGenerator;

// ----------------------------------------------------------------------

class DrawingOrder : public std::vector<size_t>
{
 public:
    DrawingOrder(Chart& aChart);

    void raise(size_t aPointNo);
    void lower(size_t aPointNo);

}; // class DrawingOrder

// ----------------------------------------------------------------------

class ChartDraw
{
 public:
    ChartDraw(Chart& aChart, size_t aProjectionNo);

    void prepare();
    void draw(Surface& aSurface);
    void draw(std::string aFilename, double aSize);

    void modify(IndexGenerator&& aGen, const PointStyle& aStyle, bool aRaise = false, bool aLower = false);

    inline void modify_point_by_index(size_t aIndex, const PointStyle& aStyle, bool aRaise = false, bool aLower = false)
        {
            mPointStyles[aIndex] = aStyle;
            if (aRaise)
                drawing_order().raise(aIndex);
            else if (aLower)
                drawing_order().lower(aIndex);
        }

    inline void modify_points_by_indices(const std::vector<size_t>& aIndices, const PointStyle& aStyle, bool aRaise = false, bool aLower = false)
        {
            for (size_t index: aIndices)
                modify_point_by_index(index, aStyle, aRaise, aLower);
        }

    void mark_egg_antigens();
    void mark_reassortant_antigens();
    void mark_all_grey(Color aColor);
    void scale_points(double aPointScale, double aOulineScale);
    inline void rotate(double aAngle) { mTransformation.rotate(aAngle); }
    inline void flip(double aX, double aY) { mTransformation.flip(aX, aY); } // reflect about a line specified with vector [aX, aY]

    DrawingOrder& drawing_order() { return mDrawingOrder; }

    inline void background_color(Color aBackgroud) { dynamic_cast<BackgroundBorderGrid&>(mMapElements["background-border-grid"]).background_color(aBackgroud); }
    inline void grid(Color aGridColor, double aGridLineWidth) { dynamic_cast<BackgroundBorderGrid&>(mMapElements["background-border-grid"]).grid(aGridColor, aGridLineWidth); }
    inline void border(Color aBorderColor, double aBorderWidth) { dynamic_cast<BackgroundBorderGrid&>(mMapElements["background-border-grid"]).border(aBorderColor, aBorderWidth); }
    inline void continent_map(const Location& aOffset, Pixels aWidth) { dynamic_cast<ContinentMap&>(mMapElements["continent-map"]).offset_width(aOffset, aWidth); }
    
 private:
    Chart& mChart;
    size_t mProjectionNo;
    Viewport mViewport;
    Transformation mTransformation;
    std::vector<PointStyle> mPointStyles;
    DrawingOrder mDrawingOrder;
    MapElements mMapElements;

}; // class ChartDraw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
