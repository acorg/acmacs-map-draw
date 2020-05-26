#include "acmacs-base/rjson-v3-helper.hh"
#include "acmacs-chart-2/serum-circle.hh"
#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/select-filter.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

//  "?homologous_titer": "1280",
//  "empirical":   {"fill": "#C08080FF", "outline": "#4040FF", "outline_width": 2, "?outline_dash": "dash2", "?angles": [0, 30], "?radius_line": {"dash": "dash2", "color": "red", "line_width": 1, "show": true}},
//  "theoretical": {"fill": "#C08080FF", "outline": "#0000C0", "outline_width": 2, "?outline_dash": "dash2", "?angles": [0, 30], "?radius_line": {"dash": "dash2", "color": "red", "line_width": 1, "show": true}},
//  "fallback":    {"fill": "#C08080FF", "outline": "#0000C0", "outline_width": 2, "outline_dash": "dash3",  "?angles": [0, 30], "?radius_line": {"dash": "dash2", "color": "red", "line_width": 1, "show": true}},
//  "mark_serum":   {"fill": "lightblue", "outline": "black", "order": "raise", "label": {"format": "{full_name}", "offset": [0, 1.2], "color": "black", "size": 12}},
//  "mark_antigen": {"fill": "lightblue", "outline": "black", "order": "raise", "label": {"name_type": "{full_name}", "offset": [0, 1.2], "color": "black", "size": 12}}},

bool acmacs::mapi::v1::Settings::apply_serum_circles()
{
    using namespace std::string_view_literals;

    auto& chart = chart_draw().chart();
    auto antigens = chart.antigens();
    auto sera = chart.sera();
    auto titers = chart.titers();
    auto layout = chart_draw().layout();

    const auto serum_indexes = select_sera(getenv("serum"sv, "sera"sv));
    const auto fold = rjson::v3::read_number(getenv("fold"sv), 2.0);
    const auto forced_homologous_titer = rjson::v3::read_string(getenv("homologous_titer"sv));
    const auto print_report = rjson::v3::read_bool(getenv("report"sv), false);
    const auto verb = verbose_from(rjson::v3::read_bool(getenv("verbose"sv), false));
    const auto& antigen_selector{getenv("antigen"sv, "antigens"sv)};
    for (auto serum_index : serum_indexes) {
        fmt::memory_buffer report;
        if (!layout->point_has_coordinates(serum_index + antigens->size())) {
            fmt::format_to(report, "  serum is disconnected");
        }
        else if (const auto antigen_indexes = select_antigens_for_serum_circle(serum_index, antigen_selector); !antigen_indexes.empty()) {
            const auto column_basis = chart.column_basis(serum_index, chart_draw().projection_no());
            acmacs::chart::SerumCircle empirical, theoretical;
            if (forced_homologous_titer.has_value()) {
                empirical = acmacs::chart::serum_circle_empirical(antigen_indexes, *forced_homologous_titer, serum_index, *layout, column_basis, *titers, fold, verb);
                theoretical = acmacs::chart::serum_circle_theoretical(*forced_homologous_titer, serum_index, column_basis, fold);
            }
            else {
                empirical = acmacs::chart::serum_circle_empirical(antigen_indexes, serum_index, *layout, column_basis, *titers, fold, verb);
                theoretical = acmacs::chart::serum_circle_theoretical(antigen_indexes, serum_index, column_basis, *titers, fold);
            }

            for (size_t ag_no{0}; ag_no < antigen_indexes.size(); ++ag_no) {
                fmt::format_to(report, "  AG {} {}\n", antigen_indexes[ag_no], antigens->at(antigen_indexes[ag_no])->full_name());
                if (const auto& empirical_data = empirical.per_antigen()[ag_no]; empirical_data.valid())
                    fmt::format_to(report, "  {}  empir: {:.4f}  titer: {}\n", empirical_data.antigen_no, *empirical_data.radius, empirical_data.titer);
                else
                    fmt::format_to(report, "  {}  NO empirical: {}  titer: {}\n", empirical_data.antigen_no, empirical_data.report_reason(), empirical_data.titer);
                if (const auto& theoretical_data = theoretical.per_antigen()[ag_no]; theoretical_data.valid())
                    fmt::format_to(report, "  {}  theor: {:.4f}  titer: {}\n", theoretical_data.antigen_no, *theoretical_data.radius, theoretical_data.titer);
                else
                    fmt::format_to(report, "  {}  NO theoretical: {}  titer: {}\n", theoretical_data.antigen_no, theoretical_data.report_reason(), theoretical_data.titer);
            }

            // mark antigen
            // mark serum
        }
        else
            fmt::format_to(report, "  no homologous antigens selected (selector: {})", antigen_selector);

        if (print_report)
            AD_INFO("serum circle SR {} {}\n{}", serum_index, sera->at(serum_index)->full_name(), fmt::to_string(report));
    }

    // const auto verbose = rjson::get_or(args(), "report", false);
    // for (auto serum_index : serum_indexes) {
    //     const auto antigen_indices = select_mark_antigens(aChartDraw, serum_index, acmacs::chart::find_homologous::all, verbose);
    //     const double fold = rjson::get_or(args(), "fold", 2.0);
    //     if (const auto& homologous_titer = args()["homologous_titer"]; !homologous_titer.is_null())
    //         make_serum_circle(aChartDraw, serum_index, antigen_indices, acmacs::chart::Titer(homologous_titer.to<std::string_view>()), args()["empirical"], args()["theoretical"],
    //         args()["fallback"], fold, verbose);
    //     else
    //         make_serum_circle(aChartDraw, serum_index, antigen_indices, args()["empirical"], args()["theoretical"], args()["fallback"], fold, verbose);
    // }

    return true;

} // acmacs::mapi::v1::Settings::apply_serum_circles

// ----------------------------------------------------------------------

acmacs::chart::PointIndexList acmacs::mapi::v1::Settings::select_antigens_for_serum_circle(size_t serum_index, const rjson::v3::value& antigen_selector)
{
    acmacs::chart::PointIndexList antigen_indexes;
    auto& chart = chart_draw().chart();
    auto serum = chart.sera()->at(serum_index);
    if (!antigen_selector.is_null()) {
        antigen_indexes = select_antigens(antigen_selector);
        acmacs::map_draw::select::filter::name_in(*chart.antigens(), antigen_indexes, serum->name());
    }
    else {
        chart.set_homologous(acmacs::chart::find_homologous::relaxed);
        antigen_indexes = serum->homologous_antigens();
    }

    // if (antigen_indexes.empty())
    //     throw acmacs::mapi::unrecognized{fmt::format("no homologous antigens seelected for SR {} {} (antigen selector: {})", serum_index, chart_draw().chart().sera()->at(serum_index)->full_name(), antigen_selector)};
    return antigen_indexes;

} // acmacs::mapi::v1::Settings::select_antigens_for_serum_circle

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
