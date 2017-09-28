#pragma once

#include <string>
#include <vector>

#include "acmacs-base/rjson.hh"

// ----------------------------------------------------------------------

class Chart;

// ----------------------------------------------------------------------

class SelectAntigensSera
{
 public:
    virtual ~SelectAntigensSera();

    virtual std::vector<size_t> select(const Chart& aChart, const rjson::value& aSelector) = 0;

}; // class SelectAntigensSera

// ----------------------------------------------------------------------

class SelectAntigens : public SelectAntigensSera
{
 public:
    std::vector<size_t> select(const Chart& aChart, const rjson::value& aSelector) override;

};  // class SelectAntigens

// ----------------------------------------------------------------------

class SelectSera : public SelectAntigensSera
{
 public:
    std::vector<size_t> select(const Chart& aChart, const rjson::value& aSelector) override;

};  // class SelectSera

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
