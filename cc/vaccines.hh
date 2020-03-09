#pragma once

#include "acmacs-base/sfinae.hh"
#include "acmacs-base/point-style.hh"
#include "hidb-5/hidb.hh"
#include "hidb-5/vaccines.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

class ChartDraw;
namespace acmacs::chart { class Chart; }

// ----------------------------------------------------------------------

class VaccineMatchData
{
 public:
    VaccineMatchData() : mNo{0} {}

    VaccineMatchData& type(std::string_view aType) { mType.assign(aType); return *this; }
    VaccineMatchData& passage(std::string_view aPassage) { mPassage.assign(aPassage); return *this; }
    VaccineMatchData& no(size_t aNo) { mNo = aNo; return *this; }
    VaccineMatchData& name(std::string_view aName) { mName.assign(aName); return *this; }

    std::string_view type() const { return mType; }
    std::string_view passage() const { return mPassage; }
    size_t no() const { return mNo; }
    std::string_view name() const { return mName; }

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
        Entry(size_t aVaccinesOfChartIndex, hidb::Vaccines::PassageType aPassageType, const acmacs::PointStyle& aStyle)
            : vaccines_of_chart_index(aVaccinesOfChartIndex), passage_type(aPassageType), style(aStyle) {}

        size_t vaccines_of_chart_index;
        hidb::Vaccines::PassageType passage_type;
        size_t antigen_no = 0;
        acmacs::PointStyle style;

        bool match(const hidb::VaccinesOfChart& aVaccinesOfChart, const VaccineMatchData& aMatchData) const
        {
            if (aMatchData.passage() == "cell-egg" || aMatchData.passage() == "egg-cell")
                return aVaccinesOfChart[vaccines_of_chart_index].match(aMatchData.name(), aMatchData.type());
            else
                return (aMatchData.passage().empty() || hidb::Vaccines::passage_type(aMatchData.passage()) == passage_type) &&
                       aVaccinesOfChart[vaccines_of_chart_index].match(aMatchData.name(), aMatchData.type());
        }

    }; // class Entry

    Vaccines(const acmacs::chart::Chart& aChart);

    std::string report_all(const hidb::Vaccines::ReportConfig& config) const { return mVaccinesOfChart.report(config); }
    std::string report(const hidb::Vaccines::ReportConfig& config) const;
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
