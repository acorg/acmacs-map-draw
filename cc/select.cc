#include <chrono>
#include <cstdlib>

#include "acmacs-base/rjson.hh"
#include "hidb-5/hidb.hh"
#include "acmacs-chart-2/serum-line.hh"
#include "acmacs-map-draw/vaccines.hh"
#include "acmacs-map-draw/select.hh"

// using namespace std::string_literals;

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
//#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif

// ----------------------------------------------------------------------

SelectAntigensSera::~SelectAntigensSera()
{
} // SelectAntigensSera::~SelectAntigensSera

// ----------------------------------------------------------------------

acmacs::chart::Indexes SelectAntigensSera::select(const ChartSelectInterface& aChartSelectInterface, const rjson::value& aSelector)
{
    try {
        return std::visit(
            [this, &aChartSelectInterface](const auto& val) -> acmacs::chart::Indexes {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, std::string>)
                    return this->command(aChartSelectInterface, rjson::object{{val, true}});
                else if constexpr (std::is_same_v<T, rjson::object>)
                    return this->command(aChartSelectInterface, val);
                else
                    throw std::exception{};
            },
            aSelector);
    }
    catch (std::exception& err) {
        throw std::runtime_error{"Unsupported selector value: " + rjson::to_string(aSelector) + ": " + err.what()};
    }

} // SelectAntigensSera::select

// ----------------------------------------------------------------------

