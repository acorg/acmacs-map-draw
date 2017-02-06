#include "vaccines.hh"

// ----------------------------------------------------------------------

inline bool Vaccines::Entry::operator < (const Vaccines::Entry& a) const
{
    const auto a_nt = a.antigen_data->number_of_tables(), t_nt = antigen_data->number_of_tables();
    return t_nt == a_nt ? most_recent_table_date > a.most_recent_table_date : t_nt > a_nt;
}

// ----------------------------------------------------------------------

bool Vaccines::HomologousSerum::operator < (const Vaccines::HomologousSerum& a) const
{
    bool result = true;
    if (serum->serum_species() == "SHEEP") { // avoid using sheep serum as homologous (NIMR)
        result = false;
    }
    else {
        const auto s_nt = a.serum_data->number_of_tables(), t_nt = serum_data->number_of_tables();
        result = t_nt == s_nt ? most_recent_table_date > a.most_recent_table_date : t_nt > s_nt;
    }
    return result;

} // Vaccines::HomologousSerum::operator <

// ----------------------------------------------------------------------

size_t Vaccines::HomologousSerum::number_of_tables() const
{
    return serum_data->number_of_tables();

} // Vaccines::HomologousSerum::number_of_tables

// ----------------------------------------------------------------------

void Vaccines::add(size_t aAntigenIndex, const Antigen& aAntigen, const hidb::AntigenSerumData<hidb::Antigen>* aAntigenData, std::vector<HomologousSerum>&& aSera, std::string aMostRecentTableDate)
{
    if (aAntigen.is_reassortant())
        mReassortant.emplace_back(aAntigenIndex, &aAntigen, aAntigenData, std::move(aSera), aMostRecentTableDate);
    else if (aAntigen.is_egg())
        mEgg.emplace_back(aAntigenIndex, &aAntigen, aAntigenData, std::move(aSera), aMostRecentTableDate);
    else
        mCell.emplace_back(aAntigenIndex, &aAntigen, aAntigenData, std::move(aSera), aMostRecentTableDate);

} // Vaccines::add

// ----------------------------------------------------------------------

void Vaccines::sort()
{
    std::sort(mCell.begin(), mCell.end());
    std::sort(mEgg.begin(), mEgg.end());
    std::sort(mReassortant.begin(), mReassortant.end());

} // Vaccines::sort

// ----------------------------------------------------------------------

std::string Vaccines::report() const
{
    std::ostringstream out;
    auto entry_report = [&out](const auto& entry) {
        out << "  " << entry.antigen->full_name() << " tables:" << entry.antigen_data->number_of_tables() << " recent:" << entry.antigen_data->most_recent_table().table_id() << std::endl;
        for (const auto& hs: entry.homologous_sera)
            out << "    " << hs.serum->full_name() << " tables:" << hs.serum_data->number_of_tables() << " recent:" << hs.serum_data->most_recent_table().table_id() << std::endl;
    };

    if (!mCell.empty()) {
        out << "CELL" << std::endl;
        std::for_each(mCell.begin(), mCell.end(), entry_report);
    }

    if (!mEgg.empty()) {
        out << "EGG" << std::endl;
        std::for_each(mEgg.begin(), mEgg.end(), entry_report);
    }

    if (!mReassortant.empty()) {
        out << "REASSORTANT" << std::endl;
        std::for_each(mReassortant.begin(), mReassortant.end(), entry_report);
    }
    return out.str();

} // Vaccines::report

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
