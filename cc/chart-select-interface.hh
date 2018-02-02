#pragma once

#include "acmacs-chart-2/chart-modify.hh"

// ----------------------------------------------------------------------

class ChartSelectInterface
{
 public:
    ChartSelectInterface(acmacs::chart::ChartModifyP aChart, size_t aProjectionNo)
        : mChart(aChart),
          mProjectionModify(mChart->projection_modify(aProjectionNo)),
          mPlotSpec(mChart->plot_spec_modify())
        {
        }

    auto& chart() { return *mChart; }
    const auto& chart() const { return *mChart; }
    auto chartp() const { return mChart; }

    size_t number_of_antigens() const { return chart().number_of_antigens(); }
    size_t number_of_sera() const { return chart().number_of_sera(); }
    size_t number_of_points() const { return number_of_antigens() + number_of_sera(); }

      // for "found_in_previous" and "not_found_in_previous" select keys
    void previous_chart(acmacs::chart::ChartP aPreviousChart) { mPreviousChart = aPreviousChart; }
    const acmacs::chart::Chart* previous_chart() const { return mPreviousChart.get(); }

    const acmacs::chart::ProjectionModify& projection() const { return *mProjectionModify; }
    acmacs::chart::ProjectionModify& projection() { return *mProjectionModify; }
    size_t projection_no() const { return projection().projection_no(); }
    std::shared_ptr<acmacs::chart::Layout> layout() const { return projection().layout(); }
    const acmacs::Transformation transformation() const { return projection().transformation(); }
    std::shared_ptr<acmacs::chart::Layout> transformed_layout() const { return projection().transformed_layout(); }

    const acmacs::chart::PlotSpecModify& plot_spec() const { return *mPlotSpec; }
    acmacs::chart::PlotSpecModify& plot_spec() { return *mPlotSpec; }
    acmacs::chart::DrawingOrder& drawing_order() { return plot_spec().drawing_order_modify(); }
    const acmacs::chart::DrawingOrder drawing_order() const { return plot_spec().drawing_order(); }

 private:
    acmacs::chart::ChartModifyP mChart;
    acmacs::chart::ProjectionModifyP mProjectionModify;
    acmacs::chart::PlotSpecModifyP mPlotSpec;
    acmacs::chart::ChartP mPreviousChart;

}; // class ChartSelectInterface

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
