#pragma once

#include "hidb/hidb.hh"
#include "hidb/vaccines.hh"

// ----------------------------------------------------------------------

class Vaccines
{
 public:
    class Entry
    {
     public:
        inline Entry(size_t aVaccinesOfChartIndex, hidb::Vaccines::PassageType aPassageType) : vaccines_of_chart_index(aVaccinesOfChartIndex), passage_type(aPassageType), antigen_no(0) {}

        size_t vaccines_of_chart_index;
        hidb::Vaccines::PassageType passage_type;
        size_t antigen_no;

    }; // class Entry

    Vaccines(const Chart& aChart, const hidb::HiDb& aHiDb);

    inline std::string report_all(size_t aIndent) const { return mVaccinesOfChart.report(aIndent); }
    std::string report(size_t aIndent) const;

 private:
    hidb::VaccinesOfChart mVaccinesOfChart;
    std::vector<Entry> mEntries;

}; // class Vaccines

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
