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


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
