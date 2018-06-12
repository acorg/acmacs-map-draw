#pragma once

#include "acmacs-base/line.hh"

// ----------------------------------------------------------------------

class ChartSelectInterface;

class SerumLine
{
 public:
    SerumLine(const ChartSelectInterface& aChartSelectInterface);

    constexpr const acmacs::LineDefinedByEquation& line() const { return line_; }
    constexpr double standard_deviation() const { return standard_deviation_; }

 private:
    acmacs::LineDefinedByEquation line_;
    double standard_deviation_;

}; // class SerumLine

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
