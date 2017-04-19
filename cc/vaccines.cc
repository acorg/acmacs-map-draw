#include "vaccines.hh"
#include "acmacs-chart/chart.hh"

// ----------------------------------------------------------------------

Vaccines::Vaccines(const Chart& aChart, const hidb::HiDb& aHiDb)
{
    hidb::vaccines(mVaccinesOfChart, aChart, aHiDb);
    for (size_t vaccines_of_chart_index = 0; vaccines_of_chart_index < mVaccinesOfChart.size(); ++vaccines_of_chart_index) {
        auto update = [&](hidb::Vaccines::PassageType pt) {
            if (!mVaccinesOfChart[vaccines_of_chart_index].empty(pt)) {
                mEntries.emplace_back(vaccines_of_chart_index, pt);
            }
        };
        hidb::Vaccines::for_each_passage_type(update);
    }

} // Vaccines::Vaccines

// ----------------------------------------------------------------------

std::string Vaccines::report(size_t aIndent) const
{
    std::string result;
    for (const auto& entry: mEntries) {
        if (entry.show) {
            const auto& vacc = mVaccinesOfChart[entry.vaccines_of_chart_index];
            const std::string s = vacc.report(entry.passage_type, aIndent, entry.antigen_no);
            if (!s.empty())
                result += std::string(aIndent, ' ') + vacc.type() + " " + vacc.name() + s.substr(aIndent - 1);
        }
    }
    return result;

} // Vaccines::report

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
