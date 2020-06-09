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

void acmacs::mapi::v1::Settings::add_legend(const acmacs::chart::PointIndexList& indexes, const acmacs::PointStyleModified& style, const rjson::v3::value& legend_data)
{
    using namespace std::string_view_literals;

    const auto label{fmt::format(rjson::v3::get_or(legend_data["label"sv], "use \"label\" in \"legend\""sv),
                                 fmt::arg("count", indexes.size()))};
    if (rjson::v3::read_bool(legend_data["replace"sv], false))
        legend().remove_line(label);

    add_legend(indexes, style, label, legend_data);

} // acmacs::mapi::v1::Settings::add_legend

// ----------------------------------------------------------------------

void acmacs::mapi::v1::Settings::add_legend(const acmacs::chart::PointIndexList& indexes, const acmacs::PointStyleModified& style, std::string_view label, const rjson::v3::value& legend_data)
{
    using namespace std::string_view_literals;
    if (rjson::v3::read_bool(legend_data["show"sv], true) && !indexes->empty()) { // show is true by default
        legend().add_line(acmacs::color::Modifier{style.outline()}, acmacs::color::Modifier{style.fill()}, label);
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
    using namespace std::string_view_literals;

    auto& title_element = title();
    rjson::v3::call_if_not_null<bool>(getenv("show"sv), [&title_element](bool val) { title_element.show(val); });
    rjson::v3::call_if_not_null<Pixels>(getenv("padding"sv), [&title_element](Pixels val) { title_element.padding(val); });
    rjson::v3::call_if_not_null<double>(getenv("interline"sv), [&title_element](double val) { title_element.interline(val); });
    rjson::v3::call_if_not_null<Pixels>(getenv("text_size"sv), [&title_element](Pixels val) { title_element.text_size(val); });
    rjson::v3::call_if_not_null<Pixels>(getenv("border_width"sv), [&title_element](Pixels val) { title_element.border_width(val); });
    rjson::v3::call_if_not_null<acmacs::PointCoordinates>(getenv("offset"sv), [&title_element](const acmacs::PointCoordinates& val) { title_element.offset(val); });
    rjson::v3::call_if_not_null<acmacs::color::Modifier>(getenv("text_color"sv), [&title_element](const acmacs::color::Modifier& val) { title_element.text_color(val); });
    rjson::v3::call_if_not_null<acmacs::color::Modifier>(getenv("background"sv), [&title_element](const acmacs::color::Modifier& val) { title_element.background(val); });
    rjson::v3::call_if_not_null<acmacs::color::Modifier>(getenv("border_color"sv), [&title_element](const acmacs::color::Modifier& val) { title_element.border_color(val); });
    rjson::v3::call_if_not_null<std::string_view>(getenv("font_weight"sv), [&title_element](std::string_view val) { title_element.weight(val); });
    rjson::v3::call_if_not_null<std::string_view>(getenv("font_slant"sv), [&title_element](std::string_view val) { title_element.slant(val); });
    rjson::v3::call_if_not_null<std::string_view>(getenv("font_family"sv), [&title_element](std::string_view val) { title_element.font_family(val); });

    if (rjson::v3::read_bool(getenv("remove-lines"sv), false))
        title_element.remove_all_lines();

    getenv("lines"sv).visit([&title_element, this]<typename Val>(const Val& lines) {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
            const auto& chart_access = chart_draw().chart(0);
            for (const auto& line : lines)
                title_element.add_line(substitute_chart_metadata(line.template to<std::string_view>(), chart_access));
        }
        else if constexpr (!std::is_same_v<Val, rjson::v3::detail::null>)
            throw error{fmt::format("unrecognized: {} (expected array of strings)", lines)};
    });

    return true;

} // acmacs::mapi::v1::Settings::apply_title

// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
