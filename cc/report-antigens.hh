#pragma once

#include "acmacs-base/fmt.hh"

// ----------------------------------------------------------------------

namespace acmacs::chart
{
    class PointIndexList;
}

class ChartSelectInterface;

[[nodiscard]] std::string report_antigens(const acmacs::chart::PointIndexList& indexes, const ChartSelectInterface& aChartSelectInterface, size_t threshold);
[[nodiscard]] std::string report_sera(const acmacs::chart::PointIndexList& indexes, const ChartSelectInterface& aChartSelectInterface, size_t threshold);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
