#include "acmacs-base/rjson-v3-helper.hh"
#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/map-elements-v2.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/mapi-procrustes.hh"

// ----------------------------------------------------------------------

template <typename Target> static inline void read_fill_outline(Target& target, const rjson::v3::value& fill, const rjson::v3::value& outline, const rjson::v3::value& outline_width)
{
    if (const auto fill_v = rjson::v3::read_color(fill); fill_v.has_value())
        target.fill(*fill_v);
    if (const auto outline_v = rjson::v3::read_color(outline); outline_v.has_value())
        target.outline(*outline_v);
    if (const auto outline_width_v = rjson::v3::read_number<Pixels>(outline_width); outline_width_v.has_value())
        target.outline_width(*outline_width_v);

} // read_fill_outline

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_circle()
{
    using namespace std::string_view_literals;

    auto& circle = chart_draw().map_elements().add<map_elements::v2::Circle>();

    if (const auto coord = read_coordinates(getenv("center"sv)); coord.has_value())
        circle.center(*coord);
    ::read_fill_outline(circle, getenv("fill"sv), getenv("outline"sv), getenv("outline_width"sv));

    if (const auto radius = rjson::v3::read_number<Scaled>(getenv("radius"sv)); radius.has_value())
        circle.radius(*radius);
    if (const auto aspect = rjson::v3::read_number<Aspect>(getenv("aspect"sv)); aspect.has_value())
        circle.aspect(*aspect);
    if (const auto rotation = rjson::v3::read_number<Rotation>(getenv("rotation"sv)); rotation.has_value())
        circle.rotation(*rotation);

    return true;

} // acmacs::mapi::v1::Settings::apply_circle

// ----------------------------------------------------------------------

acmacs::mapi::v1::FigureRaw acmacs::mapi::v1::Settings::read_figure(const rjson::v3::value& points, const rjson::v3::value& close) const
{
    FigureRaw figure;
    points.visit([&figure, this]<typename Val>(const Val& value) {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
            for (const auto& en : value) {
                if (const auto coord = read_coordinates(en); coord.has_value())
                    figure.vertices.push_back(std::move(*coord));
                else
                    throw acmacs::mapi::unrecognized{fmt::format("cannot read vertex from {}", value)};
            }
        }
        else if constexpr (!std::is_same_v<Val, rjson::v3::detail::null>)
            throw acmacs::mapi::unrecognized{fmt::format("cannot read figure vertex from {}", value)};
    });

    close.visit([&figure]<typename Val>(const Val& value) {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::boolean> || std::is_same_v<Val, rjson::v3::detail::number>)
            figure.close = value.template to<bool>();
        else if constexpr (!std::is_same_v<Val, rjson::v3::detail::null>)
            throw acmacs::mapi::unrecognized{fmt::format("cannot read figure \"close\" from {}", value)};
    });
    return figure;

} // read_figure

// ----------------------------------------------------------------------

static inline map_elements::v2::ArrowData read_path_arrow(const rjson::v3::value& source, size_t path_size)
{
    using namespace std::string_view_literals;

    map_elements::v2::ArrowData result;
    if (const auto at = rjson::v3::read_number<size_t>(source["at"sv]); at.has_value())
        result.at(*at);
    if (const auto from = rjson::v3::read_number<size_t>(source["from"sv]); from.has_value())
        result.from(*from);
    if (const auto width = rjson::v3::read_number<Pixels>(source["width"sv]); width.has_value())
        result.width(*width);
    ::read_fill_outline(result, source["fill"sv], source["outline"sv], source["outline_width"sv]);

    if (!result.valid(path_size))
        AD_WARNING("invalid path arrow specification: {} (path size: {})", source, path_size);

    return result;

} // read_path_arrow

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_path()
{
    using namespace std::string_view_literals;
    auto& path = chart_draw().map_elements().add<map_elements::v2::Path>();
    path.data() = Figure{read_figure(getenv("points"sv), getenv("close"sv)), chart_draw()};
    ::read_fill_outline(path, getenv("fill"sv), getenv("outline"sv), getenv("outline_width"sv));

    getenv("arrows"sv).visit([&path]<typename Val>(const Val& value) {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
            for (const auto& en : value)
                path.arrows().push_back(::read_path_arrow(en, path.data().vertices.size()));
        }
        else if constexpr (!std::is_same_v<Val, rjson::v3::detail::null>)
            throw acmacs::mapi::unrecognized{fmt::format("cannot read path \"arrows\" from {}", value)};
    });

    return true;

} // acmacs::mapi::v1::Settings::apply_path

// ----------------------------------------------------------------------

