#include "acmacs-base/rjson-v3-helper.hh"
#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/select-filter.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

//  "?antigen": {<Select Antigens>},
//  "report": true,
//  "empirical":   {"fill": "#C08080FF", "outline": "#4040FF", "outline_width": 2, "?outline_dash": "dash2", "?angles": [0, 30], "?radius_line": {"dash": "dash2", "color": "red", "line_width": 1, "show": true}},
//  "theoretical": {"fill": "#C08080FF", "outline": "#0000C0", "outline_width": 2, "?outline_dash": "dash2", "?angles": [0, 30], "?radius_line": {"dash": "dash2", "color": "red", "line_width": 1, "show": true}},
//  "fallback":    {"fill": "#C08080FF", "outline": "#0000C0", "outline_width": 2, "outline_dash": "dash3",  "?angles": [0, 30], "?radius_line": {"dash": "dash2", "color": "red", "line_width": 1, "show": true}},
//  "mark_serum":   {"fill": "lightblue", "outline": "black", "order": "raise", "label": {"format": "{full_name}", "offset": [0, 1.2], "color": "black", "size": 12}},
//  "mark_antigen": {"fill": "lightblue", "outline": "black", "order": "raise", "label": {"name_type": "{full_name}", "offset": [0, 1.2], "color": "black", "size": 12}}},

bool acmacs::mapi::v1::Settings::apply_serum_circles()
{
    using namespace std::string_view_literals;

    const auto serum_indexes = select_sera(getenv("serum"sv, "sera"sv));
    const auto fold = rjson::v3::read_number(getenv("fold"sv), 2.0);
    for (auto serum_index : serum_indexes) {
        const auto antigen_indexes = select_antigens_for_serum_circle(serum_index, getenv("antigen"sv, "antigens"sv));
    }

    // const auto verbose = rjson::get_or(args(), "report", false);
    // for (auto serum_index : serum_indexes) {
    //     const auto antigen_indices = select_mark_antigens(aChartDraw, serum_index, acmacs::chart::find_homologous::all, verbose);
    //     const double fold = rjson::get_or(args(), "fold", 2.0);
    //     if (const auto& homologous_titer = args()["homologous_titer"]; !homologous_titer.is_null())
    //         make_serum_circle(aChartDraw, serum_index, antigen_indices, acmacs::chart::Titer(homologous_titer.to<std::string_view>()), args()["empirical"], args()["theoretical"], args()["fallback"], fold, verbose);
    //     else
    //         make_serum_circle(aChartDraw, serum_index, antigen_indices, args()["empirical"], args()["theoretical"], args()["fallback"], fold, verbose);
    // }

    return true;

} // acmacs::mapi::v1::Settings::apply_serum_circles

// ----------------------------------------------------------------------

acmacs::chart::PointIndexList acmacs::mapi::v1::Settings::select_antigens_for_serum_circle(size_t serum_index, const rjson::v3::value& antigen_selector)
{
    acmacs::chart::PointIndexList antigen_indexes;
    if (!antigen_selector.is_null()) {
        antigen_indexes = select_antigens(antigen_selector);
        acmacs::map_draw::select::filter::name_in(*chart_draw().chart().antigens(), antigen_indexes, chart_draw().chart().sera()->at(serum_index)->name());
    }
    else {
    }

    if (antigen_indexes.empty())
        throw acmacs::mapi::unrecognized{fmt::format("no homologous antigens seelected for SR {} {} (antigen selector: {})", serum_index, chart_draw().chart().sera()->at(serum_index)->full_name(), antigen_selector)};
    return antigen_indexes;

} // acmacs::mapi::v1::Settings::select_antigens_for_serum_circle

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
