#pragma once

#include "seqdb-3/seqdb.hh"
#include "acmacs-chart-2/chart-modify.hh"

// ----------------------------------------------------------------------

class ChartAccess
{
  public:
    ChartAccess(std::string_view filename, size_t projection_no) : filename_{filename}, projection_no_{projection_no} {}
    ChartAccess(acmacs::chart::ChartP chart, size_t projection_no) : projection_no_{projection_no}, original_{chart} {}
    void reset();

    constexpr std::string_view filename() const { return filename_; }

    const acmacs::chart::Chart& chart() const { chart_access();  return *original_; }
    size_t number_of_antigens() const { return chart().number_of_antigens(); }
    size_t number_of_sera() const { return chart().number_of_sera(); }
    size_t number_of_points() const { return number_of_antigens() + number_of_sera(); }
    constexpr size_t projection_no() const { return projection_no_; }

    acmacs::chart::ChartModify& modified_chart() { modified_chart_access();  return *modified_; }
    const acmacs::chart::ChartModify& modified_chart() const { modified_chart_access();  return *modified_; }
    acmacs::chart::ChartModifyP modified_chartp() const { modified_chart_access();  return modified_; }

    const acmacs::chart::ProjectionModify& modified_projection() const { modified_projection_access(); return *modified_projection_; }
    acmacs::chart::ProjectionModify& modified_projection() { modified_projection_access(); return *modified_projection_; }
    acmacs::number_of_dimensions_t number_of_dimensions() const { return modified_projection().number_of_dimensions(); }
    std::shared_ptr<acmacs::Layout> modified_layout() const { return modified_projection().layout(); }
    std::shared_ptr<acmacs::Layout> modified_transformed_layout() const { return modified_projection().transformed_layout(); }
    bool modified_point_has_coordinates(size_t point_no) const { return modified_projection().layout()->point_has_coordinates(point_no); }

    acmacs::Transformation modified_transformation() const { return modified_projection().transformation(); }
    const acmacs::Transformation& modified_inverted_transformation() const
    {
        if (!inverted_transformation_)
            inverted_transformation_ = modified_transformation().inverse();
        return *inverted_transformation_;
    }

    const acmacs::chart::PlotSpecModify& modified_plot_spec() const { modified_plot_spec_access(); return *modified_plot_spec_; }
    acmacs::chart::PlotSpecModify& modified_plot_spec() { modified_plot_spec_access(); return *modified_plot_spec_; }
    acmacs::chart::PlotSpecModifyP modified_plot_spec_ptr() const { modified_plot_spec_access(); return modified_plot_spec_; }
    acmacs::chart::DrawingOrder& modified_drawing_order() { return modified_plot_spec().drawing_order_modify(); }
    const acmacs::chart::DrawingOrder modified_drawing_order() const { return modified_plot_spec().drawing_order(); }

    const acmacs::seqdb::subset& match_seqdb() const;
    acmacs::seqdb::v3::Seqdb::aas_indexes_t aa_at_pos1_for_antigens(const std::vector<size_t>& aPositions1) const;

  private:
    const std::string_view filename_;
    const size_t projection_no_;
    mutable acmacs::chart::ChartP original_;
    mutable acmacs::chart::ChartModifyP modified_;
    mutable acmacs::chart::ProjectionModifyP modified_projection_;
    mutable std::optional<acmacs::Transformation> inverted_transformation_;
    mutable acmacs::chart::PlotSpecModifyP modified_plot_spec_;
    mutable std::optional<acmacs::seqdb::subset> matched_seqdb_;

    bool chart_access() const;
    bool modified_chart_access() const;
    bool modified_projection_access() const;
    bool modified_plot_spec_access() const;

}; // class ChartAccess

// ----------------------------------------------------------------------

class ChartSelectInterface
{
 public:
    ChartSelectInterface(std::string_view filename, size_t projection_no) : charts_{ChartAccess{filename, projection_no}} {}
    ChartSelectInterface(const std::vector<std::string_view>& filenames, size_t projection_no);
    ChartSelectInterface(acmacs::chart::ChartP chart, size_t projection_no) : charts_{ChartAccess{chart, projection_no}} {}

