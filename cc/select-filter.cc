#include "acmacs-chart-2/serum-line.hh"
#include "acmacs-map-draw/select-filter.hh"
#include "acmacs-map-draw/vaccines.hh"

// ----------------------------------------------------------------------

inline acmacs::seqdb::sequence_aligned_t sequence_aa(acmacs::chart::ChartModify& chart, size_t point_index)
{
    const auto number_of_antigens = chart.number_of_antigens();
    if (point_index < number_of_antigens)
        return acmacs::seqdb::sequence_aligned_t{chart.antigens_modify().at(point_index).sequence_aa()};
    else
        return acmacs::seqdb::sequence_aligned_t{chart.sera_modify().at(point_index - number_of_antigens).sequence_aa()};
}

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::sequenced(ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes)
{
    auto& chart = aChartSelectInterface.chart();
    acmacs::seqdb::populate(chart);
    const auto not_sequenced = [&chart](auto index) { return sequence_aa(chart, index).empty(); };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), not_sequenced), indexes.end());

} // acmacs::map_draw::select::filter::sequenced

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::not_sequenced(ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes)
{
    auto& chart = aChartSelectInterface.chart();
    acmacs::seqdb::populate(chart);
    const auto sequenced = [&chart](auto index) { return ! sequence_aa(chart, index).empty(); };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), sequenced), indexes.end());

} // acmacs::map_draw::select::filter::not_sequenced

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::clade(ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aClade)
{
    auto& chart = aChartSelectInterface.chart();
    acmacs::seqdb::populate(chart);

    const auto has_clade = [&chart, number_of_antigens = chart.number_of_antigens()](auto index, std::string_view clade) {
        if (index < number_of_antigens)
            return chart.antigens_modify().at(index).clades().exists(std::string{clade});
        else
            return chart.sera_modify().at(index - number_of_antigens).clades().exists(std::string{clade});
    };

    if (aClade[0] == '!')
        indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), [clade = aClade.substr(1), has_clade](auto index) { return has_clade(index, clade); }), indexes.end());
    else
        indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), [clade = aClade, has_clade](auto index) { return !has_clade(index, clade); }), indexes.end());

} // acmacs::map_draw::select::filter::clade

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::amino_acid_at_pos(ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, char amino_acid, acmacs::seqdb::pos1_t pos1, bool equal)
{
    auto& chart = aChartSelectInterface.chart();
    acmacs::seqdb::populate(chart);
    auto at_pos_neq = [amino_acid,pos1,equal](const auto& seq) -> bool { return equal ? seq.at(pos1) != amino_acid : seq.at(pos1) == amino_acid; };
    auto not_aa_at_pos = [&chart, &at_pos_neq](auto index) {
        const auto seq = sequence_aa(chart, index);
        return seq.empty() || at_pos_neq(seq);
    };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), not_aa_at_pos), indexes.end());

} // acmacs::map_draw::select::filter::amino_acid_at_pos

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::amino_acid_at_pos(ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::seqdb::amino_acid_at_pos1_eq_list_t& pos1_aa)
{
    for (const auto& entry : pos1_aa)
        amino_acid_at_pos(aChartSelectInterface, indexes, std::get<char>(entry), std::get<acmacs::seqdb::pos1_t>(entry), std::get<bool>(entry));

} // acmacs::map_draw::select::filter::amino_acid_at_pos

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::amino_acid_at_pos(ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::seqdb::amino_acid_at_pos1_eq_t& pos1_aa)
{
    amino_acid_at_pos(aChartSelectInterface, indexes, std::get<char>(pos1_aa), std::get<acmacs::seqdb::pos1_t>(pos1_aa), std::get<bool>(pos1_aa));

} // acmacs::map_draw::select::filter::amino_acid_at_pos

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::outlier(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, double aUnits)
{
    auto layout = aChartSelectInterface.chart(0).modified_layout();

    using sum_count_t = std::pair<acmacs::PointCoordinates, size_t>;
    auto sum_count_not_empty = [&layout](const auto& sum_count, size_t index) -> sum_count_t {
        const auto coord = layout->at(index);
        auto [sum, count] = sum_count;
        if (coord.exists()) {
            sum += coord;
            ++count;
        }
        return {sum, count};
    };
    auto [centroid, count] = std::accumulate(indexes.begin(), indexes.end(), sum_count_t{{0.0, 0.0}, 0}, sum_count_not_empty);
    centroid /= static_cast<double>(count);

    using point_dist_t = std::pair<size_t, double>; // point number (from indexes) and its distance to centroid
    std::vector<point_dist_t> point_dist(indexes->size());
    std::transform(indexes.begin(), indexes.end(), point_dist.begin(), [centroid=centroid,&layout](size_t index) -> point_dist_t {
        const auto coord = layout->at(index);
        if (coord.exists())
            return {index, acmacs::distance(centroid, coord)};
        else
            return {index, 0.0}; // not an outlier!
    });
    std::sort(point_dist.begin(), point_dist.end(), [](const auto& a, const auto& b) { return a.second > b.second; }); // outliers first

    AD_INFO("Outliers:");
    for (const auto& [point_no, dist]: point_dist)
        fmt::print(stderr, "{:8d} {}\n", point_no, dist);

    auto not_outlier = [&point_dist,aUnits](size_t index) { const auto& [_, d] = *std::find_if(point_dist.begin(), point_dist.end(), [index](const auto& pd) { return pd.first == index; }); return d < aUnits; };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), not_outlier), indexes.end());

} // acmacs::map_draw::select::filter::outlier

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::rectangle_in(acmacs::chart::Indexes& indexes, size_t aIndexBase, const acmacs::Layout& aLayout, const acmacs::Rectangle& aRectangle, Rotation rotation)
{
    acmacs::Transformation transformation;
    transformation.rotate(- rotation); // rectangle rotated -> layout rotated in opposite direction
    auto layout = aLayout.transform(transformation);
    const auto not_in_rectangle = [&layout,aIndexBase,&aRectangle](auto index) -> bool {
        const auto& p = layout->at(index + aIndexBase);
        return p.number_of_dimensions() == acmacs::number_of_dimensions_t{2} ? !aRectangle.within(p) : true;
    };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), not_in_rectangle), indexes.end());

} // acmacs::map_draw::select::filter::rectangle_in

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::vaccine(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::virus::type_subtype_t& virus_type, const VaccineMatchData& aMatchData)
{
    if (!virus_type.empty()) {

#include "acmacs-base/global-constructors-push.hh"
          // thread unsafe!
        static std::map<const ChartSelectInterface*, Vaccines> cache_vaccines;
#include "acmacs-base/diagnostics-pop.hh"

        auto vaccines_of_chart = cache_vaccines.find(&aChartSelectInterface);
        if (vaccines_of_chart == cache_vaccines.end()) {
            Timeit ti_vaccines("Vaccines of chart: ");
            vaccines_of_chart = cache_vaccines.emplace(&aChartSelectInterface, Vaccines{aChartSelectInterface.chart()}).first;
            // AD_DEBUG("{}", vaccines_of_chart->second.report(hidb::Vaccines::ReportConfig{}.indent(2)));
        }
        auto vaccine_indexes = vaccines_of_chart->second.indices(aMatchData);

        std::sort(vaccine_indexes.begin(), vaccine_indexes.end());
        acmacs::chart::Indexes result(vaccine_indexes.size());
        const auto end = std::set_intersection(indexes.begin(), indexes.end(), vaccine_indexes.begin(), vaccine_indexes.end(), result.begin());
        indexes.get().erase(std::copy(result.begin(), end, indexes.begin()), indexes.end());
    }
    else {
        indexes.get().clear();
        AD_WARNING("unknown virus_type for chart: {}", aChartSelectInterface.chart().make_name());
    }

} // acmacs::map_draw::select::filter::vaccine

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::layer(const acmacs::chart::Chart& chart, acmacs::chart::Indexes& indexes, int aLayer, ag_sr_ ag_sr)
{
    auto titers = chart.titers();
    if (aLayer < 0)
        aLayer = static_cast<int>(titers->number_of_layers()) + aLayer;
    if (aLayer < 0 || aLayer > static_cast<int>(titers->number_of_layers()))
        throw std::runtime_error(fmt::format("Invalid layer: {}", aLayer));

    const auto of_layer = titers->antigens_sera_of_layer(static_cast<size_t>(aLayer));
    const acmacs::chart::PointIndexList& ag_or_sr_in_layer = ag_sr == antigens ? of_layer.first : of_layer.second;
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(),
                                       [&ag_or_sr_in_layer](size_t index) { return std::find(std::begin(ag_or_sr_in_layer), std::end(ag_or_sr_in_layer), index) == std::end(ag_or_sr_in_layer); }),
                        indexes.end());

} // acmacs::map_draw::select::filter::layer

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::relative_to_serum_line(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, double distance_min, double distance_max, int direction)
{
    acmacs::chart::SerumLine serum_line(aChartSelectInterface.chart(0).modified_projection());
    auto layout = aChartSelectInterface.chart(0).modified_layout();

    auto not_relative_to_line = [&serum_line, &layout, distance_min, distance_max, direction](auto antigen_no) -> bool {
        const auto distance = serum_line.line().distance_with_direction(layout->at(antigen_no));
        return std::abs(distance) < distance_min || std::abs(distance) > distance_max || (direction != 0 && (direction * distance) < 0);
    };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), not_relative_to_line), indexes.end());

} // acmacs::map_draw::select::filter::relative_to_serum_line

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::serum_id_in(const acmacs::chart::Sera& sera, acmacs::chart::Indexes& indexes, std::string_view serum_id)
{
    if (!serum_id.empty() && serum_id[0] == '~')
        sera.filter_serum_id(indexes, std::regex{std::next(std::begin(serum_id), 1), std::end(serum_id), acmacs::regex::icase});
    else
        sera.filter_serum_id(indexes, serum_id);

} // acmacs::map_draw::select::filter::serum_id_in

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::antigens_titrated_against(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& antigen_indexes, const acmacs::chart::Indexes& serum_indexes)
{
    auto titers = aChartSelectInterface.chart().titers();
    auto not_titrated = [&titers, &serum_indexes](auto antigen_no) -> bool {
        return std::all_of(std::begin(serum_indexes), std::end(serum_indexes), [&titers, antigen_no](auto sr_no) { return titers->titer(antigen_no, sr_no).is_dont_care(); });
    };
    antigen_indexes.get().erase(std::remove_if(antigen_indexes.begin(), antigen_indexes.end(), not_titrated), antigen_indexes.end());

} // acmacs::map_draw::select::filter::antigens_titrated_against

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::sera_titrated_against(const ChartSelectInterface& aChartSelectInterface, const acmacs::chart::Indexes& antigen_indexes, acmacs::chart::Indexes& serum_indexes)
{
    auto titers = aChartSelectInterface.chart().titers();
    auto not_titrated = [&titers, &antigen_indexes](auto serum_no) -> bool {
        return std::any_of(std::begin(antigen_indexes), std::end(antigen_indexes), [&titers, serum_no](auto ag_no) { return titers->titer(ag_no, serum_no).is_dont_care(); });
    };
    serum_indexes.get().erase(std::remove_if(serum_indexes.begin(), serum_indexes.end(), not_titrated), serum_indexes.end());

} // acmacs::map_draw::select::filter::sera_titrated_against

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::antigens_not_titrated_against(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& antigen_indexes, const acmacs::chart::Indexes& serum_indexes)
{
    auto titers = aChartSelectInterface.chart().titers();
    auto titrated = [&titers, &serum_indexes](auto antigen_no) -> bool {
        return std::any_of(std::begin(serum_indexes), std::end(serum_indexes), [&titers, antigen_no](auto sr_no) { return !titers->titer(antigen_no, sr_no).is_dont_care(); });
    };
    antigen_indexes.get().erase(std::remove_if(antigen_indexes.begin(), antigen_indexes.end(), titrated), antigen_indexes.end());

} // acmacs::map_draw::select::filter::antigens_not_titrated_against

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::sera_not_titrated_against(const ChartSelectInterface& aChartSelectInterface, const acmacs::chart::Indexes& antigen_indexes, acmacs::chart::Indexes& serum_indexes)
{
    auto titers = aChartSelectInterface.chart().titers();
    auto titrated = [&titers, &antigen_indexes](auto serum_no) -> bool {
        return std::any_of(std::begin(antigen_indexes), std::end(antigen_indexes), [&titers, serum_no](auto ag_no) { return !titers->titer(ag_no, serum_no).is_dont_care(); });
    };
    serum_indexes.get().erase(std::remove_if(serum_indexes.begin(), serum_indexes.end(), titrated), serum_indexes.end());

} // acmacs::map_draw::select::filter::sera_not_titrated_against