void acmacs::mapi::v1::Settings::filter_inside_path(acmacs::chart::PointIndexList& indexes, const rjson::v3::value& points, size_t index_base) const
{
    const Figure figure{read_figure(points, rjson::v3::const_true), chart_draw()};
    chart_draw().viewport_reset_used_by(); // to avoid warning when viewport is later changed (e.g. set)
    auto layout = chart_draw().chart(0).modified_transformed_layout();
    const auto outside = [index_base, &figure, &layout](auto index) -> bool { return figure.outside(layout->at(index + index_base)); };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), outside), indexes.end());

} // acmacs::mapi::v1::Settings::filter_inside_path

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_rotate()
{
    using namespace std::string_view_literals;
    if (const auto degrees = rjson::v3::read_number<double>(getenv("degrees"sv)); degrees.has_value())
        chart_draw().rotate(*degrees * std::acos(-1) / 180.0);
    else if (const auto radians = rjson::v3::read_number<double>(getenv("radians"sv)); radians.has_value())
        chart_draw().rotate(*radians);
    else if (const auto angle = rjson::v3::read_number<double>(getenv("angle"sv)); angle.has_value()) {
        if (std::abs(*angle) < 4.0)
            chart_draw().rotate(*angle);
        else
            chart_draw().rotate(*angle * std::acos(-1) / 180.0);
    }
    else
        throw acmacs::mapi::unrecognized{"neither of \"angle\", \"degrees\", \"radians\" found"};
    return true;

} // acmacs::mapi::v1::Settings::apply_rotate

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_flip()
{
    using namespace std::string_view_literals;
    if (const auto direction = rjson::v3::read_string(getenv("direction"sv)); direction.has_value()) {
        if (*direction == "ew"sv)
            chart_draw().flip(0, 1);
        else if (*direction == "ns"sv)
            chart_draw().flip(1, 0);
        else
            throw acmacs::mapi::unrecognized{"unrecognized \"direction\""};
    }
    else
        throw acmacs::mapi::unrecognized{"\"direction\" not found"};

    return true;

} // acmacs::mapi::v1::Settings::apply_flip

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_viewport()
{
    using namespace std::string_view_literals;
    bool updated{false};
    getenv("abs"sv).visit([&updated, this]<typename Val>(const Val& value) {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
            if (value.size() != 3)
                throw acmacs::mapi::unrecognized{fmt::format("unrecognized: {} (3 numbers expected)", value)};
            chart_draw().set_viewport({value[0].template to<double>(), value[1].template to<double>()}, value[2].template to<double>());
            updated = true;
        }
        else if constexpr (!std::is_same_v<Val, rjson::v3::detail::null>)
            throw acmacs::mapi::unrecognized{fmt::format("unrecognized: {}", value)};
    });

    getenv("rel"sv).visit([&updated, this]<typename Val>(const Val& value) {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
            if (value.size() != 3)
                throw acmacs::mapi::unrecognized{fmt::format("unrecognized: {} (3 numbers expected)", value)};
            chart_draw().calculate_viewport();
            const auto& orig_viewport = chart_draw().viewport_before_changing();
            const auto new_size = value[2].template to<double>() + orig_viewport.size.width;
            if (new_size < 1)
                throw acmacs::mapi::unrecognized{"invalid size difference in \"rel\""};
            chart_draw().set_viewport(orig_viewport.origin + acmacs::PointCoordinates{value[0].template to<double>(), value[1].template to<double>()}, new_size);
            updated = true;
        }
        else if constexpr (!std::is_same_v<Val, rjson::v3::detail::null>)
            throw acmacs::mapi::unrecognized{fmt::format("unrecognized: {}", value)};
    });

    if (!updated)
        throw acmacs::mapi::unrecognized{"neither \"abs\" nor \"rel\" found"};

    return true;

} // acmacs::mapi::v1::Settings::apply_viewport

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_background()
{
    using namespace std::string_view_literals;
    chart_draw().background_color(acmacs::color::Modifier{WHITE, rjson::v3::read_color_or_empty(getenv("color"sv))});
    return true;

} // acmacs::mapi::v1::Settings::apply_background

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_border()
{
    using namespace std::string_view_literals;
    chart_draw().border(acmacs::color::Modifier{BLACK, rjson::v3::read_color_or_empty(getenv("color"sv))}, rjson::v3::read_number(getenv("line_width"sv), 1.0));
    return true;

} // acmacs::mapi::v1::Settings::apply_border

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_grid()
{
    using namespace std::string_view_literals;
    chart_draw().grid(acmacs::color::Modifier{Color{0xCCCCCC} /* grey80 */, rjson::v3::read_color_or_empty(getenv("color"sv))}, rjson::v3::read_number(getenv("line_width"sv), 1.0));
    return true;

} // acmacs::mapi::v1::Settings::apply_grid

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_point_scale()
{
    using namespace std::string_view_literals;
    chart_draw().scale_points(rjson::v3::read_number(getenv("scale"sv), 1.0), rjson::v3::read_number(getenv("outline_scale"sv), 1.0));
    return true;

} // acmacs::mapi::v1::Settings::apply_point_scale

