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

    friend class VaccineMatcher;

}; // class Vaccines

// ----------------------------------------------------------------------

class VaccineMatcher
{
 public:
    inline VaccineMatcher& no(size_t aNo) { for_each([this, aNo](Vaccines::Entry& e) { if (this->mVaccines.mVaccinesOfChart[e.vaccines_of_chart_index].number_of(e.passage_type) <= aNo) throw std::runtime_error("Invalid antigen no: " + std::to_string(aNo)); e.antigen_no = aNo; }); return *this; }
    inline VaccineMatcher& show(bool aShow) { for_each([aShow](Vaccines::Entry& e) { if (aShow) e.style.show(); else e.style.hide(); }); return *this; }
    inline VaccineMatcher& shape(std::string aShape) { for_each([aShape](Vaccines::Entry& e) { e.style.shape(aShape); }); return *this; }
    inline VaccineMatcher& size(double aSize) { for_each([aSize](Vaccines::Entry& e) { e.style.size(Pixels{aSize}); }); return *this; }
    // inline VaccineMatcher& fill(Color c) { mFill = c; return *this; }
    // inline VaccineMatcher& outline(Color c) { mOutline = c; return *this; }
    // inline VaccineMatcher& outline_width(Pixels aOutlineWidth) { mOutlineWidth = aOutlineWidth; return *this; }
    // inline VaccineMatcher& aspect(Aspect aAspect) { mAspect = aAspect; return *this; }
    // inline VaccineMatcher& aspect(double aAspect) { mAspect = aAspect; return *this; }
    // inline VaccineMatcher& rotation(Rotation aRotation) { mRotation = aRotation; return *this; }
    // inline VaccineMatcher& rotation(double aRotation) { mRotation = aRotation; return *this; }

 private:
    Vaccines& mVaccines;
    std::string mName, mType, mPassageType;

    inline VaccineMatcher(Vaccines& aVaccines, std::string aName, std::string aType, std::string aPassageType)
        : mVaccines(aVaccines), mName(aName), mType(aType), mPassageType(aPassageType) {}

    template <typename F> inline void for_each(F f)
        {
            for (auto& entry: mVaccines.mEntries) {
                if (entry.match(mVaccines.mVaccinesOfChart, mName, mType, mPassageType))
                    f(entry);
            }
        }

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