// ----------------------------------------------------------------------

std::map<std::string_view, size_t> acmacs::map_draw::select::clades(ChartSelectInterface& aChartSelectInterface)
{
    auto& chart = aChartSelectInterface.chart();
    acmacs::seqdb::populate(chart);
    std::map<std::string_view, size_t> result;
    for (auto antigen : aChartSelectInterface.chart().antigens_modify()) {
        for (const auto& clade: antigen->clades())
            ++result[clade];
    }
    return result;

} // acmacs::map_draw::select::clades

// ----------------------------------------------------------------------

template <typename AgSr> void acmacs::map_draw::select::filter::name_in(const AgSr& aAgSr, acmacs::chart::Indexes& indexes, std::string_view aName)
{
    // Timeit ti("filter_name_in " + aName + ": ", do_report_time(mVerbose));
    acmacs::chart::Indexes result(indexes->size());
    const acmacs::chart::Indexes by_name = aAgSr.find_by_name(aName);
    // std::cerr << "DEBUG: SelectAntigensSera::filter_name_in \"" << aName << "\": " << by_name << '\n';
    const auto end = std::set_intersection(indexes.begin(), indexes.end(), by_name.begin(), by_name.end(), result.begin());
    indexes.get().erase(std::copy(result.begin(), end, indexes.begin()), indexes.end());

} // acmacs::map_draw::select::filter::name_in<>

