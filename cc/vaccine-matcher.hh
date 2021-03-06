#pragma once

#include "vaccines.hh"

// ----------------------------------------------------------------------

class VaccineMatcherBase
{
 public:
    std::string report(const hidb::Vaccines::ReportConfig& config) const { return mVaccines.report(config); }

 protected:
    VaccineMatcherBase(Vaccines& aVaccines, const VaccineMatchData& aMatchData) : mVaccines(aVaccines), mMatchData(aMatchData) {}
    VaccineMatcherBase(const VaccineMatcherBase& aNother) : mVaccines(aNother.mVaccines), mMatchData(aNother.mMatchData) {}

    template <typename F> void for_each(F f)
        {
            for (auto& entry: mVaccines.mEntries) {
                if (entry.match(mVaccines.mVaccinesOfChart, mMatchData))
                    f(entry);
            }
        }

    // template <typename MF, typename Arg> void for_each_style_mf(MF mf, Arg arg)
    //     {
    //         for_each([&](auto& entry) { mf(entry.style, arg); });
    //     }

    template <typename F> void for_each_with_vacc(F f, bool only_shown = true)
        {
            for_each([&](auto& entry) {
                    if (!only_shown || entry.style.shown()) {
                        const auto* vacc_entry = mVaccines.mVaccinesOfChart[entry.vaccines_of_chart_index].for_passage_type(entry.passage_type, entry.antigen_no);
                        if (vacc_entry)
                            f(*vacc_entry);
                    }
                });
        }

    // template <typename F> void for_each_with_entry_vacc(F f)
    //     {
    //         for_each([&](auto& entry) {
    //                 if (entry.style.shown()) {
    //                     const auto* vacc_entry = mVaccines.mVaccinesOfChart[entry.vaccines_of_chart_index].for_passage_type(entry.passage_type, entry.antigen_no);
    //                     if (vacc_entry)
    //                         f(entry, *vacc_entry);
    //                 }
    //             });
    //     }

    auto& vaccine_of_chart(size_t aIndex) { return mVaccines.mVaccinesOfChart[aIndex]; }

 private:
    Vaccines& mVaccines;
    const VaccineMatchData mMatchData;
    std::string mName, mType, mPassageType;

}; // class VaccineMatcherBase

// ----------------------------------------------------------------------

class VaccineMatcherLabel : public VaccineMatcherBase
{
 private:
    template <typename MF, typename ... Arg> void for_each_mf(MF mf, Arg&& ... arg)
        {
            auto mff = std::mem_fn(mf);
            for_each_with_vacc([&](const hidb::Vaccines::Entry& ve) { mff(mChartDraw.add_label(ve.chart_antigen_index), std::forward<Arg>(arg) ...); });
        }

 public:
    using LBL = acmacs::draw::PointLabel;

    VaccineMatcherLabel& display_name(std::string aDisplayName) { for_each_mf(static_cast<LBL& (LBL::*)(std::string_view)>(&LBL::display_name), aDisplayName); return *this; }
    VaccineMatcherLabel& color(Color aColor) { for_each_mf(&LBL::color, acmacs::color::Modifier{aColor}); return *this; }
    template <typename S, typename = std::enable_if_t<acmacs::sfinae::is_string_v<S>>> VaccineMatcherLabel& color(S aColor) { for_each_mf(&LBL::color, acmacs::color::Modifier{Color(aColor)}); return *this; }
    VaccineMatcherLabel& size(Pixels aSize) { for_each_mf(&LBL::size, aSize); return *this; }
    template <typename S, typename = std::enable_if_t<acmacs::sfinae::is_string_v<S>>> VaccineMatcherLabel& weight(S aWeight) { for_each_mf(&LBL::weight, aWeight); return *this; }
    template <typename S, typename = std::enable_if_t<acmacs::sfinae::is_string_v<S>>> VaccineMatcherLabel& slant(S aSlant) { for_each_mf(&LBL::slant, aSlant); return *this; }
    template <typename S, typename = std::enable_if_t<acmacs::sfinae::is_string_v<S>>> VaccineMatcherLabel& font_family(S aFamily) { for_each_mf(&LBL::font_family, aFamily); return *this; }
    VaccineMatcherLabel& offset(const acmacs::PointCoordinates& loc) { for_each_mf(static_cast<LBL& (LBL::*)(const acmacs::PointCoordinates&)>(&LBL::offset), loc); return *this; }
    VaccineMatcherLabel& name_type(std::string aNameType);

