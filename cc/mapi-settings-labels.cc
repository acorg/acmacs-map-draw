#include "acmacs-base/rjson-v3-helper.hh"
#include "acmacs-chart-2/name-format.hh"
#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

void acmacs::mapi::v1::Settings::add_labels(const acmacs::chart::PointIndexList& indexes, size_t index_base, const rjson::v3::value& label_data)
{
    using namespace std::string_view_literals;

    const bool show = label_data["show"].visit([]<typename Val>(const Val& value) -> bool {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::null>)
            return true; // show by default
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::boolean>)
            return value.template to<bool>();
        else
            throw acmacs::mapi::unrecognized{fmt::format("unrecognized \"show\" value: {}", value)};
    });

    if (show) { // const auto& show = aLabelData["show"]; show.is_null() || show.to<bool>()) { // true by default
        for (auto index : indexes)
            add_label(index, index_base, label_data);
    }
    else {
        for (auto index : indexes)
            chart_draw().remove_label(index + index_base);
    }

} // acmacs::mapi::v1::Settings::add_labels

// ----------------------------------------------------------------------

void acmacs::mapi::v1::Settings::add_label(size_t index, size_t index_base, const rjson::v3::value& label_data)
{
    using namespace std::string_view_literals;

    const auto point_index = index + index_base;
    auto& label = chart_draw().add_label(point_index);

    for (const auto key : {"name_type"sv, "name-type"sv, "display_name"sv, "display-name"sv}) {
        if (!label_data[key].is_null())
            AD_ERROR("\"{}\" is not supported, use \"format\", run chart-name-format-help to list formats", key);
    }

    std::string pattern{"{abbreviated_name_with_passage_type}"};
    if (const auto name_format = rjson::v3::read_string(label_data["format"sv]); name_format.has_value())
        pattern = *name_format;
    if (index_base == 0)
        label.display_name(acmacs::chart::format_antigen(pattern, chart_draw().chart(), index));
    else
        label.display_name(acmacs::chart::format_serum(pattern, chart_draw().chart(), index));

    label_data["offset"sv].visit([&label]<typename Val>(const Val& value) {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::array>)
            label.offset({value[0].template to<double>(), value[1].template to<double>()});
        else if constexpr (!std::is_same_v<Val, rjson::v3::detail::null>)
            throw acmacs::mapi::unrecognized{fmt::format("unrecognized \"offset\" value: {}", value)};
    });

    if (const auto color = rjson::v3::read_color(label_data["color"sv]); color.has_value())
        label.color(*color);
    if (const auto size = rjson::v3::read_number<Pixels>(label_data["size"sv]); size.has_value())
        label.size(*size);
    if (const auto weight = rjson::v3::read_string(label_data["weight"sv]); weight.has_value())
        label.weight(*weight);
    if (const auto slant = rjson::v3::read_string(label_data["slant"sv]); slant.has_value())
        label.slant(*slant);
    if (const auto font_family = rjson::v3::read_string(label_data["font_family"sv]); font_family.has_value())
        label.font_family(*font_family);

} // acmacs::mapi::v1::Settings::add_label

// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
