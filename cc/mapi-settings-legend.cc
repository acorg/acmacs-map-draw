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
      // "title": "<format>" -- ["<format>", ...]
      // "lines": [{"display_name": "163-del", "outline": "black", "fill": "red"}]

            auto& legend_element = legend();
            legend_element.offset(rjson::v3::read_point_coordinates(getenv("offset"sv), acmacs::PointCoordinates{-10, -10}));
            legend_element.label_size(rjson::v3::read_number(getenv("label_size"sv), Pixels{12}));
            legend_element.point_size(rjson::v3::read_number(getenv("point_size"sv), Pixels{8}));
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
        legend().add_line(style.style.outline(), style.style.fill(), label);
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
