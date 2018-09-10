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

void ModProcrustesArrows::apply(ChartDraw& aChartDraw, const rjson::v1::value& /*aModData*/)
{
    const auto verbose = args().get_or_default("report", false);
    const auto scaling = args().get_or_default("scaling", false) ? acmacs::chart::procrustes_scaling_t::yes : acmacs::chart::procrustes_scaling_t::no;
    const auto secondary_projection_no = args().get_or_default("projection", 0UL);

    const auto subset_s = args().get_or_default("subset", "all");
    acmacs::chart::CommonAntigensSera::subset subset = acmacs::chart::CommonAntigensSera::subset::all;
    if (subset_s == "sera")
        subset = acmacs::chart::CommonAntigensSera::subset::sera;
    else if (subset_s == "antigens")
        subset = acmacs::chart::CommonAntigensSera::subset::antigens;
    else if (subset_s != "all")
        std::cerr << "WARNING: unrecognized common points subset: \"" << subset_s << "\", supported: all, antigens, sera\n";

    acmacs::chart::ChartP secondary_chart;
    if (const auto [present, chart_filename] = args().get_value_if("chart"); present)
        secondary_chart = acmacs::chart::import_from_file(chart_filename.str(), acmacs::chart::Verify::None, verbose ? report_time::Yes : report_time::No);
    else
        secondary_chart = aChartDraw.chartp();
    const auto match_level = make_match_level(args().get_or_default("match", ""));
    acmacs::chart::CommonAntigensSera common(aChartDraw.chart(), *secondary_chart, match_level);
    if (verbose)
        common.report();
    const auto common_points = common.points(subset);
    auto secondary_projection = secondary_chart->projection(secondary_projection_no);
    const auto procrustes_data = acmacs::chart::procrustes(aChartDraw.projection(), *secondary_projection, common_points, scaling);
    if (aChartDraw.has_title()) {
        auto& title = aChartDraw.title();
        title.add_line(secondary_chart->make_name(secondary_projection_no));
        title.add_line("RMS: " + std::to_string(procrustes_data.rms));
    }
    auto secondary_layout = procrustes_data.apply(*secondary_projection->layout());
    auto primary_layout = aChartDraw.projection().transformed_layout();
    const auto arrow_config = args().get_or_empty_object("arrow");
    const auto threshold = args().get_or_default("threshold", 0.005);
    for (size_t point_no = 0; point_no < common_points.size(); ++point_no) {
        const auto primary_coords = primary_layout->get(common_points[point_no].primary),
                secondary_coords = secondary_layout->get(common_points[point_no].secondary);
        if (primary_coords.distance(secondary_coords) > threshold) {
            auto& arrow = aChartDraw.arrow(primary_coords, secondary_coords);
            arrow.color(Color(arrow_config.get_or_default("color", "black")), Color(arrow_config.get_or_default("head_color", "black")));
            arrow.line_width(arrow_config.get_or_default("line_width", 1.0));
            arrow.arrow_head_filled(arrow_config.get_or_default("head_filled", true));
            arrow.arrow_width(arrow_config.get_or_default("arrow_width", 5.0));
        }
    }

} // ModProcrustesArrows::apply

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