acmacs::chart::Indexes SelectAntigens::command(const ChartSelectInterface& aChartSelectInterface, const rjson::value& aSelector)
{
      // std::cout << "DEBUG: antigens command: " << aSelector << '\n';
    auto antigens = aChartSelectInterface.chart().antigens();
    auto indexes = antigens->all_indexes();
    rjson::for_each(aSelector, [this,&aChartSelectInterface,&antigens,&indexes,&aSelector](const rjson::object::value_type& key_value) {
        const auto& [key, val] = key_value;
        if (!key.empty() && (key.front() == '?' || key.back() == '?')) {
              // comment
        }
        else if (key == "all") {
              // do nothing
        }
        else if (key == "reference") {
            antigens->filter_reference(indexes);
        }
        else if (key == "test") {
            antigens->filter_test(indexes);
        }
        else if (key == "egg") {
            antigens->filter_egg(indexes);
        }
        else if (key == "cell") {
            antigens->filter_cell(indexes);
        }
        else if (key == "reassortant") {
            antigens->filter_reassortant(indexes);
        }
        else if (key == "passage") {
            const std::string_view passage = val;
            if (passage == "egg")
                antigens->filter_egg(indexes);
            else if (passage == "cell")
                antigens->filter_cell(indexes);
            else if (passage == "reassortant")
                antigens->filter_reassortant(indexes);
            else
                throw std::exception{};
        }
        else if (key == "date_range") {
            antigens->filter_date_range(indexes, val[0], val[1]);
        }
        else if (key == "older_than_days") {
            using namespace std::chrono_literals;
            const int days = val;
            const auto then = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() - 24h * days);
            char buffer[20];
            std::strftime(buffer, sizeof buffer, "%Y-%m-%d", std::localtime(&then));
            antigens->filter_date_range(indexes, "", buffer);
        }
        else if (key == "younger_than_days") {
            using namespace std::chrono_literals;
            const int days = val;
            const auto then = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() - 24h * days);
            char buffer[20];
            std::strftime(buffer, sizeof buffer, "%Y-%m-%d", std::localtime(&then));
            antigens->filter_date_range(indexes, buffer, "");
        }
        else if (key == "index") {
            indexes.erase_except(val);
        }
        else if (key == "indexes") {
            acmacs::chart::Indexes to_keep(val.size());
            rjson::transform(val, to_keep.begin(), [](const rjson::value& v) -> size_t { return v; });
            indexes.erase(std::remove_if(indexes.begin(), indexes.end(), [&to_keep](auto index) -> bool { return std::find(to_keep.begin(), to_keep.end(), index) == to_keep.end(); }), indexes.end());
        }
        else if (key == "country") {
            antigens->filter_country(indexes, string::upper(static_cast<std::string_view>(val)));
        }
        else if (key == "continent") {
            antigens->filter_continent(indexes, string::upper(static_cast<std::string_view>(val)));
        }
        else if (key == "sequenced") {
            filter_sequenced(aChartSelectInterface, indexes);
        }
        else if (key == "not_sequenced") {
            filter_not_sequenced(aChartSelectInterface, indexes);
        }
        else if (key == "clade") {
            filter_clade(aChartSelectInterface, indexes, string::upper(static_cast<std::string_view>(val)));
        }
        else if (key == "amino_acid") {
            filter_amino_acid_at_pos(aChartSelectInterface, indexes, static_cast<std::string_view>(val["aa"])[0], val["pos"]);
        }
        else if (key == "outlier") {
            filter_outlier(aChartSelectInterface, indexes, val);
        }
        else if (key == "name") {
            filter_name(aChartSelectInterface, indexes, string::upper(static_cast<std::string_view>(val)));
        }
        else if (key == "full_name") {
            filter_full_name(aChartSelectInterface, indexes, string::upper(static_cast<std::string_view>(val)));
        }
        else if (key == "vaccine" || key == "vaccines") {
            try {
                filter_vaccine(aChartSelectInterface, indexes,
                               VaccineMatchData{}
                               .type(rjson::get_or(val, "type", ""))
                               .passage(rjson::get_or(val, "passage", ""))
                               .no(rjson::get_or(val, "no", 0UL))
                               .name(rjson::get_or(val, "name", "")));
            }
            catch (std::bad_variant_access&) {
                  // std::cerr << "WARNING: filter_vaccine: bad_variant_access" << '\n';
                filter_vaccine(aChartSelectInterface, indexes, {});
            }
        }
        else if (key == "in_rectangle") {
            const auto& c1 = val["c1"];
            const auto& c2 = val["c2"];
            filter_rectangle(aChartSelectInterface, indexes, {c1[0], c1[1], c2[0], c2[1]});
        }
        else if (key == "in_circle") {
            const auto& center = val["center"];
            const double radius = val["radius"];
            filter_circle(aChartSelectInterface, indexes, {{center[0], center[1]}, radius});
        }
        else if (key == "relative_to_serum_line") {
            filter_relative_to_serum_line(aChartSelectInterface, indexes, rjson::get_or(val, "distance_min", 0.0), rjson::get_or(val, "distance_max", std::numeric_limits<double>::max()), rjson::get_or(val, "direction", 0));
        }
        else if (key == "lab") {
            if (aChartSelectInterface.chart().info()->lab(acmacs::chart::Info::Compute::Yes) != string::upper(static_cast<std::string_view>(val)))
                indexes.clear();
        }
        else if (key == "subtype") {
            const std::string virus_type = aChartSelectInterface.chart().info()->virus_type(acmacs::chart::Info::Compute::Yes);
            const std::string val_u = string::upper(static_cast<std::string_view>(val));
            if (val_u != virus_type) {
                bool clear_indexes = true;
                if (virus_type == "B") {
                    const std::string lineage = aChartSelectInterface.chart().lineage();
                    clear_indexes = !(((val_u == "BVIC" || val_u == "BV") && lineage == "VICTORIA") || ((val_u == "BYAM" || val_u == "BY") && lineage == "YAMAGATA"));
                }
                else {
                    clear_indexes = !((val_u == "H1" && virus_type == "A(H1N1)") || (val_u == "H3" && virus_type == "A(H3N2)"));
                }
                if (clear_indexes)
                    indexes.clear();
            }
        }
        else if (key == "found_in_previous") {
            if (!aChartSelectInterface.previous_chart())
                throw std::runtime_error{"\"found_in_previous\" selector used but no previous chart provided"};
            auto previous_antigens = aChartSelectInterface.previous_chart()->antigens();
            antigens->filter_found_in(indexes, *previous_antigens);
        }
        else if (key == "not_found_in_previous") {
            if (!aChartSelectInterface.previous_chart())
                throw std::runtime_error{"\"not_found_in_previous\" selector used but no previous chart provided"};
            auto previous_antigens = aChartSelectInterface.previous_chart()->antigens();
            antigens->filter_not_found_in(indexes, *previous_antigens);
        }
        else if (key == "table") {
            filter_table(aChartSelectInterface, indexes, val);
        }
        else {
            std::cerr << "WARNING: unrecognized key \"" << key << "\" in selector " << aSelector << '\n';
        }
    });
    if (verbose() && !indexes.empty()) {
        std::cerr << "INFO: antigens selected: " << std::setfill(' ') << std::setw(4) << indexes.size() << ' ' << aSelector << '\n';
        if (report_names_threshold() >= indexes.size()) {
            for (auto index: indexes)
                std::cerr << "  AG " << std::setw(5) << index << ' ' << (*antigens)[index]->full_name() << '\n';
        }
    }

    return indexes;

} // SelectAntigens::command

