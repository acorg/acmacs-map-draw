#include "acmacs-chart-2/factory-import.hh"
//#include "acmacs-chart-2/chart.hh"
#include "acmacs-chart-2/procrustes.hh"
#include "acmacs-chart-2/common.hh"
#include "acmacs-map-draw/mod-procrustes.hh"

// ----------------------------------------------------------------------

static inline acmacs::chart::CommonAntigensSera::match_level_t make_match_level(std::string_view match_s)
{
    auto match_level{acmacs::chart::CommonAntigensSera::match_level_t::automatic};
    if (!match_s.empty()) {
        switch (match_s[0]) {
          case 's': match_level = acmacs::chart::CommonAntigensSera::match_level_t::strict; break;
          case 'r': match_level = acmacs::chart::CommonAntigensSera::match_level_t::relaxed; break;
          case 'i': match_level = acmacs::chart::CommonAntigensSera::match_level_t::ignored; break;
          case 'a': match_level = acmacs::chart::CommonAntigensSera::match_level_t::automatic; break;
          default:
              std::cerr << "Unrecognized --match argument, automatic assumed" << '\n';
              break;
        }
    }
    return match_level;
}

// ----------------------------------------------------------------------

void ModProcrustesArrows::apply(ChartDraw& aChartDraw, const rjson::value& aModData)
{
      // {"N": "procrustes_arrows", "chart": "secondary.ace", "projection": 0, "match": "auto", "?match": "auto, strict, relaxed, ignored"}

    const auto verbose = args().get_or_default("report", false);
    const auto scaling = args().get_or_default("scaling", false) ? acmacs::chart::procrustes_scaling_t::yes : acmacs::chart::procrustes_scaling_t::no;
    const auto secondary_projection_no = args().get_or_default("projection", 0UL);

    acmacs::chart::ChartP secondary_chart;
    if (const auto [present, chart_filename] = args().get_value_if("chart"); present)
        secondary_chart = acmacs::chart::import_from_file(chart_filename.str(), acmacs::chart::Verify::None, verbose ? report_time::Yes : report_time::No);
    else
        secondary_chart = aChartDraw.chartp();
    const auto match_level = make_match_level(args().get_or_default("match", ""));
    acmacs::chart::CommonAntigensSera common(aChartDraw.chart(), *secondary_chart, match_level);
    if (verbose)
        common.report();
    const auto procrustes_data = acmacs::chart::procrustes(aChartDraw.projection(), *secondary_chart->projection(secondary_projection_no), common.points(), scaling);

} // ModProcrustesArrows::apply

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
