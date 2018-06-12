#pragma once

#include "acmacs-base/line.hh"

// ----------------------------------------------------------------------

class ChartDraw;

class SerumLine
{
 public:
    SerumLine(const ChartDraw& aChartDraw);

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
