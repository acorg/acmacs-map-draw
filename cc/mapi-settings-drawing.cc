#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/map-elements-v2.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_circle()
{
    using namespace std::string_view_literals;

    const auto report_error = [](std::string_view key, const auto& value) { throw acmacs::mapi::unrecognized{fmt::format("unrecognized \"circle\" \"{}\": {}", key, value)}; };

    auto& circle = chart_draw().map_elements().add<map_elements::v2::Circle>();

    // "center": {"l": [0, 0]}

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



// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