// ----------------------------------------------------------------------

// static inline void make_line(map_elements::v2::Path& path, map_elements::v2::Coordinates&& p1, map_elements::v2::Coordinates&& p2, const acmacs::color::Modifier& outline, Pixels outline_width)
// {
//     path.data().close = false;
//     path.data().vertices.emplace_back(std::move(p1));
//     path.data().vertices.emplace_back(std::move(p2));
//     path.outline(outline);
//     path.outline_width(outline_width);

// } // make_line

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_connection_lines()
{
    using namespace std::string_view_literals;

    ConnectionLinePlotSpec plot_spec;
    plot_spec.color.add(acmacs::color::Modifier{rjson::v3::read_color_or_empty(getenv("color"sv))});
    plot_spec.line_width = rjson::v3::read_number(getenv("line_width"sv), plot_spec.line_width);

    const auto s_antigens = selected_antigens(getenv("antigens"sv), if_null::all);
    const auto s_sera = selected_sera(getenv("sera"sv), if_null::all);
    connection_lines(chart_draw(), s_antigens, s_sera, plot_spec, getenv("report"sv).to<bool>());
    return true;

    // auto antigen_indexes = select_antigens(getenv("antigens"sv), if_null::all);
    // auto serum_indexes = select_sera(getenv("sera"sv), if_null::all);
    // const auto number_of_antigens = chart_draw().chart().number_of_antigens();
    // auto layout = chart_draw().chart(0).modified_layout();
    // antigen_indexes.remove_if([&layout](size_t index) { return !layout->point_has_coordinates(index); });
    // serum_indexes.remove_if([&layout, number_of_antigens](size_t index) { return !layout->point_has_coordinates(index + number_of_antigens); });

    // const acmacs::color::Modifier connection_line_color{GREY, rjson::v3::read_color_or_empty(getenv("color"sv))};
    // const auto connection_line_width{rjson::v3::read_number(getenv("line_width"sv), Pixels{0.5})};

    // std::vector<std::pair<size_t, size_t>> lines_to_draw;
    // auto titers = chart_draw().chart().titers();
    // for (const auto ag_no : antigen_indexes) {
    //     for (const auto sr_no : serum_indexes) {
    //         if (const auto titer = titers->titer(ag_no, sr_no); !titer.is_dont_care()) {
    //             ::make_line(chart_draw().map_elements().add<map_elements::v2::Path>(), map_elements::v2::Coordinates::points{ag_no}, map_elements::v2::Coordinates::points{sr_no + number_of_antigens},
    //                         connection_line_color, connection_line_width);
    //             lines_to_draw.emplace_back(ag_no, sr_no);
    //         }
    //     }
    // }
    // if (getenv("report"sv).to<bool>())
    //     AD_INFO("connection lines: ({}) {}", lines_to_draw.size(), lines_to_draw);

    // return true;

} // acmacs::mapi::v1::Settings::apply_connection_lines

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_error_lines()
{
    using namespace std::string_view_literals;

    ErrorLinePlotSpec plot_spec;
    plot_spec.more.add(acmacs::color::Modifier{rjson::v3::read_color_or_empty(getenv("more"sv))});
    plot_spec.less.add(acmacs::color::Modifier{rjson::v3::read_color_or_empty(getenv("less"sv))});
    plot_spec.line_width = rjson::v3::read_number(getenv("line_width"sv), plot_spec.line_width);

    const auto s_antigens = selected_antigens(getenv("antigens"sv), if_null::all);
    const auto s_sera = selected_sera(getenv("sera"sv), if_null::all);
    error_lines(chart_draw(), s_antigens, s_sera, plot_spec, getenv("report"sv).to<bool>());
    return true;

    // using namespace std::string_view_literals;
    // auto layout = chart_draw().chart(0).modified_layout();
    // auto antigens = chart_draw().chart().antigens();
    // auto sera = chart_draw().chart().sera();
    // auto antigen_indexes = select_antigens(getenv("antigens"sv), if_null::all);
    // auto serum_indexes = select_sera(getenv("sera"sv), if_null::all);
    // const auto number_of_antigens = chart_draw().chart().number_of_antigens();
    // antigen_indexes.remove_if([&layout](size_t index) { return !layout->point_has_coordinates(index); });
    // serum_indexes.remove_if([&layout, number_of_antigens](size_t index) { return !layout->point_has_coordinates(index + number_of_antigens); });
    // const auto error_lines = chart_draw().chart(0).modified_projection().error_lines();

    // const auto line_width{rjson::v3::read_number(getenv("line_width"sv), Pixels{0.5})};
    // const acmacs::color::Modifier more{RED, rjson::v3::read_color_or_empty(getenv("more"sv))};
    // const acmacs::color::Modifier less{BLUE, rjson::v3::read_color_or_empty(getenv("less"sv))};
    // const auto report{getenv("report"sv).to<bool>()};

    // for (const auto ag_no : antigen_indexes) {
    //     for (const auto sr_no : serum_indexes) {
    //         const auto p2_no = sr_no + number_of_antigens;
    //         if (const auto found = std::find_if(std::begin(error_lines), std::end(error_lines), [p1_no = ag_no, p2_no](const auto& erl) { return erl.point_1 == p1_no && erl.point_2 == p2_no; });
    //             found != std::end(error_lines)) {
    //             if (report)
    //                 AD_INFO("error line {} {} -- {} {} : {}", ag_no, antigens->at(ag_no)->name_full(), sr_no, sera->at(sr_no)->name_full(), found->error_line);
    //             const auto p1 = layout->at(ag_no), p2 = layout->at(p2_no);
    //             const auto v3 = (p2 - p1) / distance(p1, p2) * (-found->error_line) / 2.0;
    //             const auto& color = found->error_line > 0 ? more : less;
    //             ::make_line(chart_draw().map_elements().add<map_elements::v2::Path>(), map_elements::v2::Coordinates::transformed{p1}, map_elements::v2::Coordinates::transformed{p1 + v3}, color, line_width);
    //             ::make_line(chart_draw().map_elements().add<map_elements::v2::Path>(), map_elements::v2::Coordinates::transformed{p2}, map_elements::v2::Coordinates::transformed{p2 - v3}, color, line_width);
    //         }
    //     }
    // }

    // return true;

} // acmacs::mapi::v1::Settings::apply_error_lines

