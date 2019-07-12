#pragma once

#include <string_view>

#include "acmacs-chart-2/chart.hh"

// ----------------------------------------------------------------------

std::string export_layout_sequences_into_csv(std::string_view filename, const acmacs::chart::Chart& chart, size_t projection_no);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