    VaccineMatcherLabel& hide() { for_each_with_vacc([this](const auto& vacc_entry) { mChartDraw.remove_label(vacc_entry.chart_antigen_index); }, false); return *this; }

 private:
    ChartDraw& mChartDraw;

    VaccineMatcherLabel(const VaccineMatcherBase& aNother, ChartDraw& aChartDraw)
        : VaccineMatcherBase(aNother), mChartDraw(aChartDraw) {}

    friend class VaccineMatcher;

}; // class VaccineMatcherLabel

// ----------------------------------------------------------------------

class VaccineMatcher : public VaccineMatcherBase
{
 public:
    VaccineMatcher(Vaccines& aVaccines, const VaccineMatchData& aMatchData) : VaccineMatcherBase(aVaccines, aMatchData) {}

    VaccineMatcher& no(size_t aNo) { for_each([this, aNo](Vaccines::Entry& e) { if (this->vaccine_of_chart(e.vaccines_of_chart_index).number_of(e.passage_type) <= aNo) throw std::runtime_error("Invalid antigen no: " + std::to_string(aNo)); e.antigen_no = aNo; }); return *this; }
    VaccineMatcher& show(bool aShow) { for_each([aShow](Vaccines::Entry& e) { e.style.shown(aShow); }); return *this; }
    VaccineMatcher& shape(std::string aShape) { for_each([aShape](Vaccines::Entry& e) { e.style.shape(acmacs::PointShape{aShape}); }); return *this; }
    VaccineMatcher& size(double aSize) { for_each([aSize](Vaccines::Entry& e) { e.style.size(Pixels{aSize}); }); return *this; }
    VaccineMatcher& fill(Color aColor) { for_each([aColor](Vaccines::Entry& e) { e.style.fill(acmacs::color::Modifier{aColor}); }); return *this; }
    VaccineMatcher& fill(std::string aColor) { for_each([aColor](Vaccines::Entry& e) { e.style.fill(acmacs::color::Modifier{Color(aColor)}); }); return *this; }
    VaccineMatcher& fill(std::string_view aColor) { for_each([aColor](Vaccines::Entry& e) { e.style.fill(acmacs::color::Modifier{Color(aColor)}); }); return *this; }
    VaccineMatcher& outline(Color aColor) { for_each([aColor](Vaccines::Entry& e) { e.style.outline(acmacs::color::Modifier{Color(aColor)}); }); return *this; }
    VaccineMatcher& outline(std::string aColor) { for_each([aColor](Vaccines::Entry& e) { e.style.outline(acmacs::color::Modifier{Color(aColor)}); }); return *this; }
    VaccineMatcher& outline(std::string_view aColor) { for_each([aColor](Vaccines::Entry& e) { e.style.outline(acmacs::color::Modifier{Color(aColor)}); }); return *this; }
    VaccineMatcher& outline_width(double aOutlineWidth) { for_each([aOutlineWidth](Vaccines::Entry& e) { e.style.outline_width(Pixels{aOutlineWidth}); }); return *this; }
    VaccineMatcher& aspect(double aAspect) { for_each([aAspect](Vaccines::Entry& e) { e.style.aspect(Aspect{aAspect}); }); return *this; }
    VaccineMatcher& rotation(double aRotation) { for_each([aRotation](Vaccines::Entry& e) { e.style.rotation(Rotation{aRotation}); }); return *this; }

    VaccineMatcherLabel* label(ChartDraw& aChartDraw) { return new VaccineMatcherLabel(*this, aChartDraw); }
    void hide_label(ChartDraw& aChartDraw) { VaccineMatcherLabel(*this, aChartDraw).hide(); }

}; // class VaccineMatcher

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
