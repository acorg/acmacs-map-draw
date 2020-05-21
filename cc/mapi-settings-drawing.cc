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

bool acmacs::mapi::v1::Settings::apply_circle()
{
    using namespace std::string_view_literals;

    const auto report_error = [](std::string_view key, const auto& value) { throw acmacs::mapi::unrecognized{fmt::format("unrecognized \"circle\" \"{}\": {}", key, value)}; };

    auto& circle = chart_draw().map_elements().add<map_elements::v2::Circle>();

    // "center": {"l": [0, 0]}

    if (const auto coord = ::read_coordinates(getenv("center"sv), *this); coord.has_value())
        circle.center(*coord);

    getenv("radius"sv).visit([&circle, report_error]<typename Value>(const Value& value) {
        if constexpr (std::is_same_v<Value, rjson::v3::detail::number>)
            circle.radius(value.template to<Scaled>());
        else if constexpr (!std::is_same_v<Value, rjson::v3::detail::null>)
            report_error("radius"sv, value);
    });

    getenv("aspect"sv).visit([&circle, report_error]<typename Value>(const Value& value) {
        if constexpr (std::is_same_v<Value, rjson::v3::detail::number>)
            circle.aspect(value.template to<Aspect>());
        else if constexpr (!std::is_same_v<Value, rjson::v3::detail::null>)
            report_error("aspect"sv, value);
    });

    getenv("rotation"sv).visit([&circle, report_error]<typename Value>(const Value& value) {
        if constexpr (std::is_same_v<Value, rjson::v3::detail::number>)
            circle.rotation(value.template to<Rotation>());
        else if constexpr (!std::is_same_v<Value, rjson::v3::detail::null>)
            report_error("rotation"sv, value);
    });

    getenv("fill"sv).visit([&circle, report_error]<typename Value>(const Value& value) {
        if constexpr (std::is_same_v<Value, rjson::v3::detail::string>)
            circle.fill(acmacs::color::Modifier{value.template to<std::string_view>()});
        else if constexpr (!std::is_same_v<Value, rjson::v3::detail::null>)
            report_error("fill"sv, value);
    });

    getenv("outline"sv).visit([&circle, report_error]<typename Value>(const Value& value) {
        if constexpr (std::is_same_v<Value, rjson::v3::detail::string>)
            circle.outline(acmacs::color::Modifier{value.template to<std::string_view>()});
        else if constexpr (!std::is_same_v<Value, rjson::v3::detail::null>)
            report_error("outline"sv, value);
    });

    getenv("outline_width"sv).visit([&circle, report_error]<typename Value>(const Value& value) {
        if constexpr (std::is_same_v<Value, rjson::v3::detail::number>)
            circle.outline_width(value.template to<Pixels>());
        else if constexpr (!std::is_same_v<Value, rjson::v3::detail::null>)
            report_error("outline_width"sv, value);
    });

    return true;

} // acmacs::mapi::v1::Settings::apply_circle

// ----------------------------------------------------------------------

  // # {"N": "path", "points": [<point>, ...], "close": true, "width": 1, "outline": "red", "fill": "transparent",
  // #  "arrows": [{"at": <point-index>, "from": <point-index>, "outline": "magenta", "width": 10, "fill": "magenta"}]},

bool acmacs::mapi::v1::Settings::apply_path()
{
    try {
        throw std::exception{};
        return true;
    }
    catch (std::exception&) {
        throw acmacs::mapi::unrecognized{fmt::format("unrecognized {}", getenv_toplevel())};
    }

} // acmacs::mapi::v1::Settings::apply_path

// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
