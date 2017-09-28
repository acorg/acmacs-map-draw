#include "acmacs-chart/chart.hh"

#include "select.hh"

// ----------------------------------------------------------------------

SelectAntigensSera::~SelectAntigensSera()
{
} // SelectAntigensSera::~SelectAntigensSera

// ----------------------------------------------------------------------

std::vector<size_t> SelectAntigens::select(const Chart& aChart, const rjson::value& aSelector)
{
    return {};

} // SelectAntigens::select

// ----------------------------------------------------------------------

std::vector<size_t> SelectSera::select(const Chart& aChart, const rjson::value& aSelector)
{
    return {};

} // SelectSera::select

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
