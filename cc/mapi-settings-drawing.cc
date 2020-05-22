#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/map-elements-v2.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

    // # {"v": [x, y]} -- viewport based, top left corner of viewport is 0,0
    // # {"l": [x, y]} -- non transformed layout based
    // # {"t": [x, y]} -- transformed layout based
    // # {"a": {<antigen-select>}} -- if multiple antigens selected, middle point of them used
    // # {"s": {<serum-select>}} -- if multiple antigens selected, middle point of them used

static inline std::optional<map_elements::v2::Coordinates> read_coordinates(const rjson::v3::value& source, const acmacs::mapi::v1::Settings& settings)
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

    const auto read = [read_values, &settings](const rjson::v3::detail::object& obj) -> map_elements::v2::Coordinates {
        if (const auto& v_val = obj["v"sv]; !v_val.is_null())
            return map_elements::v2::Coordinates::viewport{read_values(v_val)};
        else if (const auto& l_val = obj["l"sv]; !l_val.is_null())
            return map_elements::v2::Coordinates::layout{read_values(l_val)};
        else if (const auto& t_val = obj["t"sv]; !t_val.is_null())
            return map_elements::v2::Coordinates::transformed{read_values(t_val)};
        else if (const auto& a_val = obj["a"sv]; !a_val.is_null())
            return map_elements::v2::Coordinates::points{settings.select_antigens(a_val)};
        else if (const auto& s_val = obj["s"sv]; !s_val.is_null()) {
            return map_elements::v2::Coordinates::points{settings.select_sera(s_val).serum_index_to_point(settings.chart_draw().chart().number_of_antigens())};
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

template <typename Target> static inline std::optional<Target> read_from_number(const rjson::v3::value& source)
{
    return source.visit([]<typename Val>(const Val& value) -> std::optional<Target> {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::number>)
            return value.template to<Target>();
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::null>)
            return std::nullopt;
        else
            throw acmacs::mapi::unrecognized{fmt::format("unrecognized: {}", value)};
    });

} // read_from_number

// ----------------------------------------------------------------------

static inline std::optional<acmacs::color::Modifier> read_from_color(const rjson::v3::value& source)
{
    return source.visit([]<typename Val>(const Val& value) -> std::optional<acmacs::color::Modifier> {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::string>)
            return acmacs::color::Modifier{value.template to<std::string_view>()};
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::null>)
            return std::nullopt;
        else
            throw acmacs::mapi::unrecognized{fmt::format("unrecognized: {}", value)};
    });

} // read_from_number

// ----------------------------------------------------------------------

static inline std::optional<std::string_view> read_from_string(const rjson::v3::value& source)
{
    return source.visit([]<typename Val>(const Val& value) -> std::optional<std::string_view> {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::string>)
            return value.template to<std::string_view>();
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::null>)
            return std::nullopt;
        else
            throw acmacs::mapi::unrecognized{fmt::format("unrecognized: {}", value)};
    });

} // read_from_string

// ----------------------------------------------------------------------

template <typename Target> static inline void read_fill_outline(Target& target, const rjson::v3::value& fill, const rjson::v3::value& outline, const rjson::v3::value& outline_width)
{
    if (const auto fill_v = ::read_from_color(fill); fill_v.has_value())
        target.fill(*fill_v);
    if (const auto outline_v = ::read_from_color(outline); outline_v.has_value())
        target.outline(*outline_v);
    if (const auto outline_width_v = ::read_from_number<Pixels>(outline_width); outline_width_v.has_value())
        target.outline_width(*outline_width_v);

} // read_fill_outline

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_circle()
{
    using namespace std::string_view_literals;

    auto& circle = chart_draw().map_elements().add<map_elements::v2::Circle>();

    if (const auto coord = ::read_coordinates(getenv("center"sv), *this); coord.has_value())
        circle.center(*coord);
    ::read_fill_outline(circle, getenv("fill"sv), getenv("outline"sv), getenv("outline_width"sv));

    if (const auto radius = ::read_from_number<Scaled>(getenv("radius"sv)); radius.has_value())
        circle.radius(*radius);
    if (const auto aspect = ::read_from_number<Aspect>(getenv("aspect"sv)); aspect.has_value())
        circle.aspect(*aspect);
    if (const auto rotation = ::read_from_number<Rotation>(getenv("rotation"sv)); rotation.has_value())
        circle.rotation(*rotation);

    return true;

} // acmacs::mapi::v1::Settings::apply_circle

