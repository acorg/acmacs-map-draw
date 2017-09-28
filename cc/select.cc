#include <chrono>

#include "acmacs-chart/chart.hh"

#include "select.hh"

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
