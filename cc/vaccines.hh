#pragma once

#include "hidb/hidb.hh"
#include "hidb/vaccines.hh"

// ----------------------------------------------------------------------

class VaccineMatcher;

class Vaccines
{
 public:
    class Entry
    {
     public:
        inline Entry(size_t aVaccinesOfChartIndex, hidb::Vaccines::PassageType aPassageType) : vaccines_of_chart_index(aVaccinesOfChartIndex), passage_type(aPassageType), antigen_no(0), show(true) {}

        size_t vaccines_of_chart_index;
        hidb::Vaccines::PassageType passage_type;
        size_t antigen_no;
        bool show;

        inline bool match(hidb::VaccinesOfChart& aVaccinesOfChart, std::string aName, std::string aType, std::string aPassageType) const
            {
                return (aPassageType.empty() || hidb::Vaccines::passage_type(aPassageType) == passage_type) && aVaccinesOfChart[vaccines_of_chart_index].match(aName, aType);
            }

    }; // class Entry

    Vaccines(const Chart& aChart, const hidb::HiDb& aHiDb);

    inline std::string report_all(size_t aIndent) const { return mVaccinesOfChart.report(aIndent); }
    std::string report(size_t aIndent) const;
    VaccineMatcher* match(std::string aName, std::string aType, std::string aPassageType);

 private:
    hidb::VaccinesOfChart mVaccinesOfChart;
    std::vector<Entry> mEntries;

    friend class VaccineMatcher;

}; // class Vaccines

// ----------------------------------------------------------------------

class VaccineMatcher
{
 public:
    inline void show(bool aShow) { for_each([aShow](Vaccines::Entry& e) { e.show = aShow; }); }

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
