#include "acmacs-base/range-v3.hh"
#include "acmacs-chart-2/chart.hh"
//#include "draw.hh"
#include "vaccines.hh"

// ----------------------------------------------------------------------

static inline acmacs::PointStyleModified& point_style_for(acmacs::PointStyleModified&& aStyle, hidb::Vaccines::PassageType pt)
{
    switch (pt) {
      case hidb::Vaccines::Egg:
          aStyle.aspect(AspectEgg);
          aStyle.rotation(NoRotation);
          break;
      case hidb::Vaccines::Cell:
          aStyle.aspect(AspectNormal);
          aStyle.rotation(NoRotation);
          break;
      case hidb::Vaccines::Reassortant:
          aStyle.aspect(AspectEgg);
          aStyle.rotation(RotationReassortant);
          break;
      case hidb::Vaccines::PassageTypeSize:
          break;
    }
    return aStyle;
}

Vaccines::Vaccines(const acmacs::chart::Chart& aChart)
    : mVaccinesOfChart{hidb::vaccines(aChart)}
{
    for (size_t vaccines_of_chart_index = 0; vaccines_of_chart_index < mVaccinesOfChart.size(); ++vaccines_of_chart_index) {
        auto update = [&](hidb::Vaccines::PassageType pt) {
            if (!mVaccinesOfChart[vaccines_of_chart_index].empty(pt)) {
                mEntries.emplace_back(vaccines_of_chart_index, pt, point_style_for(acmacs::PointStyleModified(), pt));
            }
        };
        hidb::Vaccines::for_each_passage_type(update);
    }

} // Vaccines::Vaccines

// ----------------------------------------------------------------------

std::string Vaccines::report(const hidb::Vaccines::ReportConfig& config) const
{
    std::string result;
    for (const auto& entry: mEntries) {
        if (entry.style.shown()) {
            const auto& vacc = mVaccinesOfChart[entry.vaccines_of_chart_index];
            const std::string s = vacc.report(entry.passage_type, config, entry.antigen_no);
            if (!s.empty())
                result += fmt::format("{:{}c}{} {} {}\n{}", ' ', config.indent_,  vacc.type(), vacc.name(), entry.style.fill(), s);
        }
    }
    return result;

} // Vaccines::report

// ----------------------------------------------------------------------

std::vector<size_t> Vaccines::indices() const
{
    std::vector<size_t> ind;
    for (const auto& entry: mEntries) {
        if (entry.style.shown()) {
            if (const auto* vacc = mVaccinesOfChart[entry.vaccines_of_chart_index].for_passage_type(entry.passage_type, entry.antigen_no); vacc)
                ind.push_back(vacc->chart_antigen_index);
        }
    }
    return ind;

} // Vaccines::indices

// ----------------------------------------------------------------------

std::vector<size_t> Vaccines::indices(const VaccineMatchData& aMatchData) const
{
    std::vector<std::pair<size_t, hidb::Vaccines::PassageType>> ind_passage;
    for (const auto& entry: mEntries) {
        if (entry.match(mVaccinesOfChart, aMatchData)) {
            if (const auto* vacc = mVaccinesOfChart[entry.vaccines_of_chart_index].for_passage_type(entry.passage_type, aMatchData.no() /* entry.antigen_no */); vacc)
                ind_passage.emplace_back(vacc->chart_antigen_index, entry.passage_type);
        }
    }

    const auto to_index = [](const auto& en) { return en.first; };
    if (aMatchData.passage() == "cell-egg") {
        const auto is_cell = [](const auto& en) { return en.second == hidb::Vaccines::Cell; };
        if (ranges::any_of(ind_passage, is_cell))
            return ind_passage | ranges::views::filter(is_cell) | ranges::views::transform(to_index) | ranges::to_vector;
        else
            return ind_passage | ranges::views::transform(to_index) | ranges::to_vector;
    }
    else if (aMatchData.passage() == "egg-cell") {
        const auto is_egg = [](const auto& en) { return en.second == hidb::Vaccines::Egg; };
        if (ranges::any_of(ind_passage, is_egg))
            return ind_passage | ranges::views::filter(is_egg) | ranges::views::transform(to_index) | ranges::to_vector;
        else
            return ind_passage | ranges::views::transform(to_index) | ranges::to_vector;
    }
    else
        return ind_passage | ranges::views::transform(to_index) | ranges::to_vector;

} // Vaccines::indices

// ----------------------------------------------------------------------

void Vaccines::plot(ChartDraw& aChartDraw) const
{
    for (const auto& entry: mEntries) {
        if (entry.style.shown()) {
            if (const auto* vacc = mVaccinesOfChart[entry.vaccines_of_chart_index].for_passage_type(entry.passage_type, entry.antigen_no); vacc)
                aChartDraw.modify(vacc->chart_antigen_index, entry.style, PointDrawingOrder::Raise);
        }
    }

} // Vaccines::plot

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
