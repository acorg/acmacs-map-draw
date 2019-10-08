#include "acmacs-chart-2/factory-import.hh"
//#include "acmacs-chart-2/chart.hh"
#include "acmacs-chart-2/procrustes.hh"
#include "acmacs-chart-2/common.hh"
#include "acmacs-map-draw/mod-procrustes.hh"
#include "acmacs-map-draw/select.hh"

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

void ModProcrustesArrows::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = rjson::get_or(args(), "report", false);
    const auto scaling = rjson::get_or(args(), "scaling", false) ? acmacs::chart::procrustes_scaling_t::yes : acmacs::chart::procrustes_scaling_t::no;
    const auto secondary_projection_no = rjson::get_or(args(), "projection", 0UL);

    const auto subset_s = rjson::get_or(args(), "subset", "all");
    acmacs::chart::CommonAntigensSera::subset subset = acmacs::chart::CommonAntigensSera::subset::all;
    if (subset_s == "sera")
        subset = acmacs::chart::CommonAntigensSera::subset::sera;
    else if (subset_s == "antigens")
        subset = acmacs::chart::CommonAntigensSera::subset::antigens;
    else if (subset_s != "all")
        std::cerr << "WARNING: unrecognized common points subset: \"" << subset_s << "\", supported: all, antigens, sera\n";

    acmacs::chart::Indexes antigen_indexes, serum_indexes;
    if (subset == acmacs::chart::CommonAntigensSera::subset::all) {
        if (const auto& subset_antigens = args()["subset_antigens"]; !subset_antigens.is_null())
            antigen_indexes = SelectAntigens(SelectAntigens::verbose::no, 30).select(aChartDraw, subset_antigens);
        else if (const auto& subset_sera = args()["subset_sera"]; !subset_sera.is_null())
            serum_indexes = SelectSera(SelectAntigens::verbose::no, 30).select(aChartDraw, subset_sera);
    }

    acmacs::chart::ChartP secondary_chart;
    if (const auto& chart_filename = args()["chart"]; !chart_filename.is_null())
        secondary_chart = acmacs::chart::import_from_file(chart_filename.to_string_view(), acmacs::chart::Verify::None, do_report_time(verbose));
    else
        secondary_chart = aChartDraw.chartp();
    const auto match_level = make_match_level(rjson::get_or(args(), "match", ""));
    acmacs::chart::CommonAntigensSera common(aChartDraw.chart(), *secondary_chart, match_level);
    if (verbose)
        common.report();
    std::vector<acmacs::chart::CommonAntigensSera::common_t> common_points;
    if (!antigen_indexes->empty())
        common_points = common.points_for_primary_antigens(antigen_indexes);
    else if (!serum_indexes->empty())
        common_points = common.points_for_primary_sera(serum_indexes);
    else
        common_points = common.points(subset);
    auto secondary_projection = secondary_chart->projection(secondary_projection_no);
    const auto procrustes_data = acmacs::chart::procrustes(aChartDraw.projection(), *secondary_projection, common_points, scaling);
    if (aChartDraw.has_title()) {
        auto& title = aChartDraw.title();
        title.add_line(secondary_chart->make_name(secondary_projection_no));
        title.add_line("RMS: " + std::to_string(procrustes_data.rms));
    }
    auto secondary_layout = procrustes_data.apply(*secondary_projection->layout());
    auto primary_layout = aChartDraw.projection().transformed_layout();
    const auto& arrow_config = args()["arrow"];
    const auto threshold = rjson::get_or(args(), "threshold", 0.005);
    for (size_t point_no = 0; point_no < common_points.size(); ++point_no) {
        const auto primary_coords = primary_layout->get(common_points[point_no].primary),
                secondary_coords = secondary_layout->get(common_points[point_no].secondary);
        if (acmacs::distance(primary_coords, secondary_coords) > threshold) {
            auto& arrow = aChartDraw.arrow(primary_coords, secondary_coords);
            arrow.color(Color(rjson::get_or(arrow_config, "color", "black")), Color(rjson::get_or(arrow_config, "head_color", "black")));
            arrow.line_width(rjson::get_or(arrow_config, "line_width", 1.0));
            arrow.arrow_head_filled(rjson::get_or(arrow_config, "head_filled", true));
            arrow.arrow_width(rjson::get_or(arrow_config, "arrow_width", 5.0));
        }
    }

} // ModProcrustesArrows::apply

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
