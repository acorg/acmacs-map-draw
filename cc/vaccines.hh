#pragma once

#include "hidb/hidb.hh"
#include "hidb/vaccines.hh"
#include "acmacs-chart/point-style.hh"

// ----------------------------------------------------------------------

class ChartDraw;
class VaccineMatcher;

class Vaccines
{
 public:
    class Entry
    {
     public:
        inline Entry(size_t aVaccinesOfChartIndex, hidb::Vaccines::PassageType aPassageType, const PointStyle& aStyle)
            : vaccines_of_chart_index(aVaccinesOfChartIndex), passage_type(aPassageType), antigen_no(0), style(aStyle) {}

        size_t vaccines_of_chart_index;
        hidb::Vaccines::PassageType passage_type;
        size_t antigen_no;
        PointStyle style;

        inline bool match(hidb::VaccinesOfChart& aVaccinesOfChart, std::string aName, std::string aType, std::string aPassageType) const
            {
                return (aPassageType.empty() || hidb::Vaccines::passage_type(aPassageType) == passage_type) && aVaccinesOfChart[vaccines_of_chart_index].match(aName, aType);
            }

    }; // class Entry

    Vaccines(const Chart& aChart, const hidb::HiDb& aHiDb);

    inline std::string report_all(size_t aIndent) const { return mVaccinesOfChart.report(aIndent); }
    std::string report(size_t aIndent) const;
    VaccineMatcher* match(std::string aName, std::string aType, std::string aPassageType);
    void plot(ChartDraw& aChartDraw) const;

 private:
    hidb::VaccinesOfChart mVaccinesOfChart;
    std::vector<Entry> mEntries;

    friend class VaccineMatcherBase;

}; // class Vaccines

// ----------------------------------------------------------------------

class VaccineMatcherBase
{
 protected:
    inline VaccineMatcherBase(Vaccines& aVaccines, std::string aName, std::string aType, std::string aPassageType)
        : mVaccines(aVaccines), mName(aName), mType(aType), mPassageType(aPassageType) {}
    inline VaccineMatcherBase(const VaccineMatcherBase& aNother)
        : mVaccines(aNother.mVaccines), mName(aNother.mName), mType(aNother.mType), mPassageType(aNother.mPassageType) {}

    template <typename F> inline void for_each(F f)
        {
            for (auto& entry: mVaccines.mEntries) {
                if (entry.match(mVaccines.mVaccinesOfChart, mName, mType, mPassageType))
                    f(entry);
            }
        }

    template <typename MF, typename Arg> inline void for_each_style_mf(MF mf, Arg arg)
        {
            for_each([&](auto& entry) { mf(entry.style, arg); });
        }

    template <typename F> inline void for_each_with_vacc(F f)
        {
            for_each([&](auto& entry) {
                    if (entry.style.shown()) {
                        const auto* vacc_entry = mVaccines.mVaccinesOfChart[entry.vaccines_of_chart_index].for_passage_type(entry.passage_type, entry.antigen_no);
                        if (vacc_entry)
                            f(*vacc_entry);
                    }
                });
        }

    // template <typename F> inline void for_each_with_entry_vacc(F f)
    //     {
    //         for_each([&](auto& entry) {
    //                 if (entry.style.shown()) {
    //                     const auto* vacc_entry = mVaccines.mVaccinesOfChart[entry.vaccines_of_chart_index].for_passage_type(entry.passage_type, entry.antigen_no);
    //                     if (vacc_entry)
    //                         f(entry, *vacc_entry);
    //                 }
    //             });
    //     }

    inline auto& vaccine_of_chart(size_t aIndex) { return mVaccines.mVaccinesOfChart[aIndex]; }

 private:
    Vaccines& mVaccines;
    std::string mName, mType, mPassageType;

}; // class VaccineMatcherBase

// ----------------------------------------------------------------------

class VaccineMatcherLabel : public VaccineMatcherBase
{
 private:
    template <typename MF, typename ... Arg> inline void for_each_mf(MF mf, Arg ...arg)
        {
            auto mff = std::mem_fn(mf);
            for_each_with_vacc([&](const hidb::Vaccines::Entry& ve) { mff(mChartDraw.add_label(ve.antigen_index), arg ...); });
        }

