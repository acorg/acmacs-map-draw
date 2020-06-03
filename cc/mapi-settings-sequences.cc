#include "acmacs-base/html.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/string-compare.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/rjson-v3-helper.hh"
#include "acmacs-base/color-amino-acid.hh"
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
    const auto json_output = subsets_to_compare.format_json(2);

    if (std::string json_filename{rjson::v3::read_string(getenv("json"sv), ""sv)}; !json_filename.empty()) {
        if (json_filename != "-"sv && !acmacs::string::endswith(json_filename, ".json"sv))
            json_filename.append(".json"sv);
        acmacs::file::write(json_filename, json_output);
    }

    if (const auto html_filename{rjson::v3::read_string(getenv("html"sv), ""sv)}; !html_filename.empty())
        compare_sequences_generate_html(html_filename, json_output);

    if (rjson::v3::read_bool(getenv("report"sv), false)) {
    }


    return true;
}

// ----------------------------------------------------------------------

constexpr static std::string_view sHtmlBody{R"(<h2>Compare sequences</h2>
<div id="compare-sequences" class="compare-sequences">
    <div class="most-frequent-per-group"></div>
    <div class="frequency-per-group"></div>
    <div class="positions-with-diversity"></div>
    <div class="full-sequences"></div>
</div>
)"};

void acmacs::mapi::v1::Settings::compare_sequences_generate_html(std::string_view filename, std::string_view data)
{
    using namespace std::string_view_literals;

    std::string html_filename{filename};
    if (!acmacs::string::endswith(html_filename, ".html"sv))
        html_filename.append(".html"sv);
    const auto prefix{html_filename.substr(0, html_filename.size() - 5)};
    const auto data_filename{fmt::format("{}.data.js", prefix)};
    const auto data_var_name{fmt::format("compare_sequences_{}", ::string::replace(prefix, "/"sv, "_"sv, "-"sv, "_"sv))};
    acmacs::file::write(data_filename, fmt::format("const {} =\n{}", data_var_name, data));

    std::string data_filename_name{data_filename};
    if (const auto pos = std::string_view{data_filename}.find_last_of('/'); pos != std::string_view::npos)
        data_filename_name.erase(0, pos + 1);

    acmacs::html::Generator html;
    html.title("Compare sequences"sv);
    html.add_css(acmacs::amino_acid_nucleotide_color_css());
    html.add_script_link(data_filename_name);
    html.add_to_body(sHtmlBody);
    acmacs::file::write(html_filename, html.generate());

    if (rjson::v3::read_bool(getenv("open"sv), false))
        acmacs::open(html_filename);

} // acmacs::mapi::v1::Settings::compare_sequences_generate_html

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