// ----------------------------------------------------------------------

const std::vector<seqdb::SeqdbEntrySeq>& SelectAntigensSera::seqdb_entries(const ChartSelectInterface& aChartSelectInterface)
{
      // thread unsafe!
    static std::map<const ChartSelectInterface*, std::vector<seqdb::SeqdbEntrySeq>> cache_seqdb_entries_for_chart;

    auto found = cache_seqdb_entries_for_chart.find(&aChartSelectInterface);
    if (found == cache_seqdb_entries_for_chart.end()) {
        found = cache_seqdb_entries_for_chart.emplace(&aChartSelectInterface, decltype(cache_seqdb_entries_for_chart)::mapped_type{}).first;
        seqdb::get(seqdb::ignore_errors::no, timer()).match(*aChartSelectInterface.chart().antigens(), found->second, aChartSelectInterface.chart().info()->virus_type(acmacs::chart::Info::Compute::Yes), seqdb::report::no);
    }
    return found->second;

} // SelectAntigensSera::seqdb_entries

// ----------------------------------------------------------------------

void SelectAntigens::filter_sequenced(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes)
{
    const auto& entries = seqdb_entries(aChartSelectInterface);
    auto not_sequenced = [&entries](auto index) -> bool { return !entries[index]; };
    indexes.erase(std::remove_if(indexes.begin(), indexes.end(), not_sequenced), indexes.end());

} // SelectAntigens::filter_sequenced

// ----------------------------------------------------------------------

void SelectAntigens::filter_not_sequenced(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes)
{
    const auto& entries = seqdb_entries(aChartSelectInterface);
    auto sequenced = [&entries](auto index) -> bool { return entries[index]; };
    indexes.erase(std::remove_if(indexes.begin(), indexes.end(), sequenced), indexes.end());

} // SelectAntigens::filter_not_sequenced

// ----------------------------------------------------------------------

std::map<std::string, size_t> SelectAntigens::clades(const ChartSelectInterface& aChartSelectInterface)
{
    std::map<std::string, size_t> result;
    for (const auto& entry: seqdb_entries(aChartSelectInterface)) {
        if (entry) {
            for (const auto& clade: entry.seq().clades())
                ++result[clade];
        }
    }
    return result;

} // SelectAntigens::clades

// ----------------------------------------------------------------------

void SelectAntigens::filter_clade(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string aClade)
{
    const auto& entries = seqdb_entries(aChartSelectInterface);
    auto not_in_clade = [&entries,aClade](auto index) -> bool { const auto& entry = entries[index]; return !entry || !entry.seq().has_clade(aClade); };
    indexes.erase(std::remove_if(indexes.begin(), indexes.end(), not_in_clade), indexes.end());

} // SelectAntigens::filter_clade

// ----------------------------------------------------------------------

void SelectAntigens::filter_amino_acid_at_pos(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, char amino_acid, size_t pos)
{
    const auto& entries = seqdb_entries(aChartSelectInterface);
    auto not_aa_at_pos = [&entries,amino_acid, pos](auto index) -> bool { const auto& entry = entries[index]; return !entry || entry.seq().amino_acid_at(pos) != amino_acid; };
    indexes.erase(std::remove_if(indexes.begin(), indexes.end(), not_aa_at_pos), indexes.end());

} // SelectAntigens::filter_amino_acid_at_pos

// ----------------------------------------------------------------------

