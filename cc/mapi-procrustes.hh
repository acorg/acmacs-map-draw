#pragma once

#include "acmacs-base/size-scale.hh"
#include "acmacs-base/color-modifier.hh"
#include "acmacs-chart-2/procrustes.hh"
#include "acmacs-chart-2/grid-test.hh"

class ChartDraw;

namespace acmacs::chart
{
    struct SelectedAntigensModify;
    struct SelectedSeraModify;

} // namespace acmacs::chart

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
    std::pair<distances_t, acmacs::chart::ProcrustesData> procrustes_arrows(ChartDraw& chart_draw, const acmacs::chart::Projection& secondary_projection, const acmacs::chart::CommonAntigensSera& common, acmacs::chart::procrustes_scaling_t scaling, const ArrowPlotSpec& arrow_plot_spec);

    // ----------------------------------------------------------------------

    struct ConnectionLinePlotSpec
    {
        acmacs::color::Modifier color{GREY};
        Pixels line_width{0.5};
    };

    struct ErrorLinePlotSpec
    {
        acmacs::color::Modifier more{RED};
        acmacs::color::Modifier less{BLUE};
        Pixels line_width{0.5};
    };

    struct HemisphringArrowsPlotSpec
    {
        acmacs::color::Modifier hemisphering{0x007FFF};
        acmacs::color::Modifier trapped{0xFF8000};
        Pixels line_width{1};
        Pixels arrow_width{5};
        Pixels arrow_outline_width{1};
    };

    void connection_lines(ChartDraw& chart_draw, const acmacs::chart::SelectedAntigensModify& antigens, const acmacs::chart::SelectedSeraModify& sera, const ConnectionLinePlotSpec& plot_spec, bool report);
    void error_lines(ChartDraw& chart_draw, const acmacs::chart::SelectedAntigensModify& antigens, const acmacs::chart::SelectedSeraModify& sera, const ErrorLinePlotSpec& plot_spec, bool report);
    void hemisphering_arrows(ChartDraw& chart_draw, const acmacs::chart::GridTest::Results& results, const HemisphringArrowsPlotSpec& plot_spec);

} // namespace acmacs::mapi::inline v1

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