// ----------------------------------------------------------------------

static inline void read_path_vertices(std::vector<map_elements::v2::Coordinates>& path, const rjson::v3::value& points, const acmacs::mapi::v1::Settings& settings)
{
    points.visit([&path, &settings]<typename Val>(const Val& value) {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
            for (const auto& en : value) {
                if (const auto coord = ::read_coordinates(en, settings); coord.has_value())
                    path.push_back(std::move(*coord));
                else
                    throw acmacs::mapi::unrecognized{fmt::format("cannot read vertex from {}", value)};
            }
        }
        else if constexpr (!std::is_same_v<Val, rjson::v3::detail::null>)
            throw acmacs::mapi::unrecognized{fmt::format("cannot read path vertex from {}", value)};
    });

} // read_path_vertices

// ----------------------------------------------------------------------

static inline void read_path_data(map_elements::v2::PathData& path, const rjson::v3::value& points, const rjson::v3::value& close, const acmacs::mapi::v1::Settings& settings)
{
    read_path_vertices(path.vertices, points, settings);
    // points.visit([&path, &settings]<typename Val>(const Val& value) {
    //     if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
    //         for (const auto& en : value) {
    //             if (const auto coord = ::read_coordinates(en, settings); coord.has_value())
    //                 path.vertices.push_back(std::move(*coord));
    //             else
    //                 throw acmacs::mapi::unrecognized{fmt::format("cannot read vertex from {}", value)};
    //         }
    //     }
    //     else if constexpr (!std::is_same_v<Val, rjson::v3::detail::null>)
    //         throw acmacs::mapi::unrecognized{fmt::format("cannot read path vertex from {}", value)};
    // });

    close.visit([&path]<typename Val>(const Val& value) {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::boolean> || std::is_same_v<Val, rjson::v3::detail::number>)
            path.close = value.template to<bool>();
        else if constexpr (!std::is_same_v<Val, rjson::v3::detail::null>)
            throw acmacs::mapi::unrecognized{fmt::format("cannot read path \"close\" from {}", value)};
    });

} // read_path_data

// ----------------------------------------------------------------------