    size_t number_of_charts() const { return charts_.size(); }
    ChartAccess& chart(size_t index);
    const ChartAccess& chart(size_t index) const;
    ChartAccess& chart(std::string_view filename, size_t projection_no = 0);
    void reset() { std::for_each(std::begin(charts_), std::end(charts_), [](auto& en) { en.reset(); }); }

    const acmacs::chart::ChartModify& chart() const { return chart(0).modified_chart(); }
    acmacs::chart::ChartModify& chart() { return chart(0).modified_chart(); }
    const acmacs::chart::PlotSpecModify& plot_spec() const { return chart(0).modified_plot_spec(); }
    acmacs::chart::PlotSpecModify& plot_spec() { return chart(0).modified_plot_spec(); }

      // for "found_in_previous" and "not_found_in_previous" select keys
    void previous_chart(std::string_view previous_chart_filename);
    void previous_chart(acmacs::chart::ChartP aPreviousChart) { mPreviousChart = aPreviousChart; }
    const acmacs::chart::Chart* previous_chart() const { return mPreviousChart.get(); }

    // const ChartAccess& chart(std::string_view filename) const;
    // acmacs::chart::ChartModify& chart() { return *charts_[0]; }
    // const acmacs::chart::ChartModify& chart() const { return *charts_[0]; }
    // auto chartp() const { return charts_[0]; }

    // acmacs::chart::ChartModify& chart(size_t index) { load_chart(index);  return *charts_[index]; }
    // const acmacs::chart::ChartModify& chart(size_t index) const { load_chart(index);  return *charts_[index]; }
    // std::string_view chart_name(size_t index) const { return chart_filenames_.at(index); }
    // const acmacs::chart::ChartModify& chart(std::string_view filename) const;

    // size_t number_of_antigens() const { return chart().number_of_antigens(); }
    // size_t number_of_sera() const { return chart().number_of_sera(); }
    // size_t number_of_points() const { return number_of_antigens() + number_of_sera(); }

    // const acmacs::chart::ProjectionModify& projection() const { return *mProjectionModify; }
    // acmacs::chart::ProjectionModify& projection() { return *mProjectionModify; }
    // size_t projection_no() const { return projection().projection_no(); }
    // acmacs::number_of_dimensions_t number_of_dimensions() const { return projection().number_of_dimensions(); }
    // std::shared_ptr<acmacs::Layout> layout() const { return projection().layout(); }
    // std::shared_ptr<acmacs::Layout> transformed_layout() const { return projection().transformed_layout(); }
    // bool point_has_coordinates(size_t point_no) const { return projection().layout()->point_has_coordinates(point_no); }

    // acmacs::Transformation transformation() const { return projection().transformation(); }
    // const acmacs::Transformation& inverted_transformation() const
    // {
    //     if (!inverted_transformation_)
    //         inverted_transformation_ = transformation().inverse();
    //     return *inverted_transformation_;
    // }

    // const acmacs::chart::PlotSpecModify& plot_spec() const { return *mPlotSpec; }
    // acmacs::chart::PlotSpecModify& plot_spec() { return *mPlotSpec; }
    // acmacs::chart::PlotSpecModifyP plot_spec_ptr() const { return mPlotSpec; }
    // acmacs::chart::DrawingOrder& drawing_order() { return plot_spec().drawing_order_modify(); }
    // const acmacs::chart::DrawingOrder drawing_order() const { return plot_spec().drawing_order(); }

    // const acmacs::seqdb::subset& match_seqdb() const;
    // acmacs::seqdb::v3::Seqdb::aas_indexes_t aa_at_pos1_for_antigens(const std::vector<size_t>& aPositions1) const;

  private:
    std::vector<ChartAccess> charts_;
    acmacs::chart::ChartP mPreviousChart;
    // mutable std::vector<std::string_view> chart_filenames_;
    // mutable std::vector<acmacs::chart::ChartModifyP> charts_;
    // acmacs::chart::ProjectionModifyP mProjectionModify;
    // acmacs::chart::PlotSpecModifyP mPlotSpec;
    // mutable std::optional<acmacs::seqdb::subset> matched_seqdb_;
    // mutable std::optional<acmacs::Transformation> inverted_transformation_;

    // void load_chart(size_t index) const;

}; // class ChartSelectInterface

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