template void acmacs::map_draw::select::filter::name_in(const acmacs::chart::Antigens& aAgSr, acmacs::chart::Indexes& indexes, std::string_view aName);
template void acmacs::map_draw::select::filter::name_in(const acmacs::chart::Sera& aAgSr, acmacs::chart::Indexes& indexes, std::string_view aName);

// ----------------------------------------------------------------------

template <typename AgSr> void acmacs::map_draw::select::filter::name_not_in(const AgSr& aAgSr, acmacs::chart::Indexes& indexes, std::string_view aName)
{
    // Timeit ti("filter_name_in " + aName + ": ", do_report_time(mVerbose));
    acmacs::chart::Indexes result(indexes->size());
    const acmacs::chart::Indexes by_name = aAgSr.find_by_name(aName);
    // std::cerr << "DEBUG: SelectAntigensSera::filter_name_in \"" << aName << "\": " << by_name << '\n';
    const auto end = std::set_difference(indexes.begin(), indexes.end(), by_name.begin(), by_name.end(), result.begin());
    indexes.get().erase(std::copy(result.begin(), end, indexes.begin()), indexes.end());

} // acmacs::map_draw::select::filter::name_in<>

template void acmacs::map_draw::select::filter::name_not_in(const acmacs::chart::Antigens& aAgSr, acmacs::chart::Indexes& indexes, std::string_view aName);
template void acmacs::map_draw::select::filter::name_not_in(const acmacs::chart::Sera& aAgSr, acmacs::chart::Indexes& indexes, std::string_view aName);

