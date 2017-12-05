#pragma once

#include <string>
#include <vector>
#include <memory>

#include "acmacs-base/throw.hh"
#include "acmacs-base/range.hh"
#include "acmacs-base/timeit.hh"
#include "acmacs-chart-2/chart-modify.hh"
#include "acmacs-draw/viewport.hh"
#include "acmacs-map-draw/point-style-draw.hh"
#include "acmacs-map-draw/map-elements.hh"
#include "acmacs-map-draw/labels.hh"

class Surface;

// ----------------------------------------------------------------------

class ChartDraw
{
 public:
    ChartDraw(acmacs::chart::ChartModifyP aChart, size_t aProjectionNo);

    void prepare();
    void draw(Surface& aSurface) const;
    void draw(std::string aFilename, double aSize, report_time aTimer = report_time::No) const;
    const acmacs::Viewport& calculate_viewport(bool verbose = true);

    inline auto& chart() { return *mChart; }
    inline const auto& chart() const { return *mChart; }
    inline std::shared_ptr<acmacs::chart::Layout> layout() const { return mProjectionModify->layout(); }
    inline size_t projection_no() const { return mProjectionModify->projection_no(); }

      // for "found_in_previous" and "not_found_in_previous" select keys
    inline void previous_chart(acmacs::chart::ChartP aPreviousChart) { mPreviousChart = aPreviousChart; }
    inline const acmacs::chart::Chart* previous_chart() const { return mPreviousChart.get(); }

    template <typename T> inline void modify_drawing_order(const T& aPoints, PointDrawingOrder aPointDrawingOrder)
        {
            switch (aPointDrawingOrder) {
              case PointDrawingOrder::Raise:
                  mPlotSpec->raise(aPoints);
                  break;
              case PointDrawingOrder::Lower:
                  mPlotSpec->lower(aPoints);
                  break;
              case PointDrawingOrder::NoChange:
                  break;
            }
        }

    template <typename T> inline void modify_sera_drawing_order(const T& aPoints, PointDrawingOrder aPointDrawingOrder)
        {
            switch (aPointDrawingOrder) {
              case PointDrawingOrder::Raise:
                  mPlotSpec->raise_serum(aPoints);
                  break;
              case PointDrawingOrder::Lower:
                  mPlotSpec->lower_serum(aPoints);
                  break;
              case PointDrawingOrder::NoChange:
                  break;
            }
        }

    inline void modify(const acmacs::chart::Indexes& aPoints, const acmacs::PointStyle& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange)
        {
            mPlotSpec->modify(aPoints, aStyle);
            modify_drawing_order(aPoints, aPointDrawingOrder);
        }

    inline void modify(size_t aPointNo, const acmacs::PointStyle& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange)
        {
            mPlotSpec->modify(aPointNo, aStyle);
            modify_drawing_order(aPointNo, aPointDrawingOrder);
        }

    inline void modify_sera(const acmacs::chart::Indexes& aSera, const acmacs::PointStyle& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange)
        {
            mPlotSpec->modify_sera(aSera, aStyle);
            modify_sera_drawing_order(aSera, aPointDrawingOrder);
        }

    inline void modify_serum(size_t aSerumNo, const acmacs::PointStyle& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange)
        {
            mPlotSpec->modify_serum(aSerumNo, aStyle);
            modify_sera_drawing_order(aSerumNo, aPointDrawingOrder);
        }

    void hide_all_except(const acmacs::chart::Indexes& aNotHide);
    void mark_egg_antigens();
    void mark_reassortant_antigens();
    void mark_all_grey(Color aColor);
    inline void scale_points(double aPointScale, double aOulineScale) { mPlotSpec->scale_all(aPointScale, aOulineScale); }
    inline void modify_all_sera(const acmacs::PointStyle& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange) { modify_sera(chart().sera()->all_indexes(), aStyle, aPointDrawingOrder); }

    inline void rotate(double aAngle)
        {
            if (!float_zero(aAngle))
                log("rotate radians:", aAngle, " degrees:", 180.0 * aAngle / M_PI, " ", aAngle > 0 ? "counter-" : "", "clockwise");
            mProjectionModify->rotate_radians(aAngle);
        }

    inline void flip(double aX, double aY)
        {
            log("flip ", aX, " ", aY);
            mProjectionModify->flip(aX, aY); // reflect about a line specified with vector [aX, aY]
        }

    inline const acmacs::Transformation transformation() const { return mProjectionModify->transformation(); }
    inline std::shared_ptr<acmacs::chart::Layout> transformed_layout() const { return mProjectionModify->transformed_layout(); }

    inline void viewport(double aX, double aY, double aSize) { mViewport.set(aX, aY, aSize); }
    inline void viewport(const acmacs::Viewport& aViewport) { mViewport = aViewport; }
    inline const acmacs::Viewport& viewport() const { return mViewport; }

    inline acmacs::chart::DrawingOrder& drawing_order() { return mPlotSpec->drawing_order_modify(); }
    inline acmacs::chart::DrawingOrder drawing_order() const { return mPlotSpec->drawing_order(); }

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
    inline map_elements::Label& add_label(size_t aIndex) { return mLabels.add(aIndex, chart()); }
    inline void remove_label(size_t aIndex) { return mLabels.remove(aIndex); }
    map_elements::SerumCircle& serum_circle(size_t aSerumNo, Scaled aRadius);
    map_elements::Line& line(const acmacs::Location& aBegin, const acmacs::Location& aEnd);
    map_elements::Arrow& arrow(const acmacs::Location& aBegin, const acmacs::Location& aEnd);
    map_elements::Point& point(const acmacs::Location& aCenter, Pixels aSize);
    map_elements::Rectangle& rectangle(const acmacs::Location& aCorner1, const acmacs::Location& aCorner2);
    map_elements::Circle& circle(const acmacs::Location& aCenter, Scaled aSize);
    void remove_serum_circles();


    inline size_t number_of_antigens() const { return chart().number_of_antigens(); }
    inline size_t number_of_sera() const { return chart().number_of_sera(); }
    inline size_t number_of_points() const { return number_of_antigens() + number_of_sera(); }

    void save(std::string aFilename, std::string aProgramName);

 private:
    acmacs::chart::ChartModifyP mChart;
    acmacs::chart::ProjectionModifyP mProjectionModify;
    acmacs::chart::PlotSpecModifyP mPlotSpec;
    acmacs::chart::ChartP mPreviousChart;
    acmacs::Viewport mViewport;
    map_elements::Elements mMapElements;
    map_elements::Labels mLabels;

}; // class ChartDraw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
