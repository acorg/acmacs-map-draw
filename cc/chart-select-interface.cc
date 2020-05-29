#include "acmacs-chart-2/factory-import.hh"
#include "acmacs-map-draw/chart-select-interface.hh"

// ----------------------------------------------------------------------

ChartSelectInterface::ChartSelectInterface(acmacs::chart::ChartModifyP aChart, size_t aProjectionNo)
    : chart_filenames_{std::string_view{}}, charts_{aChart}, mProjectionModify(aChart->projection_modify(aProjectionNo)), mPlotSpec(aChart->plot_spec_modify())
{

} // ChartSelectInterface::ChartSelectInterface

// ----------------------------------------------------------------------

ChartSelectInterface::ChartSelectInterface(std::string_view chart_filename, size_t aProjectionNo)
    : chart_filenames_{chart_filename}, charts_{1}
{
    load_chart(0);
    mProjectionModify = chart().projection_modify(aProjectionNo);
    mPlotSpec = chart().plot_spec_modify();

} // ChartSelectInterface::ChartSelectInterface

// ----------------------------------------------------------------------

ChartSelectInterface::ChartSelectInterface(const std::vector<std::string_view>& chart_filenames, size_t aProjectionNo)
    : chart_filenames_{chart_filenames}, charts_{chart_filenames.size()}
{
    load_chart(0);
    mProjectionModify = chart().projection_modify(aProjectionNo);
    mPlotSpec = chart().plot_spec_modify();

} // ChartSelectInterface::ChartSelectInterface

// ----------------------------------------------------------------------

void ChartSelectInterface::load_chart(size_t index) const
{
    if (index >= number_of_charts())
        throw std::runtime_error{fmt::format("ChartSelectInterface: invalid chart index {} (available: 0..{})", index, number_of_charts() - 1)};
    if (!charts_[index])
        charts_[index] = std::make_shared<acmacs::chart::ChartModify>(acmacs::chart::import_from_file(chart_filenames_[index]));

} // ChartSelectInterface::load_chart

// ----------------------------------------------------------------------

const acmacs::chart::ChartModify& ChartSelectInterface::chart(std::string_view filename) const
{
    if (const auto found = std::find(std::begin(chart_filenames_), std::end(chart_filenames_), filename); found != std::end(chart_filenames_))
        return chart(static_cast<size_t>(found - std::begin(chart_filenames_)));
    chart_filenames_.push_back(filename);
    charts_.emplace_back();
    return chart(chart_filenames_.size() - 1);

} // ChartSelectInterface::chart

// ----------------------------------------------------------------------

void ChartSelectInterface::previous_chart(std::string_view previous_chart_filename)
{
    mPreviousChart = std::make_shared<acmacs::chart::ChartModify>(acmacs::chart::import_from_file(previous_chart_filename));

} // ChartSelectInterface::previous_chart

// ----------------------------------------------------------------------

const acmacs::seqdb::subset& ChartSelectInterface::match_seqdb() const
{
    if (!matched_seqdb_)
        matched_seqdb_ = acmacs::seqdb::get().match(*chart().antigens(), chart().info()->virus_type(acmacs::chart::Info::Compute::Yes));
    return *matched_seqdb_;

} // ChartSelectInterface::match_seqdb

// ----------------------------------------------------------------------

acmacs::seqdb::v3::Seqdb::aas_indexes_t ChartSelectInterface::aa_at_pos1_for_antigens(const std::vector<size_t>& aPositions1) const
{
    const auto& seqdb = acmacs::seqdb::get();
    acmacs::seqdb::v3::Seqdb::aas_indexes_t aas_indexes;
    for (auto [ag_no, ref] : acmacs::enumerate(match_seqdb())) {
        if (ref) {
            std::string aa(aPositions1.size(), 'X');
            std::transform(aPositions1.begin(), aPositions1.end(), aa.begin(), [&seqdb, ref = ref](size_t pos) { return ref.aa_at_pos(seqdb, acmacs::seqdb::pos1_t{pos}); });
            aas_indexes[aa].push_back(ag_no);
        }
    }
    return aas_indexes;

} // ChartSelectInterface::aa_at_pos1_for_antigens

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
