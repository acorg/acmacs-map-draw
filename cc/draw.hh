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
    inline DrawingOrder& operator=(const std::vector<size_t>& aSource) { std::vector<size_t>::operator=(aSource); return *this; }

    void raise(size_t aPointNo);
    void lower(size_t aPointNo);

}; // class DrawingOrder

// ----------------------------------------------------------------------

class ChartDraw
{
 public:
    ChartDraw(Chart_Type& aChart, size_t aProjectionNo);

    void prepare();
    void draw(Surface& aSurface) const;
#ifdef ACMACS_TARGET_OS
    void draw(std::string aFilename, double aSize, report_time aTimer = report_time::No) const;
#endif
    const acmacs::Viewport& calculate_viewport(bool verbose = true);

    inline const std::vector<PointStyleDraw>& point_styles() const { return mPointStyles; }
    inline std::vector<PointStyle> point_styles_base() const { std::vector<PointStyle> ps{mPointStyles.begin(), mPointStyles.end()}; return ps; }
    inline auto& chart() { return mChart; }
    inline const auto& chart() const { return mChart; }
    inline auto projection_no() const { return mProjectionNo; }
    inline const auto& layout() const { return chart().projection(projection_no()).layout(); }
    inline auto& layout() { return chart().projection(projection_no()).layout(); }

      // for "found_in_previous" and "not_found_in_previous" select keys
    inline void previous_chart(Chart_Type& aPreviousChart) { mPreviousChart = &aPreviousChart; }
    inline const auto& previous_chart() const { return mPreviousChart; }

    template <typename index_type> inline void modify(index_type aIndex, const PointStyle& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange)
        {
            const auto index = static_cast<size_t>(aIndex);
            mPointStyles.at(index) = aStyle;
              // std::cerr << "DEBUG: modified " << index << ' ' << mPointStyles[index] << '\n';
            switch (aPointDrawingOrder) {
              case PointDrawingOrder::Raise:
                  drawing_order().raise(index);
                  break;
              case PointDrawingOrder::Lower:
                  drawing_order().lower(index);
                  break;
              case PointDrawingOrder::NoChange:
                  break;
            }
        }

    template <typename index_type> inline void modify_serum(index_type aSerumNo, const PointStyle& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange)
        {
            modify(static_cast<size_t>(aSerumNo) + number_of_antigens(), aStyle, aPointDrawingOrder);
        }

    template <typename IndexIterator> inline void modify(IndexIterator first, IndexIterator last, const PointStyle& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange)
        {
            for (; first != last; ++first)
                modify(*first, aStyle, aPointDrawingOrder);
        }

    inline void modify(acmacs::IndexGenerator&& aGen, const PointStyle& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange)
        {
            for (auto index: aGen)
                modify(index, aStyle, aPointDrawingOrder);
        }

    template <typename IndexIterator> inline void modify_sera(IndexIterator first, IndexIterator last, const PointStyle& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange)
        {
            for (; first != last; ++first)
                modify_serum(*first, aStyle, aPointDrawingOrder);
        }

    void hide_all_except(const std::vector<size_t>& aNotHide);
    void mark_egg_antigens();
    void mark_reassortant_antigens();
    void mark_all_grey(Color aColor);
    void scale_points(double aPointScale, double aOulineScale);
    void modify_all_sera(const PointStyle& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange);

    inline void rotate(double aAngle)
        {
            if (!float_zero(aAngle))
                log("rotate radians:", aAngle, " degrees:", 180.0 * aAngle / M_PI, " ", aAngle > 0 ? "counter-" : "", "clockwise");
            mTransformation.rotate(aAngle);
        }

    inline void flip(double aX, double aY)
        {
            // std::cout << "INFO: flip " << aX << " " << aY << std::endl;
            log("flip ", aX, " ", aY);
            mTransformation.flip(aX, aY); // reflect about a line specified with vector [aX, aY]
        }
    inline const acmacs::Transformation& transformation() const { return mTransformation; }

    inline void viewport(double aX, double aY, double aSize) { mViewport.set(aX, aY, aSize); }
    inline void viewport(const acmacs::Viewport& aViewport) { mViewport = aViewport; }
    inline const acmacs::Viewport& viewport() const { return mViewport; }

    DrawingOrder& drawing_order() { return mDrawingOrder; }

    inline void background_color(Color aBackgroud) { DYNAMIC_CAST(map_elements::BackgroundBorderGrid&, (mMapElements["background-border-grid"])).background_color(aBackgroud); }
    inline void grid(Color aGridColor, double aGridLineWidth) { DYNAMIC_CAST(map_elements::BackgroundBorderGrid&, (mMapElements["background-border-grid"])).grid(aGridColor, aGridLineWidth); }
    inline void border(Color aBorderColor, double aBorderWidth) { DYNAMIC_CAST(map_elements::BackgroundBorderGrid&, (mMapElements["background-border-grid"])).border(aBorderColor, aBorderWidth); }
    inline void continent_map(const acmacs::Location& aOffset, Pixels aWidth) { DYNAMIC_CAST(map_elements::ContinentMap&, (mMapElements["continent-map"])).offset_width(aOffset, aWidth); }
    inline map_elements::LegendPointLabel& legend(const acmacs::Location& aOffset) { auto& legend = DYNAMIC_CAST(map_elements::LegendPointLabel&, (mMapElements["legend-point-label"])); legend.offset(aOffset); return legend; }
    inline map_elements::LegendPointLabel& legend() { return DYNAMIC_CAST(map_elements::LegendPointLabel&, (mMapElements["legend-point-label"])); }
    inline void remove_legend() { mMapElements.remove("legend-point-label"); }
    inline map_elements::Title& title(const acmacs::Location& aOffset) { auto& title = DYNAMIC_CAST(map_elements::Title&, (mMapElements["title"])); title.offset(aOffset); return title; }
    inline map_elements::Title& title() { return DYNAMIC_CAST(map_elements::Title&, (mMapElements["title"])); }
    inline map_elements::Labels& labels() { return mLabels; }
    inline map_elements::Label& add_label(size_t aIndex) { return mLabels.add(aIndex, mChart); }
    inline void remove_label(size_t aIndex) { return mLabels.remove(aIndex); }
    map_elements::SerumCircle& serum_circle(size_t aSerumNo, Scaled aRadius);
    map_elements::Line& line(const acmacs::Location& aBegin, const acmacs::Location& aEnd);
    map_elements::Arrow& arrow(const acmacs::Location& aBegin, const acmacs::Location& aEnd);
    map_elements::Point& point(const acmacs::Location& aCenter, Pixels aSize);
    map_elements::Rectangle& rectangle(const acmacs::Location& aCorner1, const acmacs::Location& aCorner2);
    map_elements::Circle& circle(const acmacs::Location& aCenter, Scaled aSize);
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
    inline size_t number_of_points() const { return number_of_antigens() + number_of_sera(); }

    void export_ace(std::string aFilename);
    void export_lispmds(std::string aFilename);

 private:
    Chart_Type& mChart;
    Chart_Type* mPreviousChart = nullptr;
    size_t mProjectionNo;
    acmacs::Viewport mViewport;
    acmacs::Transformation mTransformation;
    std::vector<PointStyleDraw> mPointStyles;
    DrawingOrder mDrawingOrder;
    map_elements::Elements mMapElements;
    map_elements::Labels mLabels;
    mutable std::unique_ptr<LayoutBase> mTransformedLayout;

}; // class ChartDraw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
