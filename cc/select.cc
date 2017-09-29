#include <chrono>
#include <cstdlib>

#include "acmacs-chart/chart.hh"
#include "locationdb/locdb.hh"

#include "select.hh"

using namespace std::string_literals;

// ----------------------------------------------------------------------

SelectAntigensSera::SelectAntigensSera(std::string aLocDbFilename, std::string aHidbDir, std::string aSeqdbFilename)
    : mLocDbFilename{aLocDbFilename}, mHidbDir{aHidbDir}, mSeqdbFilename{aSeqdbFilename}
{
    if (mLocDbFilename.empty())
        mLocDbFilename = std::getenv("HOME") + "/AD/data/locationdb.json.xz"s;
    if (mHidbDir.empty())
        mHidbDir = std::getenv("HOME") + "/AD/data"s;
    if (mSeqdbFilename.empty())
        mSeqdbFilename = std::getenv("HOME") + "/AD/data/seqdb.json.xz"s;
}

// ----------------------------------------------------------------------

SelectAntigensSera::~SelectAntigensSera()
{
} // SelectAntigensSera::~SelectAntigensSera

// ----------------------------------------------------------------------

const LocDb& SelectAntigensSera::get_location_database() const
{
    return ::get_location_database(mLocDbFilename);

} // SelectAntigensSera::get_location_database

// ----------------------------------------------------------------------

const seqdb::Seqdb& SelectAntigensSera::get_seqdb() const
{
    return seqdb::get(mSeqdbFilename);

} // SelectAntigensSera::get_seqdb

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
    catch (std::exception&) {
          // catch (SelectorVisitor::unexpected_value&) {
        throw std::runtime_error{"Unsupported selector value: " + aSelector.to_json()};
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
        if (key == "all") {
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
            const std::string country = string::upper(value);
            antigens.filter_country(indices, country, get_location_database());
        }
        else if (key == "continent") {
            const std::string continent = string::upper(value);
            antigens.filter_continent(indices, continent, get_location_database());
        }
        else if (key == "sequenced") {
            filter_sequenced(aChart, indices);
        }
        else if (key == "not_sequenced") {
            filter_not_sequenced(aChart, indices);
        }
        else if (key == "clade") {
            const std::string clade = value;
            filter_clade(aChart, indices, value);
        }
        else {
            std::cerr << "WARNING: unrecognized key \"" << key << "\" in selector " << aSelector << '\n';
        }
    }
    return indices;

} // SelectAntigens::command

// ----------------------------------------------------------------------

const std::vector<seqdb::SeqdbEntrySeq>& SelectAntigens::seqdb_entries(const Chart& aChart)
{
    if (!mSeqdbEntries || mChartForSeqdbEntries != &aChart) {
        mSeqdbEntries = std::make_unique<std::vector<seqdb::SeqdbEntrySeq>>();
        get_seqdb().match(aChart.antigens(), *mSeqdbEntries, true);
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

std::vector<size_t> SelectSera::command(const Chart& aChart, const rjson::object& aSelector)
{
    std::cout << "DEBUG: sera command: " << aSelector << '\n';
    return {};

} // SelectSera::command

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
