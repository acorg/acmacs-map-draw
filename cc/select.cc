#include <chrono>
#include <cstdlib>

#include "acmacs-chart/chart.hh"
#include "locationdb/locdb.hh"
#include "hidb/hidb.hh"

#include "vaccines.hh"
#include "select.hh"

using namespace std::string_literals;

// ----------------------------------------------------------------------

SelectAntigensSera::~SelectAntigensSera()
{
} // SelectAntigensSera::~SelectAntigensSera

// ----------------------------------------------------------------------

class SelectorVisitor : public rjson::value_visitor_base<std::vector<size_t>>
{
 public:
    inline SelectorVisitor(const Chart& aChart, SelectAntigensSera& aSelect) : mChart{aChart}, mSelect{aSelect} {}
    using rjson::value_visitor_base<std::vector<size_t>>::operator();

    inline std::vector<size_t> operator()(const rjson::string& aValue) override
        {
            return mSelect.command(mChart, {{aValue, rjson::boolean{true}}});
        }

    inline std::vector<size_t> operator()(const rjson::object& aValue) override
        {
            return mSelect.command(mChart, aValue);
        }

 private:
    const Chart& mChart;
    SelectAntigensSera& mSelect;

}; // class SelectorVisitor

// ----------------------------------------------------------------------

std::vector<size_t> SelectAntigensSera::select(const Chart& aChart, const rjson::value& aSelector)
{
    try {
        return std::visit(SelectorVisitor{aChart, *this}, aSelector);
    }
    catch (std::exception& err) {
          // catch (SelectorVisitor::unexpected_value&) {
        throw std::runtime_error{"Unsupported selector value: " + aSelector.to_json() + ": " + err.what()};
    }

} // SelectAntigensSera::select

// ----------------------------------------------------------------------

