#include "acmacs-chart/chart.hh"
#include "draw.hh"
#include "vaccines.hh"

// ----------------------------------------------------------------------

static inline PointStyle& point_style_for(PointStyle&& aStyle, hidb::Vaccines::PassageType pt)
{
    switch (pt) {
      case hidb::Vaccines::Egg: aStyle.aspect(AspectEgg).rotation(RotationRegular); break;
      case hidb::Vaccines::Cell: aStyle.aspect(AspectRegular).rotation(RotationRegular); break;
      case hidb::Vaccines::Reassortant: aStyle.aspect(AspectEgg).rotation(RotationReassortant); break;
      case hidb::Vaccines::PassageTypeSize: break;
    }
    return aStyle;
}

Vaccines::Vaccines(const Chart& aChart, const hidb::HiDb& aHiDb)
{
    hidb::vaccines(mVaccinesOfChart, aChart, aHiDb);
    for (size_t vaccines_of_chart_index = 0; vaccines_of_chart_index < mVaccinesOfChart.size(); ++vaccines_of_chart_index) {
        auto update = [&](hidb::Vaccines::PassageType pt) {
            if (!mVaccinesOfChart[vaccines_of_chart_index].empty(pt)) {
                mEntries.emplace_back(vaccines_of_chart_index, pt, point_style_for(PointStyle(), pt));
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
        if (entry.style.shown()) {
            const auto& vacc = mVaccinesOfChart[entry.vaccines_of_chart_index];
            const std::string s = vacc.report(entry.passage_type, aIndent, entry.antigen_no);
            if (!s.empty())
                result += std::string(aIndent, ' ') + vacc.type() + " " + vacc.name() + s.substr(aIndent - 1) + " " + entry.style.fill();
        }
    }
    return result;

} // Vaccines::report

// ----------------------------------------------------------------------

void Vaccines::plot(ChartDraw& aChartDraw) const
{
    for (const auto& entry: mEntries) {
        if (entry.style.shown()) {
            const auto* vacc = mVaccinesOfChart[entry.vaccines_of_chart_index].for_passage_type(entry.passage_type, entry.antigen_no);
            if (vacc) {
                aChartDraw.modify(vacc->antigen_index, entry.style, true, false);
            }
        }
    }

} // Vaccines::plot

// ----------------------------------------------------------------------

VaccineMatcherLabel& VaccineMatcherLabel::name_type(std::string aNameType)
{
    for_each_with_vacc([&](const hidb::Vaccines::Entry& ve) {
        const auto& antigen = static_cast<const Antigen&>(mChartDraw.chart().antigen(ve.antigen_index)); // dynamic_cast throws here (clang 4.1)
        std::string name;
        if (aNameType == "abbreviated")
            name = antigen.abbreviated_name(mLocDb);
        else if (aNameType == "abbreviated_with_passage_type")
            name = antigen.abbreviated_name_with_passage_type(mLocDb);
        else if (aNameType == "abbreviated_location_with_passage_type")
            name = antigen.abbreviated_location_with_passage_type(mLocDb);
        else {
            if (aNameType != "full") {
                log_warning("unsupported name_type \"", aNameType, "\" (\"full\" is used)");
            }
            name = antigen.full_name();
        }
          // std::cerr << "DEBUG: name " << aNameType << ": \"" << name << '"' << std::endl;
        mChartDraw.add_label(ve.antigen_index).display_name(name);
    });
    return *this;

} // VaccineMatcherLabel::name_type

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
