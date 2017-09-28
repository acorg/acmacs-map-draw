#pragma once

#include <string>
#include <vector>

#include "acmacs-base/rjson.hh"

// ----------------------------------------------------------------------

class Chart;
class LocDb;
namespace seqdb { class Seqdb; }

// ----------------------------------------------------------------------

class SelectAntigensSera
{
 public:
    SelectAntigensSera(std::string aLocDbFilename, std::string aHidbDir, std::string aSeqdbFilename);
    virtual ~SelectAntigensSera();

    virtual std::vector<size_t> select(const Chart& aChart, const rjson::value& aSelector);
    virtual std::vector<size_t> command(const Chart& aChart, const rjson::object& aSelector) = 0;

 protected:
    const LocDb& get_location_database() const;
    const seqdb::Seqdb& get_seqdb() const;

 private:
    std::string mLocDbFilename;
    std::string mHidbDir;
    std::string mSeqdbFilename;

}; // class SelectAntigensSera

// ----------------------------------------------------------------------

class SelectAntigens : public SelectAntigensSera
{
 public:
    using SelectAntigensSera::SelectAntigensSera;

    std::vector<size_t> command(const Chart& aChart, const rjson::object& aSelector) override;

};  // class SelectAntigens

// ----------------------------------------------------------------------

class SelectSera : public SelectAntigensSera
{
 public:
    using SelectAntigensSera::SelectAntigensSera;

    std::vector<size_t> command(const Chart& aChart, const rjson::object& aSelector) override;

};  // class SelectSera

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