std::vector<size_t> SelectAntigens::command(const Chart& aChart, const rjson::object& aSelector)
{
    using namespace std::chrono_literals;
      // std::cout << "DEBUG: antigens command: " << aSelector << '\n';
    const auto& antigens = aChart.antigens();
    auto indices = antigens.all_indices();
    for (const auto& [key, value]: aSelector) {
        if (!key.empty() && (key.front() == '?' || key.back() == '?')) {
              // comment
        }
        else if (key == "all") {
              // do nothing
        }
        else if (key == "reference") {
            antigens.filter_reference(indices);
        }
        else if (key == "test") {
            antigens.filter_test(indices);
        }
        else if (key == "egg") {
            antigens.filter_egg(indices);
        }
        else if (key == "cell") {
            antigens.filter_cell(indices);
        }
        else if (key == "reassortant") {
            antigens.filter_reassortant(indices);
        }
        else if (key == "passage") {
            const std::string passage = value;
            if (passage == "egg")
                antigens.filter_egg(indices);
            else if (passage == "cell")
                antigens.filter_cell(indices);
            else if (passage == "reassortant")
                antigens.filter_reassortant(indices);
            else
                throw std::exception{};
        }
        else if (key == "date_range") {
            const rjson::array& dr = value;
            antigens.filter_date_range(indices, dr[0], dr[1]);
        }
        else if (key == "older_than_days") {
            const int days = value;
            const auto then = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() - 24h * days);
            char buffer[20];
            std::strftime(buffer, sizeof buffer, "%Y-%m-%d", std::localtime(&then));
            antigens.filter_date_range(indices, "", buffer);
        }
        else if (key == "younger_than_days") {
            const int days = value;
            const auto then = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() - 24h * days);
            char buffer[20];
            std::strftime(buffer, sizeof buffer, "%Y-%m-%d", std::localtime(&then));
            antigens.filter_date_range(indices, buffer, "");
        }
        else if (key == "index") {
            const size_t index = value;
            if (std::find(indices.begin(), indices.end(), index) == indices.end()) {
                indices.clear();
            }
            else {
                indices.clear();
                indices.push_back(index);
            }
        }
        else if (key == "indices") {
            const rjson::array& to_keep_v = value;
            std::vector<size_t> to_keep(to_keep_v.size());
            std::transform(to_keep_v.begin(), to_keep_v.end(), to_keep.begin(), [](const auto& v) -> size_t { return v; });
            indices.erase(std::remove_if(indices.begin(), indices.end(), [&to_keep](auto index) -> bool { return std::find(to_keep.begin(), to_keep.end(), index) == to_keep.end(); }), indices.end());
        }
        else if (key == "country") {
            antigens.filter_country(indices, string::upper(value));
        }
        else if (key == "continent") {
            antigens.filter_continent(indices, string::upper(value));
        }
        else if (key == "sequenced") {
            filter_sequenced(aChart, indices);
        }
        else if (key == "not_sequenced") {
            filter_not_sequenced(aChart, indices);
        }
        else if (key == "clade") {
            filter_clade(aChart, indices, value); //string::upper(value));
        }
        else if (key == "name") {
            filter_name(aChart, indices, string::upper(value));
        }
        else if (key == "full_name") {
            filter_full_name(aChart, indices, string::upper(value));
        }
        else if (key == "vaccine" || key == "vaccines") {
            try {
                filter_vaccine(aChart, indices,
                               VaccineMatchData{}
                               .type(value.get_or_default("type", ""s))
                               .passage(value.get_or_default("passage", ""s))
                               .no(value.get_or_default("no", 0U))
                               .name(value.get_or_default("name", ""s)));
            }
            catch (std::bad_variant_access&) {
                filter_vaccine(aChart, indices, {});
            }
        }
        else if (key == "in_rectangle") {
            // const auto& c1 = value.get_field<rjson::array>("c1");
            // const auto& c2 = value.get_field<rjson::array>("c2");
            const auto& c1 = value["c1"];
            const auto& c2 = value["c2"];
            const size_t projection_no = 0;
            filter_rectangle(aChart, indices, aChart.projection(projection_no), {c1[0], c1[1], c2[0], c2[1]});
        }
        else if (key == "in_circle") {
            // const auto& center = value.get_field<rjson::array>("center");
            // const auto radius = value.get_field_number("radius");
            const auto& center = value["center"];
            const double radius = value["radius"];
            const size_t projection_no = 0;
            filter_circle(aChart, indices, aChart.projection(projection_no), {center[0], center[1], radius});
        }
        else {
            std::cerr << "WARNING: unrecognized key \"" << key << "\" in selector " << aSelector << '\n';
        }
    }
    if (verbose())
        std::cerr << "Antigens selected: " << std::setfill(' ') << std::setw(4) << indices.size() << ' ' << aSelector << '\n';

    return indices;

} // SelectAntigens::command

// ----------------------------------------------------------------------

const std::vector<seqdb::SeqdbEntrySeq>& SelectAntigens::seqdb_entries(const Chart& aChart)
{
    if (!mSeqdbEntries || mChartForSeqdbEntries != &aChart) {
        mSeqdbEntries = std::make_unique<std::vector<seqdb::SeqdbEntrySeq>>();
        seqdb::get(timer()).match(aChart.antigens(), *mSeqdbEntries, false);
    }
    return *mSeqdbEntries;

} // SelectAntigens::seqdb_entries

// ----------------------------------------------------------------------

void SelectAntigens::filter_sequenced(const Chart& aChart, std::vector<size_t>& indices)
{
    const auto& entries = seqdb_entries(aChart);
    auto not_sequenced = [&entries](auto index) -> bool { return !entries[index]; };
    indices.erase(std::remove_if(indices.begin(), indices.end(), not_sequenced), indices.end());

} // SelectAntigens::filter_sequenced

// ----------------------------------------------------------------------

void SelectAntigens::filter_not_sequenced(const Chart& aChart, std::vector<size_t>& indices)
{
    const auto& entries = seqdb_entries(aChart);
    auto sequenced = [&entries](auto index) -> bool { return entries[index]; };
    indices.erase(std::remove_if(indices.begin(), indices.end(), sequenced), indices.end());

} // SelectAntigens::filter_not_sequenced

// ----------------------------------------------------------------------

std::map<std::string, size_t> SelectAntigens::clades(const Chart& aChart)
{
    std::map<std::string, size_t> result;
    for (const auto& entry: seqdb_entries(aChart)) {
        if (entry) {
            for (const auto& clade: entry.seq().clades())
                ++result[clade];
        }
    }
    return result;

} // SelectAntigens::clades

