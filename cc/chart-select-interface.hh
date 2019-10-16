#pragma once

#include "seqdb-3/seqdb.hh"
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

    const acmacs::seqdb::subset& match_seqdb() const
    {
        if (!matched_seqdb_)
            matched_seqdb_ = acmacs::seqdb::get().match(*chart().antigens(), chart().info()->virus_type(acmacs::chart::Info::Compute::Yes));
        return *matched_seqdb_;
    }

    acmacs::seqdb::v3::Seqdb::aas_indexes_t aa_at_pos1_for_antigens(const std::vector<size_t>& aPositions1) const
    {
        acmacs::seqdb::v3::Seqdb::aas_indexes_t aas_indexes;
        for (auto [ag_no, ref] : acmacs::enumerate(match_seqdb())) {
            if (ref) {
                std::string aa(aPositions1.size(), 'X');
                std::transform(aPositions1.begin(), aPositions1.end(), aa.begin(), [ref = ref](size_t pos) { return ref.seq().aa_at_pos(acmacs::seqdb::pos1_t{pos}); });
                aas_indexes[aa].push_back(ag_no);
            }
        }
        return aas_indexes;
    }

 private:
    acmacs::chart::ChartModifyP mChart;
    acmacs::chart::ProjectionModifyP mProjectionModify;
    acmacs::chart::PlotSpecModifyP mPlotSpec;
    acmacs::chart::ChartP mPreviousChart;
    mutable std::optional<acmacs::seqdb::subset> matched_seqdb_;

}; // class ChartSelectInterface

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
