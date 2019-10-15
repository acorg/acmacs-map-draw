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
    const auto antigen_indexes = SelectAntigens(verbose ? acmacs::verbose::yes : acmacs::verbose::no).select(aChartDraw, select.is_null() ? rjson::value{"all"} : select);

    const auto blobs = projection.blobs(rjson::get_or(args(), "stress_diff", 0.5), antigen_indexes, rjson::get_or(args(), "number_of_drections", 36UL), rjson::get_or(args(), "stress_diff_precision", 1e-5));
    const auto transformation = aChartDraw.projection().transformation();
    const auto layout = aChartDraw.projection().layout();
    for (auto index : antigen_indexes) {
        const auto coords = layout->get(index);
        const auto& data = blobs.data_for_point(index);
        auto& path = aChartDraw.path().color(Color(rjson::get_or(args(), "color", "pink"))).line_width(rjson::get_or(args(), "line_width", 1.0));

        for (size_t step = 0; step < data.size(); ++step) {
            acmacs::PointCoordinates vertix{coords.x() + std::cos(blobs.angle_step() * step) * data[step], coords.y() + std::sin(blobs.angle_step() * step) * data[step]};
            // aChartDraw.line(transformation.transform(coords), transformation.transform(vertix)) .color(RED).line_width(2);
            path.add(transformation.transform(vertix));
        }
        path.close(Color(rjson::get_or(args(), "fill", "transparent")));

        // aChartDraw.line(coords, acmacs::PointCoordinates(coords[0] + data[0], coords[1])).color(BLUE).line_width(2);
        // aChartDraw.line(coords, acmacs::PointCoordinates(coords[0] + std::cos(blobs.angle_step()) * data[1], coords[1] + std::sin(blobs.angle_step()) * data[1])).color(RED).line_width(2);
        // std::cerr << "DEBUG: data[0] " << data[0] << " angle_step " << blobs.angle_step() << '\n';

        // auto& path = aChartDraw.path().color(Color(rjson::get_or(args(), "color", "pink"))).line_width(rjson::get_or(args(), "line_width", 1.0));
        // for (size_t step = 0; step < data.size(); ++step) {
        //     acmacs::PointCoordinates vertix{data[step], 0};
        //     const auto angle = blobs.angle_step() * step;
        //     const auto cos = std::cos(angle), sin = std::sin(angle);
        //     auto v2 = transformation.transform(acmacs::PointCoordinates{coords[0] + cos * vertix[0] - sin * vertix[1], sin * vertix[0] + cos * vertix[1]});
        //     path.add(v2);

        //     auto& line = aChartDraw.line(transformation.transform(coords), v2).color(CYAN).line_width(1);

        // }
        // path.close(Color(rjson::get_or(args(), "fill", "transparent")));

        // std::cerr << ">>> AG " << index << ' ' << aChartDraw.chart().antigen(index)->full_name() << '\n'
        //           << ">>>     " << data << '\n';
    }

} // ModBlobs::apply

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
