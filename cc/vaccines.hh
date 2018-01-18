#pragma once

#include "hidb-5/hidb.hh"
#include "hidb-5/vaccines.hh"
#include "acmacs-base/point-style.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

class ChartDraw;
namespace acmacs::chart { class Chart; }

// ----------------------------------------------------------------------

class VaccineMatchData
{
 public:
    inline VaccineMatchData() : mNo{0} {}

    inline VaccineMatchData& type(std::string aType) { mType = aType; return *this; }
    inline VaccineMatchData& passage(std::string aPassage) { mPassage = aPassage; return *this; }
    inline VaccineMatchData& no(size_t aNo) { mNo = aNo; return *this; }
    inline VaccineMatchData& name(std::string aName) { mName = aName; return *this; }

    inline std::string type() const { return mType; }
    inline std::string passage() const { return mPassage; }
    inline size_t no() const { return mNo; }
    inline std::string name() const { return mName; }

 private:
    std::string mType;
    std::string mPassage;
    size_t mNo;
    std::string mName;

}; // class VaccineMatchData

// ----------------------------------------------------------------------

class Vaccines
{
 public:
    class Entry
    {
     public:
        inline Entry(size_t aVaccinesOfChartIndex, hidb::Vaccines::PassageType aPassageType, const acmacs::PointStyle& aStyle)
            : vaccines_of_chart_index(aVaccinesOfChartIndex), passage_type(aPassageType), style(aStyle) {}

        size_t vaccines_of_chart_index;
        hidb::Vaccines::PassageType passage_type;
        size_t antigen_no = 0;
        acmacs::PointStyle style;

        inline bool match(const hidb::VaccinesOfChart& aVaccinesOfChart, const VaccineMatchData& aMatchData) const
            {
                return (aMatchData.passage().empty() || hidb::Vaccines::passage_type(aMatchData.passage()) == passage_type)
                        && aVaccinesOfChart[vaccines_of_chart_index].match(aMatchData.name(), aMatchData.type());
            }

    }; // class Entry

    Vaccines(const acmacs::chart::Chart& aChart, bool aVerbose = false);

    inline std::string report_all(size_t aIndent) const { return mVaccinesOfChart.report(aIndent); }
    std::string report(size_t aIndent) const;
    std::vector<size_t> indices() const;
    std::vector<size_t> indices(const VaccineMatchData& aMatchData) const;
    void plot(ChartDraw& aChartDraw) const;

 private:
    hidb::VaccinesOfChart mVaccinesOfChart;
    std::vector<Entry> mEntries;

    friend class VaccineMatcherBase;

}; // class Vaccines

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
