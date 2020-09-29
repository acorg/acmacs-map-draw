#include "acmacs-base/string-substitute.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "acmacs-chart-2/factory-export.hh"
#include "acmacs-map-draw/chart-select-interface.hh"

// ----------------------------------------------------------------------

void ChartAccess::reset()
{
    modified_.reset();
    modified_projection_.reset();
    inverted_transformation_ = std::nullopt;
    modified_plot_spec_.reset();

} // ChartAccess::reset

// ----------------------------------------------------------------------

bool ChartAccess::chart_access() const
{
    if (!original_) {
        original_ = acmacs::chart::import_from_file(filename_);
        return true;
    }
    else
        return false;

} // ChartAccess::chart_access

// ----------------------------------------------------------------------

bool ChartAccess::modified_chart_access() const
{
    if (chart_access() || !modified_) {
        modified_ = std::make_shared<acmacs::chart::ChartModify>(original_);
        return true;
    }
    else
        return false;

} // ChartAccess::modified_chart_access

// ----------------------------------------------------------------------

bool ChartAccess::modified_projection_access() const
{
    if (modified_chart_access() || !modified_projection_) {
        modified_projection_ = modified_->projection_modify(projection_no_);
        return true;
    }
    else
        return false;

} // ChartAccess::modified_projection_access

// ----------------------------------------------------------------------

bool ChartAccess::modified_plot_spec_access() const
{
    if (modified_chart_access() || !modified_plot_spec_) {
        modified_plot_spec_ = modified_->plot_spec_modify();
        return true;
    }
    else
        return false;

} // ChartAccess::modified_plot_spec_access

// ----------------------------------------------------------------------

void ChartAccess::export_chart(std::string_view filename) const
{
    using namespace std::string_view_literals;
    const auto to_export = [this]() -> const acmacs::chart::Chart& {
        if (modified_)
            return *modified_;
        else
            return *original_;
    };
    AD_INFO("Exporting chart to {}", filename);
    acmacs::chart::export_factory(to_export(), filename, "mapi"sv);

} // ChartAccess::export_chart

// ----------------------------------------------------------------------

const acmacs::seqdb::subset& ChartAccess::match_seqdb() const
{
    if (!matched_seqdb_) {
        auto antigens = chart().antigens();
        matched_seqdb_ = acmacs::seqdb::get().match(*antigens, chart().info()->virus_type(acmacs::chart::Info::Compute::Yes));

        // for (size_t ag_no{0}; ag_no < matched_seqdb_->size(); ++ag_no) {
        //     if (const auto& en{(*matched_seqdb_)[ag_no]}; !en.empty())
        //         AD_DEBUG("AG {:3d} {:50s} -- {:50s} -- {}", ag_no, antigens->at(ag_no)->full_name(), en.seq_id(), en.seq_with_sequence(acmacs::seqdb::get()).clades);
        // }
    }
    return *matched_seqdb_;

} // ChartAccess::match_seqdb

// ----------------------------------------------------------------------

acmacs::seqdb::v3::Seqdb::aas_indexes_t ChartAccess::aa_at_pos1_for_antigens(const std::vector<size_t>& aPositions1) const
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

} // ChartAccess::aa_at_pos1_for_antigens

// ----------------------------------------------------------------------

// void ChartAccess::chart_metadata(fmt::dynamic_format_arg_store<fmt::format_context>& store) const
// {
//     int is_this_function_necessary;

//     using namespace std::string_view_literals;

//     auto info{chart().info()};
//     auto titers{chart().titers()};
//     const auto& projection = modified_projection();

//     const std::string virus_type{info->virus_type()}, lineage{chart().lineage()};
//     const auto assay{info->assay()};
//     const auto subset{info->subset(acmacs::chart::Info::Compute::Yes)};

//     std::string virus_type_lineage;
//     if (lineage.empty())
//         virus_type_lineage = virus_type;
//     else
//         virus_type_lineage = fmt::format("{}/{}", virus_type, ::string::capitalize(lineage.substr(0, 3)));
//     std::string virus_type_lineage_subset_short;
//     if (virus_type == "A(H1N1)"sv) {
//         if (subset == "2009pdm"sv)
//             virus_type_lineage_subset_short = "h1pdm";
//         else
//             virus_type_lineage_subset_short = "h1";
//     }
//     else if (virus_type == "A(H3N2)"sv)
//         virus_type_lineage_subset_short = "h3";
//     else if (virus_type == "B"sv)
//         virus_type_lineage_subset_short = fmt::format("b{}", ::string::lower(lineage.substr(0, 3)));
//     else
//         virus_type_lineage_subset_short = ::string::lower(virus_type);

//     std::string virus_type_lineage_short{virus_type_lineage_subset_short};
//     if (virus_type == "A(H1N1)"sv)
//         virus_type_lineage_short = "h1";

//     std::string assay_neut{assay.hi_or_neut()};
//     if (assay_neut == "hi")
//         assay_neut.clear();

