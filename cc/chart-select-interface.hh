#pragma once

#include "seqdb-3/seqdb.hh"
#include "acmacs-chart-2/chart-modify.hh"

// ----------------------------------------------------------------------

class ChartSelectInterface
{
 public:
    ChartSelectInterface(acmacs::chart::ChartModifyP aChart, size_t aProjectionNo);
    ChartSelectInterface(std::string_view chart_filename, size_t aProjectionNo);
    ChartSelectInterface(const std::vector<std::string_view>& chart_filenames, size_t aProjectionNo);

    acmacs::chart::ChartModify& chart() { return *charts_[0]; }
    const acmacs::chart::ChartModify& chart() const { return *charts_[0]; }
    auto chartp() const { return charts_[0]; }

    size_t number_of_charts() const { return charts_.size(); }
    acmacs::chart::ChartModify& chart(size_t index) { load_chart(index);  return *charts_[index]; }
    const acmacs::chart::ChartModify& chart(size_t index) const { load_chart(index);  return *charts_[index]; }
    std::string_view chart_name(size_t index) const { return chart_filenames_.at(index); }
    const acmacs::chart::ChartModify& chart(std::string_view filename) const;

    size_t number_of_antigens() const { return chart().number_of_antigens(); }
    size_t number_of_sera() const { return chart().number_of_sera(); }
    size_t number_of_points() const { return number_of_antigens() + number_of_sera(); }

      // for "found_in_previous" and "not_found_in_previous" select keys
    void previous_chart(std::string_view previous_chart_filename);
    void previous_chart(acmacs::chart::ChartP aPreviousChart) { mPreviousChart = aPreviousChart; }
    const acmacs::chart::Chart* previous_chart() const { return mPreviousChart.get(); }

    const acmacs::chart::ProjectionModify& projection() const { return *mProjectionModify; }
    acmacs::chart::ProjectionModify& projection() { return *mProjectionModify; }
    size_t projection_no() const { return projection().projection_no(); }
    acmacs::number_of_dimensions_t number_of_dimensions() const { return projection().number_of_dimensions(); }
    std::shared_ptr<acmacs::Layout> layout() const { return projection().layout(); }
    acmacs::Transformation transformation() const { return projection().transformation(); }
    std::shared_ptr<acmacs::Layout> transformed_layout() const { return projection().transformed_layout(); }
    bool point_has_coordinates(size_t point_no) const { return projection().layout()->point_has_coordinates(point_no); }

    const acmacs::chart::PlotSpecModify& plot_spec() const { return *mPlotSpec; }
    acmacs::chart::PlotSpecModify& plot_spec() { return *mPlotSpec; }
    acmacs::chart::PlotSpecModifyP plot_spec_ptr() const { return mPlotSpec; }
    acmacs::chart::DrawingOrder& drawing_order() { return plot_spec().drawing_order_modify(); }
    const acmacs::chart::DrawingOrder drawing_order() const { return plot_spec().drawing_order(); }

    const acmacs::seqdb::subset& match_seqdb() const;
    acmacs::seqdb::v3::Seqdb::aas_indexes_t aa_at_pos1_for_antigens(const std::vector<size_t>& aPositions1) const;

  private:
    mutable std::vector<std::string_view> chart_filenames_;
    mutable std::vector<acmacs::chart::ChartModifyP> charts_;
    acmacs::chart::ProjectionModifyP mProjectionModify;
    acmacs::chart::PlotSpecModifyP mPlotSpec;
    acmacs::chart::ChartP mPreviousChart;
    mutable std::optional<acmacs::seqdb::subset> matched_seqdb_;

    void load_chart(size_t index) const;

}; // class ChartSelectInterface

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
