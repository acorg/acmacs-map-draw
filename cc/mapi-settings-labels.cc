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
            add_label(index + index_base, label_data);
    }
    else {
        for (auto index : indexes)
            chart_draw().remove_label(index + index_base);
    }

} // acmacs::mapi::v1::Settings::add_labels

// ----------------------------------------------------------------------

void acmacs::mapi::v1::Settings::add_label(size_t point_index, const rjson::v3::value& label_data)
{
    auto& label = chart_draw().add_label(point_index);
    label.display_name("JOPA");

} // acmacs::mapi::v1::Settings::add_label

// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
