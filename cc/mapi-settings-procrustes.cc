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

bool acmacs::mapi::v1::Settings::apply_procrustes()
{
    using namespace std::string_view_literals;
    using namespace acmacs::chart;

    const auto scaling = rjson::v3::read_bool(getenv("scaling"sv), false) ? procrustes_scaling_t::yes : procrustes_scaling_t::no;
    const auto& secondary_chart = get_chart(getenv("chart"sv));
    const auto secondary_projection_no = rjson::v3::read_number(getenv("projection"sv), 0ul);
    if (secondary_projection_no >= secondary_chart.number_of_projections())
        throw error{fmt::format("invalid secondary chart projection number {} (chart has just {} projection(s))", secondary_projection_no, secondary_chart.number_of_projections())};
    const auto threshold = rjson::v3::read_number(getenv("threshold"sv), 0.005);

    // arrow plot spec
    Pixels line_width{1}, arrow_width{5}, arrow_outline_width{1};
    acmacs::color::Modifier outline{BLACK}, arrow_fill{BLACK}, arrow_outline{BLACK};
    if (const auto& arrow_data = getenv("arrow"sv); arrow_data.is_object()) {
        line_width = rjson::v3::read_number(arrow_data["line_width"sv], line_width);
        outline = rjson::v3::read_color(arrow_data["outline"sv], outline);
        if (const auto& head_data = arrow_data["head"sv]; head_data.is_object()) {
            arrow_width = rjson::v3::read_number(head_data["width"sv], arrow_width);
            arrow_outline_width = rjson::v3::read_number(head_data["outline_width"sv], arrow_outline_width);
            arrow_outline = rjson::v3::read_color(head_data["outline"sv], arrow_outline);
            arrow_fill = rjson::v3::read_color(head_data["fill"sv], arrow_fill);
        }
        else if (!head_data.is_null())
            AD_WARNING("invalid \"arrow\" \"head\": {} (object expected)", head_data);
    }
    else if (!arrow_data.is_null())
        AD_WARNING("invalid \"arrow\": {} (object expected)", arrow_data);

    // common points
    const auto match_level = CommonAntigensSera::match_level(rjson::v3::read_string(getenv("match"sv), "auto"sv));
    CommonAntigensSera common(chart_draw().chart(), secondary_chart, match_level);
    common.keep_only(select_antigens(getenv("antigens"sv), if_null::all), select_sera(getenv("sera"sv), if_null::all));
    std::vector<CommonAntigensSera::common_t> common_points;
    common_points = common.points(CommonAntigensSera::subset::all);

    auto secondary_projection = secondary_chart.projection(secondary_projection_no);
    const auto procrustes_data = procrustes(chart_draw().projection(), *secondary_projection, common_points, scaling);

    auto secondary_layout = procrustes_data.apply(*secondary_projection->layout());
    auto primary_layout = chart_draw().projection().transformed_layout();
    for (size_t point_no = 0; point_no < common_points.size(); ++point_no) {
        const auto primary_coords = primary_layout->at(common_points[point_no].primary), secondary_coords = secondary_layout->at(common_points[point_no].secondary);
        if (acmacs::distance(primary_coords, secondary_coords) > threshold) {
            auto& path = chart_draw().map_elements().add<map_elements::v2::Path>();
            path.outline(outline);
            path.outline_width(line_width);
            path.data().close = false;
            path.data().vertices.emplace_back(map_elements::v2::Coordinates::not_transformed{primary_coords});
            path.data().vertices.emplace_back(map_elements::v2::Coordinates::not_transformed{secondary_coords});
            auto& arrow = path.arrows().emplace_back();
            arrow.at(1);
            arrow.fill(arrow_fill);
            arrow.outline(arrow_outline);
            arrow.width(arrow_width);
            arrow.outline_width(arrow_outline_width);
        }
    }

    auto& titl = title();
    if (titl.number_of_lines() == 0)
        titl.add_line(chart_draw().chart().make_name(chart_draw().projection_no()));
    titl.add_line(secondary_chart.make_name(secondary_projection_no));
    titl.add_line(fmt::format("RMS: {:.6f}", procrustes_data.rms));

    if (rjson::v3::read_bool(getenv("report"sv), false))
        AD_INFO("Procrustes  AG:{}  SR:{}  RMS: {:.6f}\n\n{}", common.common_antigens(), common.common_sera(), procrustes_data.rms, common.report(2, verbose_from(rjson::v3::read_bool(getenv("verbose"sv), false))));
    else
        AD_INFO("Procrustes  AG:{}  SR:{}  RMS: {:.6f}", common.common_antigens(), common.common_sera(), procrustes_data.rms);

    return true;

} // acmacs::mapi::v1::Settings::apply_procrustes

// ----------------------------------------------------------------------

// {"N": "move", "antigens": {<antigen-select>}, "sera": {<serum-select>}, "report": true,
//    "?to": <point>, "?relative": [1, 1],
//    # "flip_over_line": {"from": <point>, "to": <point>},
//    # "flip_over_serum_line": 1 -- scale (1 - mirror, 0.1 - close to serum line, 0 - move to serum line)
//            }

bool acmacs::mapi::v1::Settings::apply_move()
{
    using namespace std::string_view_literals;
    auto antigen_indexes = select_antigens(getenv("antigens"sv), if_null::empty);
    auto serum_indexes = select_sera(getenv("sera"sv), if_null::empty);
    auto& projection = chart_draw().projection();
    const auto number_of_antigens{chart_draw().number_of_antigens()};

    if (const auto to = read_coordinates(getenv("to"sv)); to.has_value()) {
        const auto move_to{to->get_not_transformed(chart_draw())};
        for (auto index : antigen_indexes)
            projection.move_point(index, move_to);
        for (auto index : serum_indexes)
            projection.move_point(index + number_of_antigens, move_to);
    }

    if (rjson::v3::read_bool(getenv("report"sv), false))
        AD_INFO("Moved AG:{} SR:{}", antigen_indexes.size(), serum_indexes.size());

    return true;

} // acmacs::mapi::v1::Settings::apply_move

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
