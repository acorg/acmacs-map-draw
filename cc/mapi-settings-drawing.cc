#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/map-elements-v2.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_circle()
{
    using namespace std::string_view_literals;

    const auto report_error = [](std::string_view key, const auto& value) { throw acmacs::mapi::unrecognized{fmt::format("unrecognized \"circle\" \"{}\": {}", key, value)}; };

    auto& circle = chart_draw().map_elements().add<map_elements::v2::Circle>();

    //     {"N": "circle", "center": {"l": [0, 0]}, "radius": 0.5, "aspect": 2.0, "rotation": 45, "fill": "#80FFA500", "outline": "blue", "outline_width": 3},

    getenv("radius"sv).visit([&circle, report_error]<typename Value>(const Value& value) {
        if constexpr (std::is_same_v<Value, rjson::v3::detail::number>)
            circle.radius(value.template to<Scaled>());
        else if constexpr (!std::is_same_v<Value, rjson::v3::detail::null>)
            report_error("radius"sv, value);
    });

    return true;

} // acmacs::mapi::v1::Settings::apply_circle

// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
