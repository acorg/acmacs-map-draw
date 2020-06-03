#include "acmacs-base/rjson-v3-helper.hh"
#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/select-filter.hh"

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_vaccine()
{
    using namespace std::string_view_literals;

    if (const auto name = rjson::v3::read_string(getenv("name"sv)); name.has_value()) {
        auto antigens = chart_draw().chart().antigens();
        auto indexes = antigens->all_indexes();
        acmacs::map_draw::select::filter::name_in(*antigens, indexes, *name);
        if (!indexes.empty()) {
            if (const auto passage = rjson::v3::read_string(getenv("passage"sv)); passage.has_value()) {
                if (*passage == "egg"sv)
                    antigens->filter_egg(indexes, acmacs::chart::reassortant_as_egg::no);
                else if (*passage == "cell"sv)
                    antigens->filter_cell(indexes);
                else if (*passage == "reassortant"sv)
                    antigens->filter_reassortant(indexes);
                else
                    AD_WARNING("Unrecognized \"N\": \"vaccine\" passage specification (ignored): {}", *passage);
            }
            if (!indexes.empty()) {
                if (const auto vaccine_type = rjson::v3::read_string(getenv("vaccine_type"sv)); vaccine_type.has_value()) {
                    // AD_DEBUG("vaccine \"{}\": {}", *name, indexes);
                    chart_draw().vaccines().emplace_back(*vaccine_type, indexes);
                }
                else
                    AD_WARNING("\"N\": \"vaccine\" provides no \"vaccine_type\" (ignored): {}", getenv_toplevel());
            }
        }
    }
    else
        AD_WARNING("\"N\": \"vaccine\" provides no \"name\" (ignored): {}", getenv_toplevel());

    return true;

} // acmacs::mapi::v1::Settings::apply_vaccine

// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
