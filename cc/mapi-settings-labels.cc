#include "acmacs-base/rjson-v3-helper.hh"
#include "acmacs-chart-2/name-format.hh"
#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

void acmacs::mapi::v1::Settings::add_labels(const acmacs::chart::PointIndexList& indexes, size_t index_base, const rjson::v3::value& label_data)
{
    using namespace std::string_view_literals;

    if (rjson::v3::read_bool(label_data["show"sv], true)) {
        for (auto index : indexes)
            add_label(index, index_base, label_data);
    }
    else {// avoid slow label generating if label is not shown
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

    // label.show(rjson::v3::read_bool(label_data["show"sv], true)); // show by default

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

    if (const auto offset = rjson::v3::read_point_coordinates(label_data["offset"sv]); offset.has_value())
        label.offset(*offset);

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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