void SelectAntigens::filter_outlier(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, double aUnits)
{
    auto layout = aChartSelectInterface.layout();

    using sum_count_t = std::pair<acmacs::Coordinates, size_t>;
    auto sum_count_not_nan = [&layout](const auto& sum_count, size_t index) -> sum_count_t {
        const auto coord = layout->get(index);
        auto [sum, count] = sum_count;
        if (coord.not_nan()) {
            sum += coord;
            ++count;
        }
        return {sum, count};
    };
    auto [centroid, count] = std::accumulate(indexes.begin(), indexes.end(), sum_count_t{{0.0, 0.0}, 0}, sum_count_not_nan);
    centroid /= count;
    // std::cerr << "centroid new: " << centroid << '\n';

    using point_dist_t = std::pair<size_t, double>; // point number (from indexes) and its distance to centroid
    std::vector<point_dist_t> point_dist(indexes.size());
    std::transform(indexes.begin(), indexes.end(), point_dist.begin(), [centroid=centroid,&layout](size_t index) -> point_dist_t {
        const auto coord = layout->get(index);
        if (coord.not_nan())
            return {index, centroid.distance(coord)};
        else
            return {index, 0.0}; // not an outlier!
    });
    std::sort(point_dist.begin(), point_dist.end(), [](const auto& a, const auto& b) { return a.second > b.second; }); // outliers first

    std::cout << "Outliers:" << '\n';
    for (const auto& [point_no, dist]: point_dist)
        std::cout << std::setfill(' ') << std::setw(8) << point_no << ' ' << dist << '\n';

    auto not_outlier = [&point_dist,aUnits](size_t index) { const auto& [_, d] = *std::find_if(point_dist.begin(), point_dist.end(), [index](const auto& pd) { return pd.first == index; }); return d < aUnits; };
    indexes.erase(std::remove_if(indexes.begin(), indexes.end(), not_outlier), indexes.end());

} // SelectAntigens::filter_outlier

// ----------------------------------------------------------------------

void SelectAntigens::filter_vaccine(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const VaccineMatchData& aMatchData)
{
    const auto virus_type = aChartSelectInterface.chart().info()->virus_type(acmacs::chart::Info::Compute::Yes);
    if (!virus_type.empty()) {

          // thread unsafe!
        static std::map<const ChartSelectInterface*, Vaccines> cache_vaccines;
        auto vaccines_of_chart = cache_vaccines.find(&aChartSelectInterface);
        if (vaccines_of_chart == cache_vaccines.end()) {
            Timeit ti_vaccines("Vaccines of chart: ");
            vaccines_of_chart = cache_vaccines.emplace(&aChartSelectInterface, Vaccines{aChartSelectInterface.chart(), verbose()}).first;
            if (verbose())
                std::cerr << vaccines_of_chart->second.report(hidb::Vaccines::ReportConfig{}.indent(2)) << '\n';
        }
        auto vaccine_indexes = vaccines_of_chart->second.indices(aMatchData);

        std::sort(vaccine_indexes.begin(), vaccine_indexes.end());
        acmacs::chart::Indexes result(vaccine_indexes.size());
        const auto end = std::set_intersection(indexes.begin(), indexes.end(), vaccine_indexes.begin(), vaccine_indexes.end(), result.begin());
        indexes.erase(std::copy(result.begin(), end, indexes.begin()), indexes.end());
    }
    else {
        indexes.clear();
        std::cerr << "WARNING: unknown virus_type for chart: " << aChartSelectInterface.chart().make_name() << '\n';
    }

} // SelectAntigens::filter_vaccine

// ----------------------------------------------------------------------

acmacs::chart::Indexes SelectSera::command(const ChartSelectInterface& aChartSelectInterface, const rjson::value& aSelector)
{
    const auto& sera = aChartSelectInterface.chart().sera();
    auto indexes = sera->all_indexes();
    rjson::for_each(aSelector, [this,&aChartSelectInterface,&sera,&indexes,&aSelector](const rjson::object::value_type& key_value) {
        const auto& [key, val] = key_value;
        if (!key.empty() && (key.front() == '?' || key.back() == '?')) {
              // comment
        }
        else if (key == "all") {
              // do nothing
        }
        else if (key == "serum_id") {
            sera->filter_serum_id(indexes, string::upper(static_cast<std::string_view>(val)));
        }
        else if (key == "index") {
            indexes.erase_except(val);
        }
        else if (key == "indexes") {
            acmacs::chart::Indexes to_keep(val.size());
            rjson::transform(val, to_keep.begin(), [](const rjson::value& v) -> size_t { return v; });
            indexes.erase(std::remove_if(indexes.begin(), indexes.end(), [&to_keep](auto index) -> bool { return std::find(to_keep.begin(), to_keep.end(), index) == to_keep.end(); }), indexes.end());
        }
        else if (key == "clade") {
            filter_clade(aChartSelectInterface, indexes, string::upper(static_cast<std::string_view>(val)));
        }
        else if (key == "country") {
            sera->filter_country(indexes, string::upper(static_cast<std::string_view>(val)));
        }
        else if (key == "continent") {
            sera->filter_continent(indexes, string::upper(static_cast<std::string_view>(val)));
        }
        else if (key == "name") {
            filter_name(aChartSelectInterface, indexes, string::upper(static_cast<std::string_view>(val)));
        }
        else if (key == "full_name") {
            filter_full_name(aChartSelectInterface, indexes, string::upper(static_cast<std::string_view>(val)));
        }
        else if (key == "table") {
            filter_table(aChartSelectInterface, indexes, static_cast<std::string_view>(val));
        }
        else if (key == "in_rectangle") {
            const auto& c1 = val["c1"];
            const auto& c2 = val["c2"];
            filter_rectangle(aChartSelectInterface, indexes, {c1[0], c1[1], c2[0], c2[1]});
        }
        else if (key == "in_circle") {
            const auto& center = val["center"];
            const double radius = val["radius"];
            filter_circle(aChartSelectInterface, indexes, {{center[0], center[1]}, radius});
        }
        else {
            std::cerr << "WARNING: unrecognized key \"" << key << "\" in selector " << aSelector << '\n';
        }
    });
    if (verbose()) {
        std::cerr << "Sera selected: " << std::setfill(' ') << std::setw(4) << indexes.size() << ' ' << aSelector << '\n';
        if (report_names_threshold() >= indexes.size()) {
            for (auto index: indexes)
                std::cerr << "  SR " << std::setw(5) << index << ' ' << (*sera)[index]->full_name() << '\n';
        }
    }
    return indexes;

} // SelectSera::command

