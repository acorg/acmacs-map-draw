#include <chrono>
#include <cstdlib>

#include "locationdb/locdb.hh"
#include "hidb-5/hidb.hh"

#include "vaccines.hh"
#include "select.hh"

using namespace std::string_literals;

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
//#pragma GCC diagnostic ignored "-Wglobal-constructors"
#endif

// ----------------------------------------------------------------------

SelectAntigensSera::~SelectAntigensSera()
{
} // SelectAntigensSera::~SelectAntigensSera

// ----------------------------------------------------------------------

class SelectorVisitor : public rjson::value_visitor_base<acmacs::chart::Indexes>
{
 public:
    inline SelectorVisitor(const ChartSelectInterface& aChartSelectInterface, SelectAntigensSera& aSelect) : mChartSelectInterface{aChartSelectInterface}, mSelect{aSelect} {}
    using rjson::value_visitor_base<acmacs::chart::Indexes>::operator();

    inline acmacs::chart::Indexes operator()(const rjson::string& aValue) override
        {
            return mSelect.command(mChartSelectInterface, {{aValue, rjson::boolean{true}}});
        }

    inline acmacs::chart::Indexes operator()(const rjson::object& aValue) override
        {
            return mSelect.command(mChartSelectInterface, aValue);
        }

 private:
    const ChartSelectInterface& mChartSelectInterface;
    SelectAntigensSera& mSelect;

}; // class SelectorVisitor

// ----------------------------------------------------------------------

acmacs::chart::Indexes SelectAntigensSera::select(const ChartSelectInterface& aChartSelectInterface, const rjson::value& aSelector)
{
    try {
        return std::visit(SelectorVisitor{aChartSelectInterface, *this}, aSelector);
    }
    catch (std::exception& err) {
          // catch (SelectorVisitor::unexpected_value&) {
        throw std::runtime_error{"Unsupported selector value: " + aSelector.to_json() + ": " + err.what()};
    }

} // SelectAntigensSera::select

// ----------------------------------------------------------------------

acmacs::chart::Indexes SelectAntigens::command(const ChartSelectInterface& aChartSelectInterface, const rjson::object& aSelector)
{
      // std::cout << "DEBUG: antigens command: " << aSelector << '\n';
    auto antigens = aChartSelectInterface.chart().antigens();
    auto indexes = antigens->all_indexes();
    for (const auto& [key, value]: aSelector) {
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
            const std::string passage = value;
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
            const rjson::array& dr = value;
            antigens->filter_date_range(indexes, dr[0], dr[1]);
        }
        else if (key == "older_than_days") {
            using namespace std::chrono_literals;
            const int days = value;
            const auto then = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() - 24h * days);
            char buffer[20];
            std::strftime(buffer, sizeof buffer, "%Y-%m-%d", std::localtime(&then));
            antigens->filter_date_range(indexes, "", buffer);
        }
        else if (key == "younger_than_days") {
            using namespace std::chrono_literals;
            const int days = value;
            const auto then = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() - 24h * days);
            char buffer[20];
            std::strftime(buffer, sizeof buffer, "%Y-%m-%d", std::localtime(&then));
            antigens->filter_date_range(indexes, buffer, "");
        }
        else if (key == "index") {
            const size_t index = value;
            if (std::find(indexes.begin(), indexes.end(), index) == indexes.end()) {
                indexes.clear();
            }
            else {
                indexes.clear();
                indexes.push_back(index);
            }
        }
        else if (key == "indexes") {
            const rjson::array& to_keep_v = value;
            acmacs::chart::Indexes to_keep(to_keep_v.size());
            std::transform(to_keep_v.begin(), to_keep_v.end(), to_keep.begin(), [](const auto& v) -> size_t { return v; });
            indexes.erase(std::remove_if(indexes.begin(), indexes.end(), [&to_keep](auto index) -> bool { return std::find(to_keep.begin(), to_keep.end(), index) == to_keep.end(); }), indexes.end());
        }
        else if (key == "country") {
            antigens->filter_country(indexes, string::upper(static_cast<std::string_view>(value)));
        }
        else if (key == "continent") {
            antigens->filter_continent(indexes, string::upper(static_cast<std::string_view>(value)));
        }
        else if (key == "sequenced") {
            filter_sequenced(aChartSelectInterface, indexes);
        }
        else if (key == "not_sequenced") {
            filter_not_sequenced(aChartSelectInterface, indexes);
        }
        else if (key == "clade") {
            filter_clade(aChartSelectInterface, indexes, string::upper(static_cast<std::string_view>(value)));
        }
        else if (key == "outlier") {
            filter_outlier(aChartSelectInterface, indexes, value);
        }
        else if (key == "name") {
            filter_name(aChartSelectInterface, indexes, string::upper(static_cast<std::string_view>(value)));
        }
        else if (key == "full_name") {
            filter_full_name(aChartSelectInterface, indexes, string::upper(static_cast<std::string_view>(value)));
        }
        else if (key == "vaccine" || key == "vaccines") {
            try {
                filter_vaccine(aChartSelectInterface, indexes,
                               VaccineMatchData{}
                               .type(value.get_or_default("type", ""s))
                               .passage(value.get_or_default("passage", ""s))
                               .no(value.get_or_default("no", 0U))
                               .name(value.get_or_default("name", ""s)));
            }
            catch (std::bad_variant_access&) {
                filter_vaccine(aChartSelectInterface, indexes, {});
            }
        }
        else if (key == "in_rectangle") {
            // const auto& c1 = value.get_field<rjson::array>("c1");
            // const auto& c2 = value.get_field<rjson::array>("c2");
            const auto& c1 = value["c1"];
            const auto& c2 = value["c2"];
            filter_rectangle(aChartSelectInterface, indexes, {c1[0], c1[1], c2[0], c2[1]});
        }
        else if (key == "in_circle") {
            // const auto& center = value.get_field<rjson::array>("center");
            // const auto radius = value.get_field_number("radius");
            const auto& center = value["center"];
            const double radius = value["radius"];
            filter_circle(aChartSelectInterface, indexes, {center[0], center[1], radius});
        }
        else if (key == "lab") {
            if (aChartSelectInterface.chart().info()->lab(acmacs::chart::Info::Compute::Yes) != string::upper(static_cast<std::string_view>(value)))
                indexes.clear();
        }
        else if (key == "subtype") {
            const std::string virus_type = aChartSelectInterface.chart().info()->virus_type(acmacs::chart::Info::Compute::Yes);
            const std::string val_u = string::upper(static_cast<std::string_view>(value));
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
        else {
            std::cerr << "WARNING: unrecognized key \"" << key << "\" in selector " << aSelector << '\n';
        }
    }
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