 public:
    inline VaccineMatcherLabel& display_name(std::string aDisplayName) { for_each_mf(&Label::display_name, aDisplayName); return *this; }
    inline VaccineMatcherLabel& color(Color aColor) { for_each_mf(&Label::color, aColor); return *this; }
    inline VaccineMatcherLabel& color(std::string aColor) { for_each_mf(&Label::color, aColor); return *this; }
    inline VaccineMatcherLabel& size(double aSize) { for_each_mf(&Label::size, aSize); return *this; }
    inline VaccineMatcherLabel& weight(std::string aWeight) { for_each_mf(&Label::weight, aWeight); return *this; }
    inline VaccineMatcherLabel& slant(std::string aSlant) { for_each_mf(&Label::slant, aSlant); return *this; }
    inline VaccineMatcherLabel& font_family(std::string aFamily) { for_each_mf(&Label::font_family, aFamily); return *this; }
    inline VaccineMatcherLabel& offset(double x, double y) { for_each_mf(&Label::offset, x, y); return *this; }

 private:
    ChartDraw& mChartDraw;

    inline VaccineMatcherLabel(const VaccineMatcherBase& aNother, ChartDraw& aChartDraw) : VaccineMatcherBase(aNother), mChartDraw(aChartDraw) {}

    friend class VaccineMatcher;

}; // class VaccineMatcherLabel
// ----------------------------------------------------------------------

class VaccineMatcher : public VaccineMatcherBase
{
 public:
    inline VaccineMatcher& no(size_t aNo) { for_each([this, aNo](Vaccines::Entry& e) { if (this->vaccine_of_chart(e.vaccines_of_chart_index).number_of(e.passage_type) <= aNo) throw std::runtime_error("Invalid antigen no: " + std::to_string(aNo)); e.antigen_no = aNo; }); return *this; }
    inline VaccineMatcher& show(bool aShow) { for_each([aShow](Vaccines::Entry& e) { if (aShow) e.style.show(); else e.style.hide(); }); return *this; }
    inline VaccineMatcher& shape(std::string aShape) { for_each([aShape](Vaccines::Entry& e) { e.style.shape(aShape); }); return *this; }
    inline VaccineMatcher& size(double aSize) { for_each_style_mf(std::mem_fn<PointStyle&(Pixels)>(&PointStyle::size), Pixels{aSize}); return *this; }
    inline VaccineMatcher& fill(Color aColor) { for_each_style_mf(std::mem_fn<PointStyle&(Color)>(&PointStyle::fill), aColor); return *this; }
    inline VaccineMatcher& fill(std::string aColor) { for_each_style_mf(std::mem_fn<PointStyle&(Color)>(&PointStyle::fill), aColor); return *this; }
    inline VaccineMatcher& outline(Color aColor) { for_each_style_mf(std::mem_fn<PointStyle&(Color)>(&PointStyle::outline), aColor); return *this; }
    inline VaccineMatcher& outline(std::string aColor) { for_each_style_mf(std::mem_fn<PointStyle&(Color)>(&PointStyle::outline), aColor); return *this; }
    inline VaccineMatcher& outline_width(double aOutline_width) { for_each_style_mf(std::mem_fn<PointStyle&(Pixels)>(&PointStyle::outline_width), Pixels{aOutline_width}); return *this; }
    inline VaccineMatcher& aspect(double aAspect) { for_each_style_mf(std::mem_fn<PointStyle&(double)>(&PointStyle::aspect), aAspect); return *this; }
    inline VaccineMatcher& rotation(double aRotation) { for_each_style_mf(std::mem_fn<PointStyle&(double)>(&PointStyle::rotation), aRotation); return *this; }

    inline VaccineMatcherLabel* label(ChartDraw& aChartDraw) { return new VaccineMatcherLabel(*this, aChartDraw); }

 private:
    inline VaccineMatcher(Vaccines& aVaccines, std::string aName, std::string aType, std::string aPassageType)
        : VaccineMatcherBase(aVaccines, aName, aType, aPassageType) {}

    friend class Vaccines;

}; // class VaccineMatcher

// ----------------------------------------------------------------------

inline VaccineMatcher* Vaccines::match(std::string aName, std::string aType, std::string aPassageType)
{
    return new VaccineMatcher(*this, aName, aType, aPassageType);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