// ----------------------------------------------------------------------

template <typename AgSr> void acmacs::map_draw::select::filter::table_ag_sr(const AgSr& aAgSr, acmacs::chart::Indexes& indexes, std::string_view aTable, std::shared_ptr<hidb::Tables> aHidbTables)
{
    auto not_in_table = [aTable, &aAgSr, hidb_tables = *aHidbTables](size_t index) {
        if (auto ag_sr = aAgSr[index]; ag_sr) { // found in hidb
            for (auto table_no : ag_sr->tables()) {
                auto table = hidb_tables[table_no];
                // AD_DEBUG("{} {} [{}] \"{}\" {}", index, table_no, table->date(), table->name(), table->name() == aTable);
                if (table->date() == aTable || table->name() == aTable)
                    return false;
            }
        }
        return true;
    };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), not_in_table), indexes.end());

} // acmacs::map_draw::select::filter::table_ag_sr

template void acmacs::map_draw::select::filter::table_ag_sr(const std::vector<std::shared_ptr<hidb::Antigen>>& aAgSr, acmacs::chart::Indexes& indexes, std::string_view aTable, std::shared_ptr<hidb::Tables> aHidbTables);
template void acmacs::map_draw::select::filter::table_ag_sr(const std::vector<std::shared_ptr<hidb::Serum>>& aAgSr, acmacs::chart::Indexes& indexes, std::string_view aTable, std::shared_ptr<hidb::Tables> aHidbTables);

