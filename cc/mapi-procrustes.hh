#pragma once

#include "acmacs-base/size-scale.hh"
#include "acmacs-base/color-modifier.hh"

namespace acmacs::chart
{
    class Projection;
    class CommonAntigensSera;
    class ProcrustesData;
}

class ChartDraw;

// ----------------------------------------------------------------------

namespace acmacs::mapi::inline v1
{
    struct ArrowPlotSpec
    {
        double threshold{0.005};
        Pixels line_width{1};
        Pixels arrow_width{5};
        Pixels arrow_outline_width{1};
        acmacs::color::Modifier outline{BLACK};
        acmacs::color::Modifier arrow_fill{BLACK};
        acmacs::color::Modifier arrow_outline{BLACK};
    };

    using distances_t = std::vector<std::pair<size_t, double>>; // point_no in primary chart and its arrow distance

    distances_t procrustes_arrows(ChartDraw& chart_draw, const acmacs::chart::Projection& secondary_projection, const acmacs::chart::CommonAntigensSera& common, const acmacs::chart::ProcrustesData& procrustes_data, const ArrowPlotSpec& arrow_plot_spec);

} // namespace acmacs::mapi::inline v1

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