static inline map_elements::v2::ArrowData read_path_arrow(const rjson::v3::value& source, size_t path_size)
{
    using namespace std::string_view_literals;

    map_elements::v2::ArrowData result;
    if (const auto at = ::read_from_number<size_t>(source["at"sv]); at.has_value())
        result.at(*at);
    if (const auto from = ::read_from_number<size_t>(source["from"sv]); from.has_value())
        result.from(*from);
    if (const auto width = ::read_from_number<Pixels>(source["width"sv]); width.has_value())
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
    ::read_path_data(path.data(), getenv("points"sv), getenv("close"sv), *this);
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

// http://geomalgorithms.com/a03-_inclusion.html
// returns winding number, i.e. 0 if point is outside polygon defined by path, non-zero otherwise
static inline int winding_number(const acmacs::PointCoordinates& point, const std::vector<acmacs::PointCoordinates>& path)
{
    // >0 for point left of the line through p0 and p1
    // =0 for point on the line
    // <0 for point right of the line
    const auto is_left = [&point](auto p0, auto p1) -> double { return ((p1->x() - p0->x()) * (point.y() - p0->y()) - (point.x() - p0->x()) * (p1->y() - p0->y())); };

    int wn{0};
    auto path_end = std::prev(path.end(), path.front() == path.back() ? 1 : 0);
    for (auto vi = path.begin(); vi != path_end; ++vi) {
        auto vi_next = std::next(vi);
        if (vi_next == path_end)
            vi_next = path.begin();
        if (vi->y() <= point.y()) {
            if (vi_next->y() > point.y() && is_left(vi, vi_next) > 0)
                ++wn;
        }
        else {
            if (vi_next->y() <= point.y() && is_left(vi, vi_next) < 0)
                --wn;
        }
    }
    return wn;
}

void acmacs::mapi::v1::Settings::filter_inside_path(acmacs::chart::PointIndexList& indexes, const rjson::v3::value& points, size_t index_base) const
{
    std::vector<map_elements::v2::Coordinates> path_vertices;
    ::read_path_vertices(path_vertices, points, *this);
    std::vector<acmacs::PointCoordinates> path;
    std::transform(std::begin(path_vertices), std::end(path_vertices), std::back_inserter(path), [this](const auto& vertex) { return vertex.get(chart_draw()); });
    // AD_DEBUG("filter_inside_path {}", path);

    auto layout = chart_draw().transformed_layout();
    const auto outside = [index_base, &path, &layout](auto index) -> bool { return winding_number(layout->at(index + index_base), path) == 0; };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), outside), indexes.end());

} // acmacs::mapi::v1::Settings::filter_inside_path

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_rotate()
{
    using namespace std::string_view_literals;
    if (const auto degrees = ::read_from_number<double>(getenv("degrees"sv)); degrees.has_value())
        chart_draw().rotate(*degrees * std::acos(-1) / 180.0);
    else if (const auto radians = ::read_from_number<double>(getenv("radians"sv)); radians.has_value())
        chart_draw().rotate(*radians);
    else
        throw acmacs::mapi::unrecognized{"neither \"degrees\" nor \"radians\" found"};
    return true;

} // acmacs::mapi::v1::Settings::apply_rotate

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_flip()
{
    using namespace std::string_view_literals;
    if (const auto direction = ::read_from_string(getenv("direction"sv)); direction.has_value()) {
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
            chart_draw().calculate_viewport("mapi::v1::Settings::apply_viewport [rel]");
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
    if (const auto color = ::read_from_color(getenv("color"sv)); color.has_value())
        chart_draw().background_color(*color);

    return true;

} // acmacs::mapi::v1::Settings::apply_background

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_border()
{
    using namespace std::string_view_literals;

    Color border_color{BLACK};
    if (const auto color = ::read_from_color(getenv("color"sv)); color.has_value())
        border_color = *color;
    double border_line_width{1.0};
    if (const auto line_width = ::read_from_number<double>(getenv("line_width"sv)); line_width.has_value())
        border_line_width = *line_width;
    chart_draw().border(border_color, border_line_width);

    return true;

} // acmacs::mapi::v1::Settings::apply_border

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_grid()
{
    using namespace std::string_view_literals;

    Color grid_color{0xCCCCCC}; // grey80
    if (const auto color = ::read_from_color(getenv("color"sv)); color.has_value())
        grid_color = *color;
    double grid_line_width{1.0};
    if (const auto line_width = ::read_from_number<double>(getenv("line_width"sv)); line_width.has_value())
        grid_line_width = *line_width;

    chart_draw().grid(grid_color, grid_line_width);

    return true;

} // acmacs::mapi::v1::Settings::apply_grid

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_point_scale()
{
    using namespace std::string_view_literals;

    double point_scale{1.0};
    if (const auto scale = ::read_from_number<double>(getenv("scale"sv)); scale.has_value())
        point_scale = *scale;
    double point_outline_scale{1.0};
    if (const auto outline_scale = ::read_from_number<double>(getenv("outline_scale"sv)); outline_scale.has_value())
        point_outline_scale = *outline_scale;

    AD_DEBUG("apply_point_scale {} {}", point_scale, point_outline_scale);
    chart_draw().scale_points(point_scale, point_outline_scale);

    return true;

} // acmacs::mapi::v1::Settings::apply_point_scale

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
