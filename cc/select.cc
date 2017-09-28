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
    catch (SelectorVisitor::unexpected_value&) {
        throw std::runtime_error{"Unsupported selector value: " + aSelector.to_json()};
    }

} // SelectAntigensSera::select

// ----------------------------------------------------------------------

std::vector<size_t> SelectAntigens::command(const Chart& aChart, const rjson::object& aSelector)
{
    std::cout << "DEBUG: antigens command: " << aSelector << '\n';
    for (const auto& [key, value]: aSelector) {
        if (key == "xall") {
        }
        else {
            std::cerr << "WARNING: unrecognized key \"" << key << "\" in selector " << aSelector << '\n';
        }
    }
    return {};

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
