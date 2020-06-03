#include "acmacs-base/read-file.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/rjson-v3-helper.hh"
#include "acmacs-base/string-compare.hh"
#include "acmacs-base/quicklook.hh"
#include "seqdb-3/compare.hh"
#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

// {"N": "compare-sequences", "groups": [{"select": <Select Antigens>, "name": "group-1"} ...], "html": "filename.html (filename.data.js is generated as well)", "json": "filename.json", "report": true, "open": true},

struct group_t
{
    std::string name;
    acmacs::chart::PointIndexList antigens;

    group_t(std::string_view a_name, acmacs::chart::PointIndexList&& a_antigens) : name{a_name}, antigens{std::move(a_antigens)} {}
};

// ----------------------------------------------------------------------

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

    const auto& matched_seqdb = chart_draw().chart(0).match_seqdb();
    const auto nuc_aa{acmacs::seqdb::compare::aa};
    acmacs::seqdb::subsets_to_compare_t subsets_to_compare{nuc_aa};
    for (const auto& group_desc : groups) {
        auto& subset = subsets_to_compare.subsets.emplace_back(group_desc.name).subset;
        for (const auto ind : group_desc.antigens) {
            if (matched_seqdb[ind])
                subset.append(matched_seqdb[ind]);
        }
    }
    subsets_to_compare.make_counters();

    if (std::string json_filename{rjson::v3::read_string(getenv("json"sv), ""sv)}; !json_filename.empty()) {
        if (json_filename != "-"sv && !acmacs::string::endswith(json_filename, ".json"sv))
            json_filename.append(".json"sv);
        acmacs::file::write(json_filename, subsets_to_compare.format_json(2));
    }

    if (std::string html_filename{rjson::v3::read_string(getenv("html"sv), ""sv)}; !html_filename.empty()) {
        if (!acmacs::string::endswith(html_filename, ".html"sv))
            html_filename.append(".html"sv);
        acmacs::seqdb::compare_sequences_generate_html(html_filename, subsets_to_compare);
        if (rjson::v3::read_bool(getenv("open"sv), false))
            acmacs::open(html_filename);
    }

    if (rjson::v3::read_bool(getenv("report"sv), false))
        fmt::print("{}\n", subsets_to_compare.format_summary());

    return true;

} // acmacs::mapi::v1::Settings::apply_compare_sequences

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
