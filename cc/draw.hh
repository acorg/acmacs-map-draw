#pragma once

#include <string>
#include <vector>
#include <memory>

#include "acmacs-base/throw.hh"
#include "acmacs-base/range.hh"
#ifdef ACMACS_TARGET_OS
#include "acmacs-base/timeit.hh"
#include "acmacs-chart/chart.hh"
using Chart_Type = Chart;
#else
#include "acmacs-chart/chart-base.hh"
using Chart_Type = ChartBase;
#endif
#include "acmacs-draw/viewport.hh"
#include "acmacs-map-draw/point-style-draw.hh"
#include "acmacs-map-draw/map-elements.hh"
#include "acmacs-map-draw/labels.hh"

class Surface;

// ----------------------------------------------------------------------

class DrawingOrder : public std::vector<size_t>
{
 public:
    DrawingOrder(Chart_Type& aChart);

    void raise(size_t aPointNo);
    void lower(size_t aPointNo);

}; // class DrawingOrder

// ----------------------------------------------------------------------

class ChartDraw
{
 public:
    enum RaiseLower { NoOrderChange, Raise, Lower };

    ChartDraw(Chart_Type& aChart, size_t aProjectionNo);

    void prepare();
    void draw(Surface& aSurface) const;
#ifdef ACMACS_TARGET_OS
    void draw(std::string aFilename, double aSize, report_time aTimer = report_time::No) const;
#endif
    void calculate_viewport();

    inline const std::vector<PointStyleDraw>& point_styles() const { return mPointStyles; }
    inline std::vector<PointStyle> point_styles_base() const { std::vector<PointStyle> ps{mPointStyles.begin(), mPointStyles.end()}; return ps; }
    inline auto& chart() { return mChart; }
    inline const auto& chart() const { return mChart; }

    template <typename index_type> inline void modify(index_type aIndex, const PointStyle& aStyle, RaiseLower aRaiseLower = NoOrderChange)
        {
            const auto index = static_cast<size_t>(aIndex);
            mPointStyles[index] = aStyle;
            switch (aRaiseLower) {
              case Raise:
                  drawing_order().raise(index);
                  break;
              case Lower:
                  drawing_order().lower(index);
                  break;
              case NoOrderChange:
                  break;
            }
        }

    template <typename index_type> inline void modify_serum(index_type aSerumNo, const PointStyle& aStyle, RaiseLower aRaiseLower = NoOrderChange)
        {
            modify(static_cast<size_t>(aSerumNo) + number_of_antigens(), aStyle, aRaiseLower);
        }

    template <typename IndexIterator> inline void modify(IndexIterator first, IndexIterator last, const PointStyle& aStyle, RaiseLower aRaiseLower = NoOrderChange)
        {
            for (; first != last; ++first)
                modify(*first, aStyle, aRaiseLower);
        }

    inline void modify(IndexGenerator&& aGen, const PointStyle& aStyle, RaiseLower aRaiseLower = NoOrderChange)
        {
            for (auto index: aGen)
                modify(index, aStyle, aRaiseLower);
        }

    template <typename IndexIterator> inline void modify_sera(IndexIterator first, IndexIterator last, const PointStyle& aStyle, RaiseLower aRaiseLower = NoOrderChange)
        {
            for (; first != last; ++first)
                modify_serum(*first, aStyle, aRaiseLower);
        }

    void hide_all_except(const std::vector<size_t>& aNotHide);
    void mark_egg_antigens();
    void mark_reassortant_antigens();
    void mark_all_grey(Color aColor);
    void scale_points(double aPointScale, double aOulineScale);
    void modify_all_sera(const PointStyle& aStyle, RaiseLower aRaiseLower = NoOrderChange);

    inline void rotate(double aAngle)
        {
            // std::cout << "INFO: rotate " << aAngle << " radians = " << (180.0 * aAngle / M_PI) << " degrees" << std::endl;
            log("rotate ", aAngle, " radians = ", 180.0 * aAngle / M_PI, " degrees");
            mTransformation.rotate(aAngle);
        }

    inline void flip(double aX, double aY)
        {
            // std::cout << "INFO: flip " << aX << " " << aY << std::endl;
            log("flip ", aX, " ", aY);
            mTransformation.flip(aX, aY); // reflect about a line specified with vector [aX, aY]
        }
    inline const Transformation& transformation() const { return mTransformation; }

    inline void viewport(double aX, double aY, double aSize) { mViewport.set(aX, aY, aSize); }
    inline void viewport(const Viewport& aViewport) { mViewport = aViewport; }
    inline const Viewport& viewport() const { return mViewport; }

    DrawingOrder& drawing_order() { return mDrawingOrder; }

    inline void background_color(Color aBackgroud) { DYNAMIC_CAST(BackgroundBorderGrid&, (mMapElements["background-border-grid"])).background_color(aBackgroud); }
    inline void grid(Color aGridColor, double aGridLineWidth) { DYNAMIC_CAST(BackgroundBorderGrid&, (mMapElements["background-border-grid"])).grid(aGridColor, aGridLineWidth); }
    inline void border(Color aBorderColor, double aBorderWidth) { DYNAMIC_CAST(BackgroundBorderGrid&, (mMapElements["background-border-grid"])).border(aBorderColor, aBorderWidth); }
    inline void continent_map(const Location& aOffset, Pixels aWidth) { DYNAMIC_CAST(ContinentMap&, (mMapElements["continent-map"])).offset_width(aOffset, aWidth); }
    inline LegendPointLabel& legend(const Location& aOffset) { auto& legend = DYNAMIC_CAST(LegendPointLabel&, (mMapElements["legend-point-label"])); legend.offset(aOffset); return legend; }
    inline LegendPointLabel& legend() { return DYNAMIC_CAST(LegendPointLabel&, (mMapElements["legend-point-label"])); }
    inline void remove_legend() { mMapElements.remove("legend-point-label"); }
    inline Title& title(const Location& aOffset) { auto& title = DYNAMIC_CAST(Title&, (mMapElements["title"])); title.offset(aOffset); return title; }
    inline Title& title() { return DYNAMIC_CAST(Title&, (mMapElements["title"])); }
    inline Labels& labels() { return mLabels; }
    inline Label& add_label(size_t aIndex) { return mLabels.add(aIndex, mChart); }
    inline void remove_label(size_t aIndex) { return mLabels.remove(aIndex); }
    SerumCircle& serum_circle(size_t aSerumNo, Scaled aRadius);
    Arrow& arrow(const Location& aBegin, const Location& aEnd);
    Point& point(const Location& aCenter, Pixels aSize);
    void remove_serum_circles();

    inline const LayoutBase& transformed_layout() const
        {
            if (!mTransformedLayout || mTransformedLayout->empty()) {
                mTransformedLayout = std::unique_ptr<LayoutBase>(mChart.projection(mProjectionNo).layout().clone());
                mTransformedLayout->transform(mTransformation);
            }
            return *mTransformedLayout;
        }

    size_t number_of_antigens() const;
    size_t number_of_sera() const;

 private:
    Chart_Type& mChart;
    size_t mProjectionNo;
    Viewport mViewport;
    Transformation mTransformation;
    std::vector<PointStyleDraw> mPointStyles;
    DrawingOrder mDrawingOrder;
    MapElements mMapElements;
    Labels mLabels;
    mutable std::unique_ptr<LayoutBase> mTransformedLayout;

}; // class ChartDraw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