// ----------------------------------------------------------------------

    // # {"v": [x, y]} -- viewport based, top left corner of viewport is 0,0
    // # {"l": [x, y]} -- non transformed layout based
    // # {"t": [x, y]} -- transformed layout based
    // # {"a": {<antigen-select>}} -- if multiple antigens selected, middle point of them used
    // # {"s": {<serum-select>}} -- if multiple antigens selected, middle point of them used

std::optional<map_elements::v2::Coordinates> acmacs::mapi::v1::Settings::read_coordinates(const rjson::v3::value& source) const
{
    using namespace std::string_view_literals;

    const auto read_values = [](const rjson::v3::value& array) -> acmacs::PointCoordinates {
        return array.visit([]<typename Value>(const Value& value) -> acmacs::PointCoordinates {
            if constexpr (std::is_same_v<Value, rjson::v3::detail::array>) {
                switch (value.size()) {
                    case 2:
                        return acmacs::PointCoordinates{value[0].template to<double>(), value[1].template to<double>()};
                    case 3:
                        return acmacs::PointCoordinates{value[0].template to<double>(), value[1].template to<double>(), value[2].template to<double>()};
                }
            }
            throw std::exception{};
        });
    };

    const auto read = [read_values, this](const rjson::v3::detail::object& obj) -> map_elements::v2::Coordinates {
        if (const auto& v_val = obj["v"sv]; !v_val.is_null())
            return map_elements::v2::Coordinates::viewport{read_values(v_val)};
        else if (const auto& l_val = obj["l"sv]; !l_val.is_null())
            return map_elements::v2::Coordinates::not_transformed{read_values(l_val)};
        else if (const auto& t_val = obj["t"sv]; !t_val.is_null())
            return map_elements::v2::Coordinates::transformed{read_values(t_val)};
        else if (const auto& a_val = obj["a"sv]; !a_val.is_null())
            return map_elements::v2::Coordinates::points{select_antigens(a_val)};
        else if (const auto& s_val = obj["s"sv]; !s_val.is_null()) {
            return map_elements::v2::Coordinates::points{select_sera(s_val).serum_index_to_point(chart_draw().chart().number_of_antigens())};
        }
        else
            throw std::exception{};
    };

    try {
        return source.visit([read]<typename Value>(const Value& value) -> std::optional<map_elements::v2::Coordinates> {
            if constexpr (std::is_same_v<Value, rjson::v3::detail::object>)
                return read(value);
            else if constexpr (std::is_same_v<Value, rjson::v3::detail::null>)
                return std::nullopt;
            else
                throw std::exception{};
        });
    }
    catch (std::exception&) {
        throw acmacs::mapi::unrecognized{fmt::format("cannot read map/viewport coordinates from {}", source)};
    }

} // read_coordinates

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
