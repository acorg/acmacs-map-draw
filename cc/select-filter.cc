#include "acmacs-chart-2/serum-line.hh"
#include "acmacs-map-draw/select-filter.hh"
#include "acmacs-map-draw/vaccines.hh"

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::sequenced(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes)
{
    const auto& entries = aChartSelectInterface.match_seqdb();
    auto not_sequenced = [&entries](auto index) -> bool { return !entries[index]; };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), not_sequenced), indexes.end());

} // acmacs::map_draw::select::filter::sequenced

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::not_sequenced(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes)
{
    const auto& entries = aChartSelectInterface.match_seqdb();
    const auto sequenced = [&entries](auto index) -> bool { return !entries[index].empty(); };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), sequenced), indexes.end());

} // acmacs::map_draw::select::filter::not_sequenced

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::clade(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aClade)
{
    const auto& entries = aChartSelectInterface.match_seqdb();
    const auto& seqdb = acmacs::seqdb::get();
    auto not_in_clade = [&entries,aClade,&seqdb](auto index) -> bool { const auto& entry = entries[index]; return !entry || !entry.has_clade(seqdb, aClade); };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), not_in_clade), indexes.end());

} // acmacs::map_draw::select::filter::clade

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::amino_acid_at_pos(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, char amino_acid, acmacs::seqdb::pos1_t pos1, bool equal)
{
    const auto& entries = aChartSelectInterface.match_seqdb();
    const auto& seqdb = acmacs::seqdb::get();
    auto at_pos_neq = [amino_acid,pos1,equal,&seqdb](const auto& entry) -> bool { return equal ? entry.aa_at_pos(seqdb, pos1) != amino_acid : entry.aa_at_pos(seqdb, pos1) == amino_acid; };
    auto not_aa_at_pos = [&entries,&at_pos_neq](auto index) -> bool { const auto& entry = entries[index]; return !entry || at_pos_neq(entry); };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), not_aa_at_pos), indexes.end());

} // acmacs::map_draw::select::filter::amino_acid_at_pos

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::amino_acid_at_pos(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::seqdb::amino_acid_at_pos1_eq_list_t& pos1_aa)
{
    for (const auto& entry : pos1_aa)
        amino_acid_at_pos(aChartSelectInterface, indexes, std::get<char>(entry), std::get<acmacs::seqdb::pos1_t>(entry), std::get<bool>(entry));

} // acmacs::map_draw::select::filter::amino_acid_at_pos

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::amino_acid_at_pos(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::seqdb::amino_acid_at_pos1_eq_t& pos1_aa)
{
    amino_acid_at_pos(aChartSelectInterface, indexes, std::get<char>(pos1_aa), std::get<acmacs::seqdb::pos1_t>(pos1_aa), std::get<bool>(pos1_aa));

} // acmacs::map_draw::select::filter::amino_acid_at_pos

// ----------------------------------------------------------------------

void acmacs::map_draw::select::filter::outlier(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, double aUnits)
{
    auto layout = aChartSelectInterface.layout();

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
    acmacs::chart::SerumLine serum_line(aChartSelectInterface.projection());
    auto layout = aChartSelectInterface.layout();

    auto not_relative_to_line = [&serum_line, &layout, distance_min, distance_max, direction](auto antigen_no) -> bool {
        const auto distance = serum_line.line().distance_with_direction(layout->at(antigen_no));
        return std::abs(distance) < distance_min || std::abs(distance) > distance_max || (direction != 0 && (direction * distance) < 0);
    };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), not_relative_to_line), indexes.end());

} // acmacs::map_draw::select::filter::relative_to_serum_line

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

std::map<std::string_view, size_t> acmacs::map_draw::select::clades(const ChartSelectInterface& aChartSelectInterface)
{
    std::map<std::string_view, size_t> result;
    for (const auto& entry: aChartSelectInterface.match_seqdb()) {
        if (entry) {
            for (const auto& clade: entry.seq().clades)
                ++result[clade];
        }
    }
    return result;

} // acmacs::map_draw::select::clades

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
