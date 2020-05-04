#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_antigens()
{
    if (const auto& select_clause = getenv("select"); !select_clause.is_null()) {
    }
    else {
        throw unrecognized{"no \"select\" clause"};
    }

    return true;

    // const auto verbose = rjson::get_or(args(), "report", false);
    // const auto report_names_threshold = rjson::get_or(args(), "report_names_threshold", 30UL);
    // if (const auto& select = args()["select"]; !select.is_null()) {
    //     const auto indices = SelectAntigens(acmacs::verbose_from(verbose), report_names_threshold).select(aChartDraw, select);
    //     const auto styl = style();

    //     // AD_DEBUG("{}", styl);

    //     aChartDraw.modify(indices, styl, drawing_order());
    //     if (const auto& label = args()["label"]; !label.is_null())
    //         add_labels(aChartDraw, indices, 0, label);
    //     if (const auto& legend = args()["legend"]; !legend.is_null())
    //         add_legend(aChartDraw, indices, styl, legend);
    // }
    // else {
    //     throw unrecognized_mod{"no \"select\" in \"antigens\" mod: " + rjson::format(args())};
    // }

} // acmacs::mapi::v1::Settings::apply_antigens

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_sera()
{
    if (const auto& select_clause = getenv("select"); !select_clause.is_null()) {
    }
    else {
        throw unrecognized{"no \"select\" clause"};
    }

    return true;

    // const auto verbose = rjson::get_or(args(), "report", false);
    // if (const auto& select = args()["select"]; !select.is_null()) {
    //     auto& projection = aChartDraw.projection();
    //     if (auto relative = args().get("relative"); !relative.is_null()) {
    //         auto layout = aChartDraw.layout();
    //         for (auto index : SelectSera(acmacs::verbose_from(verbose)).select(aChartDraw, select)) {
    //             const auto coord = layout->at(index + aChartDraw.number_of_antigens());
    //             projection.move_point(index + aChartDraw.number_of_antigens(), acmacs::PointCoordinates(coord.x() + relative[0].to<double>(), coord.y() + relative[1].to<double>()));
    //         }
    //     }
    //     else if (const auto& flip_line = args()["flip_over_line"]; !flip_line.is_null()) {
    //         acmacs::PointCoordinates from{flip_line["from"][0].to<double>(), flip_line["from"][1].to<double>()}, to{flip_line["to"][0].to<double>(), flip_line["to"][1].to<double>()};
    //         if (!rjson::get_or(flip_line, "transform", true)) {
    //             const auto transformation = aChartDraw.transformation().inverse();
    //             from = transformation.transform(from);
    //             to = transformation.transform(to);
    //         }
    //         const acmacs::LineDefinedByEquation line(from, to);
    //         auto layout = aChartDraw.layout();
    //         for (auto index : SelectSera(acmacs::verbose_from(verbose)).select(aChartDraw, select)) {
    //             const auto flipped = line.flip_over(layout->at(index + aChartDraw.number_of_antigens()), 1.0);
    //             projection.move_point(index + aChartDraw.number_of_antigens(), flipped);
    //         }
    //     }
    //     else {
    //         const auto move_to = get_move_to(aChartDraw, verbose);
    //         for (auto index : SelectSera(acmacs::verbose_from(verbose)).select(aChartDraw, select)) {
    //             projection.move_point(index + aChartDraw.number_of_antigens(), move_to);
    //         }
    //     }
    // }
    // else {
    //     throw unrecognized_mod{"no \"select\" in \"move_sera\" mod: " + rjson::format(args())};
    // }

} // acmacs::mapi::v1::Settings::apply_sera

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
