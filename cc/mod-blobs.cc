#include <cmath>
#include "acmacs-base/stream.hh"
#include "acmacs-map-draw/mod-blobs.hh"
#include "acmacs-map-draw/select.hh"

// ----------------------------------------------------------------------

void ModBlobs::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    // {"stress_diff": 0.5, "number_of_drections": 36, "stress_diff_precision": 1e-5,
    //  "fill": "transparent", "color": "pink", "line_width": 1}

    const auto verbose = rjson::get_or(args(), "report", false);
    auto& projection = aChartDraw.projection();
    const auto& select = args()["select"];
    const auto antigen_indexes = SelectAntigens(verbose).select(aChartDraw, select.is_null() ? rjson::value{"all"} : select);

    const auto blobs = projection.blobs(rjson::get_or(args(), "stress_diff", 0.5), antigen_indexes, rjson::get_or(args(), "number_of_drections", 36UL), rjson::get_or(args(), "stress_diff_precision", 1e-5));
    const auto layout = aChartDraw.projection().transformed_layout();
    const auto transformation = aChartDraw.projection().transformation();
    const auto vertical_vector_transformed = transformation.transform(acmacs::Location2D{0, -1});
    const auto base_angle = std::acos(- vertical_vector_transformed[1] / acmacs::distance(vertical_vector_transformed, {0, 0})); // apply transformation to [0, -1] and calculate starting angle
    for (auto index : antigen_indexes) {
        const auto coords = layout->get(index);
        const auto& data = blobs.data_for_point(index);
        auto& path = aChartDraw.path();
        path.color(Color(rjson::get_or(args(), "color", "pink")));
        path.line_width(rjson::get_or(args(), "line_width", 1.0));
        for (size_t step = 0; step < data.size(); ++step) {
            const auto angle = base_angle - blobs.angle_step() * step;
            const auto cos = std::cos(-angle), sin = std::sin(-angle);
            const auto x = coords[0] + sin * data[step], y = coords[1] - cos * data[step]; // rotate [0, -data[step]] by angle
            path.add({x, y});
        }
        path.close(Color(rjson::get_or(args(), "fill", "transparent")));

        // std::cerr << ">>> AG " << index << ' ' << aChartDraw.chart().antigen(index)->full_name() << '\n'
        //           << ">>>     " << data << '\n';
    }

    // const auto verbose = rjson::get_or(args(), "report", false);
    // const auto scaling = rjson::get_or(args(), "scaling", false) ? acmacs::chart::procrustes_scaling_t::yes : acmacs::chart::procrustes_scaling_t::no;
    // const auto secondary_projection_no = rjson::get_or(args(), "projection", 0UL);

    // const auto subset_s = rjson::get_or(args(), "subset", "all");
    // acmacs::chart::CommonAntigensSera::subset subset = acmacs::chart::CommonAntigensSera::subset::all;
    // if (subset_s == "sera")
    //     subset = acmacs::chart::CommonAntigensSera::subset::sera;
    // else if (subset_s == "antigens")
    //     subset = acmacs::chart::CommonAntigensSera::subset::antigens;
    // else if (subset_s != "all")
    //     std::cerr << "WARNING: unrecognized common points subset: \"" << subset_s << "\", supported: all, antigens, sera\n";

    // acmacs::chart::ChartP secondary_chart;
    // if (const auto& chart_filename = args()["chart"]; !chart_filename.is_null())
    //     secondary_chart = acmacs::chart::import_from_file(chart_filename, acmacs::chart::Verify::None, do_report_time(verbose));
    // else
    //     secondary_chart = aChartDraw.chartp();
    // const auto match_level = make_match_level(rjson::get_or(args(), "match", ""));
    // acmacs::chart::CommonAntigensSera common(aChartDraw.chart(), *secondary_chart, match_level);
    // if (verbose)
    //     common.report();
    // const auto common_points = common.points(subset);
    // auto secondary_projection = secondary_chart->projection(secondary_projection_no);
    // const auto procrustes_data = acmacs::chart::procrustes(aChartDraw.projection(), *secondary_projection, common_points, scaling);
    // if (aChartDraw.has_title()) {
    //     auto& title = aChartDraw.title();
    //     title.add_line(secondary_chart->make_name(secondary_projection_no));
    //     title.add_line("RMS: " + std::to_string(procrustes_data.rms));
    // }
    // auto secondary_layout = procrustes_data.apply(*secondary_projection->layout());
    // auto primary_layout = aChartDraw.projection().transformed_layout();
    // const auto& arrow_config = args()["arrow"];
    // const auto threshold = rjson::get_or(args(), "threshold", 0.005);
    // for (size_t point_no = 0; point_no < common_points.size(); ++point_no) {
    //     const auto primary_coords = primary_layout->get(common_points[point_no].primary),
    //             secondary_coords = secondary_layout->get(common_points[point_no].secondary);
    //     if (primary_coords.distance(secondary_coords) > threshold) {
    //         auto& arrow = aChartDraw.arrow(primary_coords, secondary_coords);
    //         arrow.color(Color(rjson::get_or(arrow_config, "color", "black")), Color(rjson::get_or(arrow_config, "head_color", "black")));
    //         arrow.line_width(rjson::get_or(arrow_config, "line_width", 1.0));
    //         arrow.arrow_head_filled(rjson::get_or(arrow_config, "head_filled", true));
    //         arrow.arrow_width(rjson::get_or(arrow_config, "arrow_width", 5.0));
    //     }
    // }

} // ModBlobs::apply

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