// ----------------------------------------------------------------------

void SelectAntigens::filter_clade(const Chart& aChart, std::vector<size_t>& indices, std::string aClade)
{
    const auto& entries = seqdb_entries(aChart);
    auto not_in_clade = [&entries,aClade](auto index) -> bool { const auto& entry = entries[index]; return !entry || !entry.seq().has_clade(aClade); };
    indices.erase(std::remove_if(indices.begin(), indices.end(), not_in_clade), indices.end());

} // SelectAntigens::filter_clade

// ----------------------------------------------------------------------

void SelectAntigens::filter_vaccine(const Chart& aChart, std::vector<size_t>& indices, const VaccineMatchData& aMatchData)
{
    const auto virus_type = aChart.chart_info().virus_type();
    if (!virus_type.empty()) {
        Vaccines vaccines(aChart, verbose());
        if (verbose())
            std::cerr << vaccines.report(2) << '\n';
        auto vaccine_indices = vaccines.indices(aMatchData);
        std::sort(vaccine_indices.begin(), vaccine_indices.end());
        std::vector<size_t> result(vaccine_indices.size());
        const auto end = std::set_intersection(indices.begin(), indices.end(), vaccine_indices.begin(), vaccine_indices.end(), result.begin());
        indices.erase(std::copy(result.begin(), end, indices.begin()), indices.end());
    }
    else {
        indices.clear();
        std::cerr << "WARNING: unknown virus_type for chart: " << aChart.make_name() << '\n';
    }

} // SelectAntigens::filter_vaccine

// ----------------------------------------------------------------------

std::vector<size_t> SelectSera::command(const Chart& aChart, const rjson::object& aSelector)
{
    const auto& sera = aChart.sera();
    auto indices = sera.all_indices();
    for (const auto& [key, value]: aSelector) {
        if (!key.empty() && (key.front() == '?' || key.back() == '?')) {
              // comment
        }
        else if (key == "all") {
              // do nothing
        }
        else if (key == "serum_id") {
            sera.filter_serum_id(indices, string::upper(value));
        }
        else if (key == "index") {
            const size_t index = value;
            if (std::find(indices.begin(), indices.end(), index) == indices.end()) {
                indices.clear();
            }
            else {
                indices.clear();
                indices.push_back(index);
            }
        }
        else if (key == "indices") {
            const rjson::array& to_keep_v = value;
            std::vector<size_t> to_keep(to_keep_v.size());
            std::transform(to_keep_v.begin(), to_keep_v.end(), to_keep.begin(), [](const auto& v) -> size_t { return v; });
            indices.erase(std::remove_if(indices.begin(), indices.end(), [&to_keep](auto index) -> bool { return std::find(to_keep.begin(), to_keep.end(), index) == to_keep.end(); }), indices.end());
        }
        else if (key == "country") {
            sera.filter_country(indices, string::upper(value));
        }
        else if (key == "continent") {
            sera.filter_continent(indices, string::upper(value));
        }
        else if (key == "name") {
            filter_name(aChart, indices, string::upper(value));
        }
        else if (key == "full_name") {
            filter_full_name(aChart, indices, string::upper(value));
        }
        else if (key == "in_rectangle") {
            const auto& c1 = value["c1"];
            const auto& c2 = value["c2"];
            const size_t projection_no = 0;
            filter_rectangle(aChart, indices, aChart.projection(projection_no), {c1[0], c1[1], c2[0], c2[1]});
        }
        else if (key == "in_circle") {
            // const auto& center = value.get_field<rjson::array>("center");
            // const auto radius = value.get_field_number("radius");
            const auto& center = value["center"];
            const double radius = value["radius"];
            const size_t projection_no = 0;
            filter_circle(aChart, indices, aChart.projection(projection_no), {center[0], center[1], radius});
        }
        else {
            std::cerr << "WARNING: unrecognized key \"" << key << "\" in selector " << aSelector << '\n';
        }
    }
    if (verbose())
        std::cerr << "Sera selected: " << indices.size() << '\n';
    return indices;

} // SelectSera::command

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