//     // https://stackoverflow.com/questions/61635042/c-how-can-i-formatting-stdstring-with-collection-efficiently
//     store.push_back(fmt::arg("virus", info->virus()));
//     store.push_back(fmt::arg("virus_type", virus_type));
//     store.push_back(fmt::arg("virus_type_lineage", virus_type_lineage));
//     store.push_back(fmt::arg("lineage", lineage));
//     store.push_back(fmt::arg("subset", subset));
//     store.push_back(fmt::arg("SUBSET", ::string::upper(subset)));
//     store.push_back(fmt::arg("virus_type_lineage_subset_short", virus_type_lineage_subset_short));
//     store.push_back(fmt::arg("virus_type_lineage_short", virus_type_lineage_short));
//     store.push_back(fmt::arg("assay", assay.hi_or_neut()));
//     store.push_back(fmt::arg("Assay", assay.HI_or_Neut()));
//     store.push_back(fmt::arg("assay_full", assay));
//     store.push_back(fmt::arg("assay_neut", assay_neut));
//     store.push_back(fmt::arg("assay_HI_Neut", assay.HI_or_Neut()));
//     store.push_back(fmt::arg("lab", info->lab()));
//     store.push_back(fmt::arg("lab_lower", ::string::lower(info->lab())));
//     store.push_back(fmt::arg("rbc", info->rbc_species()));
//     store.push_back(fmt::arg("table_date", info->date(acmacs::chart::Info::Compute::Yes)));
//     store.push_back(fmt::arg("number_of_antigens", chart().number_of_antigens()));
//     store.push_back(fmt::arg("number_of_sera", chart().number_of_sera()));
//     store.push_back(fmt::arg("number_of_layers", titers->number_of_layers()));
//     store.push_back(fmt::arg("minimum_column_basis", static_cast<std::string>(projection.minimum_column_basis())));
//     store.push_back(fmt::arg("stress", projection.stress()));
//     store.push_back(fmt::arg("name", chart().make_name()));

//     auto sera = chart().sera();
//     for (auto [no, serum] : acmacs::enumerate(*sera)) {
//         store.push_back(fmt::arg(fmt::format("serum_{}_full_name", no).c_str(), serum->full_name()));
//         store.push_back(fmt::arg(fmt::format("serum_{}_full_name_without_passage", no).c_str(), serum->full_name_without_passage()));
//         store.push_back(fmt::arg(fmt::format("serum_{}_full_name_with_passage", no).c_str(), serum->full_name_with_passage()));
//         store.push_back(fmt::arg(fmt::format("serum_{}_abbreviated_name", no).c_str(), serum->abbreviated_name()));
//         store.push_back(fmt::arg(fmt::format("serum_{}_abbreviated_name_with_serum_id", no).c_str(), serum->abbreviated_name_with_serum_id()));
//         store.push_back(fmt::arg(fmt::format("serum_{}_designation", no).c_str(), serum->designation()));
//         store.push_back(fmt::arg(fmt::format("serum_{}_designation_without_serum_id", no).c_str(), serum->designation_without_serum_id()));
//     }

// } // ChartAccess::chart_metadata

// // ----------------------------------------------------------------------

// std::string ChartAccess::substitute_metadata(std::string_view pattern) const
// {
//     int is_this_function_necessary;

//     try {
//         fmt::dynamic_format_arg_store<fmt::format_context> store;
//         chart_metadata(store);
//         return acmacs::string::substitute_from_store(pattern, store, acmacs::string::if_no_substitution_found::leave_as_is);
//     }
//     catch (std::exception& err) {
//         AD_ERROR("fmt cannot substitute in \"{}\": {}", pattern, err);
//         throw;
//     }

// } // ChartAccess::substitute_metadata

// ======================================================================

enum VaccineData::type VaccineData::type_from(std::string_view source)
{
    using namespace std::string_view_literals;

    if (source == "previous"sv)
        return VaccineData::type::previous;
    else if (source == "current"sv)
        return VaccineData::type::current;
    else if (source == "surrogate"sv)
        return VaccineData::type::surrogate;
    else if (source == "any"sv)
        return VaccineData::type::any;
    else
        AD_WARNING("Unrecognized vaccine type: \"{}\" (any assumed)", source);
    return VaccineData::type::any;

} // VaccineData::type_from

// ----------------------------------------------------------------------

ChartSelectInterface::ChartSelectInterface(const std::vector<std::string_view>& filenames, size_t projection_no)
{
    for (const auto& filename : filenames)
        charts_.emplace_back(filename, projection_no);

} // ChartSelectInterface::ChartSelectInterface

// ----------------------------------------------------------------------

ChartAccess& ChartSelectInterface::chart(size_t index)
{
    if (index >= charts_.size())
        throw std::runtime_error{fmt::format("ChartSelectInterface: invalid chart index {} (available: 0..{})", index, charts_.size() - 1)};
    return charts_[index];

} // ChartSelectInterface::chart

// ----------------------------------------------------------------------

const ChartAccess& ChartSelectInterface::chart(size_t index) const
{
    if (index >= charts_.size())
        throw std::runtime_error{fmt::format("ChartSelectInterface: invalid chart index {} (available: 0..{})", index, charts_.size() - 1)};
    return charts_[index];

} // ChartSelectInterface::chart

// ----------------------------------------------------------------------

ChartAccess& ChartSelectInterface::chart(std::string_view filename, size_t projection_no)
{
    if (const auto found = std::find_if(std::begin(charts_), std::end(charts_), [filename](const auto& en) { return en.filename() == filename; }); found != std::end(charts_))
        return *found;
    else
        return charts_.emplace_back(filename, projection_no);

} // ChartSelectInterface::chart

// ----------------------------------------------------------------------

void ChartSelectInterface::previous_chart(std::string_view previous_chart_filename)
{
    mPreviousChart = std::make_shared<acmacs::chart::ChartModify>(acmacs::chart::import_from_file(previous_chart_filename));

} // ChartSelectInterface::previous_chart


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
