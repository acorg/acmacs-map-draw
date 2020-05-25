#include "acmacs-base/rjson-v3-helper.hh"
#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/map-elements-v1.hh"

// ----------------------------------------------------------------------

map_elements::v1::LegendPointLabel& acmacs::mapi::v1::Settings::legend()
{
    return chart_draw().map_elements().find_or_add<map_elements::v1::LegendPointLabel>("legend-point-label");

} // acmacs::mapi::v1::Settings::legend

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_legend()
{
    using namespace std::string_view_literals;

    if (rjson::v3::read_bool(getenv("show"sv), true)) { // show by default
        if (const auto legend_type = rjson::v3::read_string(getenv("type"sv), ""sv); legend_type == "continent-map"sv || legend_type == "continent_map"sv) {
            add_legend_continent_map();
        }
        else {
            auto& legend_element = legend();
            legend_element.offset(rjson::v3::read_point_coordinates(getenv("offset"sv), acmacs::PointCoordinates{-10, -10}));
            legend_element.label_size(rjson::v3::read_number(getenv("label_size"sv), Pixels{12}));
            legend_element.point_size(rjson::v3::read_number(getenv("point_size"sv), Pixels{8}));

            getenv("title"sv).visit([&legend_element]<typename Val>(const Val& title) {
                auto insertion_point{legend_element.lines().begin()};
                if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
                    for (const auto& line : title)
                        insertion_point = std::next(legend_element.lines().emplace(insertion_point, line.template to<std::string_view>()));
                }
                else if constexpr (std::is_same_v<Val, rjson::v3::detail::string>)
                    legend_element.lines().insert(insertion_point, title.template to<std::string_view>());
                else if constexpr (!std::is_same_v<Val, rjson::v3::detail::null>)
                    throw error{fmt::format("unrecognized: {} (expected array of strings or a string)", title)};
            });

            getenv("lines"sv).visit([&legend_element]<typename Val>(const Val& lines) {
                if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
                    for (const auto& line : lines) {
                        legend_element.lines().emplace_back(rjson::v3::read_color(line["outline"sv], TRANSPARENT), rjson::v3::read_color(line["fill"sv], TRANSPARENT),
                                                            rjson::v3::read_string(line["text"sv], "\"text\""sv));
                    }
                }
                else if constexpr (!std::is_same_v<Val, rjson::v3::detail::null>)
                    throw error{fmt::format("unrecognized: {} (expected array of objects)", lines)};
            });
        }
    }
    else {
        AD_DEBUG("remove_legend");
        chart_draw().remove_legend();
    }

    return true;

} // acmacs::mapi::v1::Settings::apply_legend

// ----------------------------------------------------------------------

void acmacs::mapi::v1::Settings::add_legend(const acmacs::chart::PointIndexList& indexes, const point_style_t& style, const rjson::v3::value& legend_data)
{
    using namespace std::string_view_literals;

    const auto label{fmt::format(rjson::v3::get_or(legend_data["label"sv], "use \"label\" in \"legend\""sv),
                                 fmt::arg("count", indexes.size()))};
    if (rjson::v3::read_bool(legend_data["replace"sv], false))
        legend().remove_line(label);

    add_legend(indexes, style, label, legend_data);

} // acmacs::mapi::v1::Settings::add_legend

// ----------------------------------------------------------------------

void acmacs::mapi::v1::Settings::add_legend(const acmacs::chart::PointIndexList& indexes, const point_style_t& style, std::string_view label, const rjson::v3::value& legend_data)
{
    using namespace std::string_view_literals;
    if (rjson::v3::read_bool(legend_data["show"sv], true) && !indexes->empty()) { // show is true by default
        legend().add_line(acmacs::color::Modifier{style.style.outline()}, acmacs::color::Modifier{style.style.fill()}, label);
    }

} // acmacs::mapi::v1::Settings::add_legend

// ----------------------------------------------------------------------

void acmacs::mapi::v1::Settings::add_legend_continent_map()
{
    using namespace std::string_view_literals;
    if (const auto offset = rjson::v3::read_point_coordinates(getenv("offset"sv)); offset.has_value())
        chart_draw().continent_map(*offset, rjson::v3::read_number(getenv("size"sv), Pixels{100.0}));
    else
        chart_draw().continent_map();

} // acmacs::mapi::v1::Settings::add_legend_continent_map

// ----------------------------------------------------------------------

map_elements::v1::Title& acmacs::mapi::v1::Settings::title()
{
    return chart_draw().map_elements().find_or_add<map_elements::v1::Title>("title");

} // acmacs::mapi::v1::Settings::title

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_title()
{
    return true;

} // acmacs::mapi::v1::Settings::apply_title

// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