// ----------------------------------------------------------------------

template <typename AgSr> void filter_table_ag_sr(const AgSr& aAgSr, acmacs::chart::Indexes& indexes, std::string_view aTable, std::shared_ptr<hidb::Tables> aHidbTables)
{
    auto not_in_table = [aTable,&aAgSr,hidb_tables=*aHidbTables](size_t index) {
        if (auto ag_sr = aAgSr[index]; ag_sr) { // found in hidb
            for (auto table_no: ag_sr->tables()) {
                auto table = hidb_tables[table_no];
                if (table->date() == aTable || table->name() == aTable)
                    return false;
            }
        }
        return true;
    };
    indexes.erase(std::remove_if(indexes.begin(), indexes.end(), not_in_table), indexes.end());
}

void SelectAntigens::filter_table(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aTable)
{
    const auto& hidb = hidb::get(aChartSelectInterface.chart().info()->virus_type());
    filter_table_ag_sr(hidb.antigens()->find(*aChartSelectInterface.chart().antigens()), indexes, aTable, hidb.tables());

} // SelectAntigens::filter_table

// ----------------------------------------------------------------------

void SelectSera::filter_table(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aTable)
{
    const auto& hidb = hidb::get(aChartSelectInterface.chart().info()->virus_type());
    filter_table_ag_sr(hidb.sera()->find(*aChartSelectInterface.chart().sera()), indexes, aTable, hidb.tables());

} // SelectSera::filter_table

// ----------------------------------------------------------------------

void SelectSera::filter_clade(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string aClade)
{
    const auto& chart = aChartSelectInterface.chart();
    chart.set_homologous(acmacs::chart::Chart::find_homologous_for_big_chart::yes);
    const auto& entries = seqdb_entries(aChartSelectInterface);
    auto homologous_not_in_clade = [&entries, aClade, &chart](auto serum_index) -> bool {
        for (auto antigen_index : chart.sera()->at(serum_index)->homologous_antigens()) {
            if (const auto& entry = entries[antigen_index]; entry && entry.seq().has_clade(aClade))
                return false;
        }
        return true;
    };
    indexes.erase(std::remove_if(indexes.begin(), indexes.end(), homologous_not_in_clade), indexes.end());

} // SelectSera::filter_clade

// ----------------------------------------------------------------------

void SelectAntigens::filter_relative_to_serum_line(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, double distance_min, double distance_max, int direction)
{
    acmacs::chart::SerumLine serum_line(aChartSelectInterface.projection());
    auto layout = aChartSelectInterface.layout();

    auto not_relative_to_line = [&serum_line, &layout, distance_min, distance_max, direction](auto antigen_no) -> bool {
        const auto distance = serum_line.line().distance_with_direction(layout->get(antigen_no));
        return std::abs(distance) < distance_min || std::abs(distance) > distance_max || (direction != 0 && (direction * distance) < 0);
    };
    indexes.erase(std::remove_if(indexes.begin(), indexes.end(), not_relative_to_line), indexes.end());

} // SelectAntigens::filter_relative_to_serum_line

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
