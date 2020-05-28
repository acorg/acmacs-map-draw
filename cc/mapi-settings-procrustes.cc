#include "acmacs-base/rjson-v3-helper.hh"
#include "acmacs-chart-2/procrustes.hh"
#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/map-elements-v2.hh"

// ----------------------------------------------------------------------

const acmacs::chart::Chart& acmacs::mapi::v1::Settings::get_chart(const rjson::v3::value& source)
{
    return source.visit([this]<typename Val>(const Val& val) -> const acmacs::chart::Chart& {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::null>) {
            if (chart_draw().number_of_charts() > 1)
                return chart_draw().chart(1);
            else
                return chart_draw().chart(0); // internal procrustes
        }
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::number>) {
            if (const auto no{val.template to<size_t>()}; chart_draw().number_of_charts() > no)
                return chart_draw().chart(no);
            else
                throw error{fmt::format("cannot make procrustes, too few charts provided, required chart {} but just {} available", no, chart_draw().number_of_charts())};
        }
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::string>) {
            return chart_draw().chart(val.template to<std::string_view>());
        }
        else
            throw error{fmt::format("unrecognized \"chart\" for procrustes: {} (expected integer or name)", val)};
    });

} // acmacs::mapi::v1::Settings::get_chart

// ----------------------------------------------------------------------

// {"N": "procrustes-arrows",
//    "chart": <filename or index>, "projection": 0,
//    "match": "auto", -- "auto", "strict", "relaxed", "ignored"
//    "antigens": "<select-antigens>", "sera": "<select-sera>",
// }

bool acmacs::mapi::v1::Settings::apply_procrustes()
{
    using namespace std::string_view_literals;
    using namespace acmacs::chart;

    const auto scaling = rjson::v3::read_bool(getenv("scaling"sv), false) ? procrustes_scaling_t::yes : procrustes_scaling_t::no;
    const auto secondary_projection_no = rjson::v3::read_number(getenv("projection"sv), 0ul);

    // const auto subset = CommonAntigensSera::subset::all;
    const auto antigen_indexes = select_antigens(getenv("antigens"sv), if_null::all);
    const auto serum_indexes = select_sera(getenv("sera"sv), if_null::all);
    const auto threshold = rjson::v3::read_number(getenv("threshold"sv), 0.005);

    const auto& secondary_chart = get_chart(getenv("chart"sv));
    const auto match_level = CommonAntigensSera::match_level(rjson::v3::read_string(getenv("match"sv), "i auto"sv));

    Pixels line_width{1};
    acmacs::color::Modifier outline{BLACK};
    if (const auto& arrow_data = getenv("arrow"sv); arrow_data.is_object()) {
        line_width = rjson::v3::read_number(arrow_data["line_width"sv], line_width);
        outline = rjson::v3::read_color(arrow_data["outline"sv], outline);
        // "head": {"width": 5, "outline": "black", "outline_width": 1, "fill": "black"}}
    }
    else if (!arrow_data.is_null())
        AD_WARNING("invalid \"arrow\": {} (object expected)", arrow_data);

    CommonAntigensSera common(chart_draw().chart(), secondary_chart, match_level);
    std::vector<CommonAntigensSera::common_t> common_points;
    common_points = common.points(CommonAntigensSera::subset::all);
    // if (!antigen_indexes->empty())
    //     common_points = common.points_for_primary_antigens(antigen_indexes);
    // else if (!serum_indexes->empty())
    //     common_points = common.points_for_primary_sera(serum_indexes);
    // else
    //     common_points = common.points(subset);
    // common.report();

    auto secondary_projection = secondary_chart.projection(secondary_projection_no);
    const auto procrustes_data = procrustes(chart_draw().projection(), *secondary_projection, common_points, scaling);
    // if (aChartDraw.has_title()) {
    //     auto& title = aChartDraw.title();
    //     title.add_line(secondary_chart->make_name(secondary_projection_no));
    //     title.add_line("RMS: " + std::to_string(procrustes_data.rms));
    // }
    auto secondary_layout = procrustes_data.apply(*secondary_projection->layout());
    auto primary_layout = chart_draw().projection().transformed_layout();
    // const auto& arrow_config = args()["arrow"];
    AD_DEBUG("common_points {}", common_points.size());
    for (size_t point_no = 0; point_no < common_points.size(); ++point_no) {
        const auto primary_coords = primary_layout->at(common_points[point_no].primary),
                secondary_coords = secondary_layout->at(common_points[point_no].secondary);
        if (acmacs::distance(primary_coords, secondary_coords) > threshold) {
            auto& path = chart_draw().map_elements().add<map_elements::v2::Path>();
            path.outline(outline);
            path.outline_width(line_width);
            path.data().close = false;
            path.data().vertices.emplace_back(map_elements::v2::Coordinates::not_transformed{primary_coords});
            path.data().vertices.emplace_back(map_elements::v2::Coordinates::not_transformed{secondary_coords});
            auto& arrow = path.arrows().emplace_back();
            arrow.at(1);
            // arrow.fill();
            // arrow.outline();
            // arrow.width();
            // arrow.outline_width();

            // auto& arrow = chart_draw().arrow(primary_coords, secondary_coords);
            // arrow.arrow_head_filled(rjson::get_or(arrow_config, "head_filled", true));
            // arrow.arrow_width(rjson::get_or(arrow_config, "arrow_width", 5.0));
        }
    }

    if (rjson::v3::read_bool(getenv("report"sv), false))
        AD_INFO("Procrustes\n  common antigens: {}\n  common sera:     {}\n  RMS:          {:.6f}", common.common_antigens(), common.common_sera(), procrustes_data.rms);

    return true;

} // acmacs::mapi::v1::Settings::apply_procrustes

// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
