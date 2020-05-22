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

    // const auto report_error = [](std::string_view key, const auto& value) { throw acmacs::mapi::unrecognized{fmt::format("unrecognized \"{}\": {}", key, value)}; };

    try {
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
    }
    catch (std::exception& err) {
        throw acmacs::mapi::unrecognized{fmt::format("{} while reading {}", err, getenv_toplevel())};
    }

} // acmacs::mapi::v1::Settings::apply_circle

// ----------------------------------------------------------------------

static inline void read_path_vertices(map_elements::v2::PathData& path, const rjson::v3::value& points, const rjson::v3::value& close, const acmacs::mapi::v1::Settings& settings)
{
    points.visit([&path, &settings]<typename Val>(const Val& value) {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
            for (const auto& en : value) {
                if (const auto coord = ::read_coordinates(en, settings); coord.has_value())
                    path.vertices.push_back(std::move(*coord));
                else
                    throw acmacs::mapi::unrecognized{fmt::format("cannot read vertex from {}", value)};
            }
        }
        else if constexpr (!std::is_same_v<Val, rjson::v3::detail::null>)
            throw acmacs::mapi::unrecognized{fmt::format("cannot read path vertex from {}", value)};
    });

    close.visit([&path]<typename Val>(const Val& value) {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::boolean> || std::is_same_v<Val, rjson::v3::detail::number>)
            path.close = value.template to<bool>();
        else if constexpr (!std::is_same_v<Val, rjson::v3::detail::null>)
            throw acmacs::mapi::unrecognized{fmt::format("cannot read path \"close\" from {}", value)};
    });

} // read_path_vertices

// ----------------------------------------------------------------------

static inline map_elements::v2::ArrowData read_path_arrow(const rjson::v3::value& source)
{
    using namespace std::string_view_literals;

    map_elements::v2::ArrowData result;
    if (const auto at = ::read_from_number<size_t>(source["at"sv]); at.has_value())
        result.at(*at);
    if (const auto from = ::read_from_number<size_t>(source["from"sv]); from.has_value())
        result.from(*from);
    ::read_fill_outline(result, source["fill"sv], source["outline"sv], source["outline_width"sv]);

    return result;

} // read_path_arrow

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_path()
{
    using namespace std::string_view_literals;
    try {
        auto& path = chart_draw().map_elements().add<map_elements::v2::Path>();
        ::read_path_vertices(path.data(), getenv("points"sv), getenv("close"sv), *this);
        ::read_fill_outline(path, getenv("fill"sv), getenv("outline"sv), getenv("outline_width"sv));

        getenv("arrows"sv).visit([&path]<typename Val>(const Val& value) {
            if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
                for (const auto& en : value)
                    path.arrows().push_back(::read_path_arrow(en));
            }
            else if constexpr (!std::is_same_v<Val, rjson::v3::detail::null>)
                throw acmacs::mapi::unrecognized{fmt::format("cannot read path \"arrows\" from {}", value)};
        });

        return true;
    }
    catch (std::exception& err) {
        throw acmacs::mapi::unrecognized{fmt::format("{} while reading {}", err, getenv_toplevel())};
    }

} // acmacs::mapi::v1::Settings::apply_path

// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
