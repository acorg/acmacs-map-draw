#include "acmacs-base/html.hh"
#include "acmacs-map-draw/mapi-settings.hh"

// ----------------------------------------------------------------------

// {"N": "compare-sequences", "groups": [{"select": <Select Antigens>, "name": "group-1"} ...], "html": "filename.html (filename.data.js is generated as well)", "json": "filename.json", "report": true, "open": true},

struct group_t
{
    std::string name;
    acmacs::chart::PointIndexList antigens;

    group_t(std::string_view a_name, acmacs::chart::PointIndexList&& a_antigens) : name{a_name}, antigens{std::move(a_antigens)} {}
};

bool acmacs::mapi::v1::Settings::apply_compare_sequences()
{
    using namespace std::string_view_literals;

    std::vector<group_t> groups;
    if (const auto& groups_v = getenv("groups"sv); groups_v.is_array()) {
        for (const auto& grp : groups_v.array()) {
            try {
                groups.emplace_back(grp["name"sv].to<std::string_view>(), select_antigens(grp["select"sv]));
            }
            catch (std::exception& err) {
                throw acmacs::mapi::unrecognized{AD_FORMAT("unrecognized \"groups\" entry: {}: {}", grp, err)};
            }
        }
    }
    else
        throw acmacs::mapi::unrecognized{AD_FORMAT("unrecognized or absent \"groups\" clause: {} (array of objects expected)", groups_v)};

    acmacs::html::Generator html;
    html.title("Compare sequences"sv);
    fmt::print("{}\n", html.generate());

    return true;

}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