const std::vector<seqdb::SeqdbEntrySeq>& SelectAntigens::seqdb_entries(const ChartSelectInterface& aChartSelectInterface)
{
      // thread unsafe!
    static std::map<const ChartSelectInterface*, std::vector<seqdb::SeqdbEntrySeq>> cache_seqdb_entries_for_chart;

    auto found = cache_seqdb_entries_for_chart.find(&aChartSelectInterface);
    if (found == cache_seqdb_entries_for_chart.end()) {
        found = cache_seqdb_entries_for_chart.emplace(&aChartSelectInterface, decltype(cache_seqdb_entries_for_chart)::mapped_type{}).first;
        seqdb::get(timer()).match(*aChartSelectInterface.chart().antigens(), found->second, aChartSelectInterface.chart().info()->virus_type(acmacs::chart::Info::Compute::Yes), false);
    }
    return found->second;

} // SelectAntigens::seqdb_entries

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

void SelectAntigens::filter_outlier(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indices, double aUnits)
{
    // auto layout = aChartSelectInterface.layout();
    // std::vector<acmacs::Coordinates> points(indices.size());
    // std::transform(indices.begin(), indices.end(), points.begin(), [&layout](size_t p_no) { return (*layout)[p_no]; });
    // std::cerr << "filter_outliers " << indices << '\n';

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
            Timeit ti_vaccines("Vaccines of chart ");
            vaccines_of_chart = cache_vaccines.emplace(&aChartSelectInterface, Vaccines{aChartSelectInterface.chart(), verbose()}).first;
            if (verbose())
                std::cerr << vaccines_of_chart->second.report(2) << '\n';
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

acmacs::chart::Indexes SelectSera::command(const ChartSelectInterface& aChartSelectInterface, const rjson::object& aSelector)
{
    const auto& sera = aChartSelectInterface.chart().sera();
    auto indexes = sera->all_indexes();
    for (const auto& [key, value]: aSelector) {
        if (!key.empty() && (key.front() == '?' || key.back() == '?')) {
              // comment
        }
        else if (key == "all") {
              // do nothing
        }
        else if (key == "serum_id") {
            sera->filter_serum_id(indexes, string::upper(static_cast<std::string_view>(value)));
        }
        else if (key == "index") {
            const size_t index = value;
            if (std::find(indexes.begin(), indexes.end(), index) == indexes.end()) {
                indexes.clear();
            }
            else {
                indexes.clear();
                indexes.push_back(index);
            }
        }
        else if (key == "indexes") {
            const rjson::array& to_keep_v = value;
            acmacs::chart::Indexes to_keep(to_keep_v.size());
            std::transform(to_keep_v.begin(), to_keep_v.end(), to_keep.begin(), [](const auto& v) -> size_t { return v; });
            indexes.erase(std::remove_if(indexes.begin(), indexes.end(), [&to_keep](auto index) -> bool { return std::find(to_keep.begin(), to_keep.end(), index) == to_keep.end(); }), indexes.end());
        }
        else if (key == "country") {
            sera->filter_country(indexes, string::upper(static_cast<std::string_view>(value)));
        }
        else if (key == "continent") {
            sera->filter_continent(indexes, string::upper(static_cast<std::string_view>(value)));
        }
        else if (key == "name") {
            filter_name(aChartSelectInterface, indexes, string::upper(static_cast<std::string_view>(value)));
        }
        else if (key == "full_name") {
            filter_full_name(aChartSelectInterface, indexes, string::upper(static_cast<std::string_view>(value)));
        }
        else if (key == "in_rectangle") {
            const auto& c1 = value["c1"];
            const auto& c2 = value["c2"];
            filter_rectangle(aChartSelectInterface, indexes, {c1[0], c1[1], c2[0], c2[1]});
        }
        else if (key == "in_circle") {
            // const auto& center = value.get_field<rjson::array>("center");
            // const auto radius = value.get_field_number("radius");
            const auto& center = value["center"];
            const double radius = value["radius"];
            filter_circle(aChartSelectInterface, indexes, {center[0], center[1], radius});
        }
        else {
            std::cerr << "WARNING: unrecognized key \"" << key << "\" in selector " << aSelector << '\n';
        }
    }
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


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