// ----------------------------------------------------------------------

template <typename AgSrPerIndex> void acmacs::map_draw::select::filter::most_used(const AgSrPerIndex& ag_sr_per_index, acmacs::chart::Indexes& indexes, size_t number_of_most_used)
{
    std::vector<std::pair<size_t, size_t>> index_num_tables;
    for (size_t index_no{0}; index_no < indexes.size(); ++index_no) {
        if (ag_sr_per_index[index_no])
            index_num_tables.emplace_back(indexes[index_no], ag_sr_per_index[index_no]->number_of_tables());
        else
            index_num_tables.emplace_back(indexes[index_no], 0);
    }
    std::sort(std::begin(index_num_tables), std::end(index_num_tables), [](const auto& e1, const auto& e2) { return e1.second > e2.second; }); // most used first
    acmacs::Indexes to_remove;
    std::transform(std::next(std::begin(index_num_tables), static_cast<ssize_t>(std::min(number_of_most_used, index_num_tables.size()))), std::end(index_num_tables), std::back_inserter(to_remove),
                   [](const auto& en) { return en.first; });
    indexes.remove(ReverseSortedIndexes{to_remove});

} // acmacs::map_draw::select::filter::most_used<>

template void acmacs::map_draw::select::filter::most_used(const std::vector<std::shared_ptr<hidb::Antigen>>& ag_sr_per_index, acmacs::chart::Indexes& indexes, size_t number_of_most_used);
template void acmacs::map_draw::select::filter::most_used(const std::vector<std::shared_ptr<hidb::Serum>>& ag_sr_per_index, acmacs::chart::Indexes& indexes, size_t number_of_most_used);

// ----------------------------------------------------------------------

template <typename AgSrPerIndex> void acmacs::map_draw::select::filter::most_used_for_name(const AgSrPerIndex& ag_sr_per_index, acmacs::chart::Indexes& indexes, size_t number_of_most_used)
{
    if (indexes.size() < 2)
        return;

    struct data_t
    {
        size_t index;
        acmacs::virus::name_t name{""};
        size_t num_tables{0};
    };

    acmacs::Indexes to_remove;
    std::vector<data_t> data;
    for (size_t index_no{0}; index_no < indexes.size(); ++index_no) {
        if (ag_sr_per_index[index_no])
            data.push_back(data_t{indexes[index_no], ag_sr_per_index[index_no]->name(), ag_sr_per_index[index_no]->number_of_tables()});
        else                    // ignore if not found in hidb and remove it from the indexes
            to_remove.push_back(indexes[index_no]);
    }

    std::sort(std::begin(data), std::end(data), [](const auto& e1, const auto& e2) {
        if (e1.name == e2.name)
            return e1.num_tables > e2.num_tables; // most used first
        else
            return e1.name < e2.name; // (names order is not improtant)
    });

    const acmacs::virus::name_t* prev_name{nullptr};
    size_t num_found{0};
    for (const auto& en : data) {
        // AD_DEBUG("most_used_for_name {} {} {}", en.index, en.name, en.num_tables);
        if (!prev_name || *prev_name != en.name) {
            prev_name = &en.name;
            num_found = 0;
        }
        if (num_found >= number_of_most_used)
            to_remove.push_back(en.index);
        ++num_found;
    }
    indexes.remove(ReverseSortedIndexes{to_remove});

} // acmacs::map_draw::select::filter::most_used_for_name<>

template void acmacs::map_draw::select::filter::most_used_for_name(const std::vector<std::shared_ptr<hidb::Antigen>>& ag_sr_per_index, acmacs::chart::Indexes& indexes, size_t number_of_most_used);
template void acmacs::map_draw::select::filter::most_used_for_name(const std::vector<std::shared_ptr<hidb::Serum>>& ag_sr_per_index, acmacs::chart::Indexes& indexes, size_t number_of_most_used);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
