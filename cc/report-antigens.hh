#pragma once

// ----------------------------------------------------------------------

namespace acmacs::chart
{
    class PointIndexList;
}

class ChartSelectInterface;

void report_antigens(const acmacs::chart::PointIndexList& indexes, const ChartSelectInterface& aChartSelectInterface, size_t threshold);
void report_sera(const acmacs::chart::PointIndexList& indexes, const ChartSelectInterface& aChartSelectInterface, size_t threshold);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
