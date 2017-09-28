#include <chrono>
#include <cstdlib>

#include "acmacs-chart/chart.hh"
#include "locationdb/locdb.hh"

#include "select.hh"

using namespace std::string_literals;

// ----------------------------------------------------------------------

SelectAntigensSera::SelectAntigensSera(std::string aLocDbFilename)
    : mLocDbFilename{aLocDbFilename}
{
    if (mLocDbFilename.empty())
        mLocDbFilename = std::getenv("HOME") + "/AD/data/locationdb.json.xz"s;
}

// ----------------------------------------------------------------------

SelectAntigensSera::~SelectAntigensSera()
{
} // SelectAntigensSera::~SelectAntigensSera

// ----------------------------------------------------------------------

const LocDb& SelectAntigensSera::get_location_database() const
{
    return ::get_location_database(mLocDbFilename, true);

} // SelectAntigensSera::get_location_database

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
        else {
            std::cerr << "WARNING: unrecognized key \"" << key << "\" in selector " << aSelector << '\n';
        }
    }
    return indices;

} // SelectAntigens::command

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
