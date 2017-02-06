#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include "acmacs-chart/chart.hh"

// ----------------------------------------------------------------------

namespace hidb
{
    class HiDb;
    class Antigen;
    class Serum;
    template <typename AS> class AntigenSerumData;
    class AntigenRefs;
}

// ----------------------------------------------------------------------

class Vaccines
{
 public:
    class HomologousSerum
    {
     public:
        inline HomologousSerum(size_t aSerumIndex, const Serum* aSerum, const hidb::AntigenSerumData<hidb::Serum>* aSerumData, std::string aMostRecentTableDate)
            : serum(aSerum), serum_index(aSerumIndex), serum_data(aSerumData), most_recent_table_date(aMostRecentTableDate) {}
        bool operator < (const HomologousSerum& a) const;
        size_t number_of_tables() const;

        const Serum* serum;
        size_t serum_index;
        const hidb::AntigenSerumData<hidb::Serum>* serum_data;
        std::string most_recent_table_date;
    };

    class Entry
    {
     public:
        inline Entry(size_t aAntigenIndex, const Antigen* aAntigen, const hidb::AntigenSerumData<hidb::Antigen>* aAntigenData, std::vector<HomologousSerum>&& aSera, std::string aMostRecentTableDate)
            : antigen(aAntigen), antigen_index(aAntigenIndex), antigen_data(aAntigenData), homologous_sera(aSera), most_recent_table_date(aMostRecentTableDate)
            { std::sort(homologous_sera.begin(), homologous_sera.end()); }
        bool operator < (const Entry& a) const;

        const Antigen* antigen;
        size_t antigen_index;
        const hidb::AntigenSerumData<hidb::Antigen>* antigen_data;
        std::vector<HomologousSerum> homologous_sera; // sorted by number of tables and the most recent table
        std::string most_recent_table_date;
    };

    inline Vaccines() {}

    inline const Entry* egg() const { return mEgg.empty() ? nullptr : &mEgg.front(); }
    inline const Entry* cell() const { return mCell.empty() ? nullptr : &mCell.front(); }
    inline const Entry* reassortant() const { return mReassortant.empty() ? nullptr : &mReassortant.front(); }

    std::string report() const;

 private:
    std::vector<Entry> mEgg;
    std::vector<Entry> mCell;
    std::vector<Entry> mReassortant;

    friend class Chart;

    void add(size_t aAntigenIndex, const Antigen& aAntigen, const hidb::AntigenSerumData<hidb::Antigen>* aAntigenData, std::vector<HomologousSerum>&& aSera, std::string aMostRecentTableDate);
    void sort();

}; // class Vaccines

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
