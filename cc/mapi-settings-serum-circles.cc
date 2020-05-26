#include "acmacs-base/rjson-v3-helper.hh"
#include "acmacs-chart-2/serum-circle.hh"
#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/select-filter.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

inline static void report_circles(fmt::memory_buffer& report, size_t indent, const acmacs::chart::Antigens& antigens, const acmacs::chart::PointIndexList& antigen_indexes, const acmacs::chart::SerumCircle& empirical,
                                  const acmacs::chart::SerumCircle& theoretical)
{
    const auto find_data = [](const acmacs::chart::SerumCircle& data, size_t antigen_index) -> const acmacs::chart::detail::SerumCirclePerAntigen& {
        if (const auto found = find_if(std::begin(data.per_antigen()), std::end(data.per_antigen()), [antigen_index](const auto& en) { return en.antigen_no == antigen_index; }); found != std::end(data.per_antigen()))
            return *found;
        else
            throw std::runtime_error{"internal error in report_circles..find_data"};
    };

    fmt::format_to(report, "{:{}c} empir   theor   titer\n", ' ', indent);
    for (const auto antigen_index : antigen_indexes) {
        const auto& empirical_data = find_data(empirical, antigen_index);
        const auto& theoretical_data = find_data(theoretical, antigen_index);
        std::string empirical_radius(6, ' '), theoretical_radius(6, ' '), empirical_report, theoretical_report;
        if (empirical_data.valid())
            empirical_radius = fmt::format("{:.4f}", *empirical_data.radius);
        else
            empirical_report.assign(empirical_data.report_reason());
        if (theoretical_data.valid())
            theoretical_radius = fmt::format("{:.4f}", *theoretical_data.radius);
        else
            theoretical_report.assign(theoretical_data.report_reason());
        fmt::format_to(report, "{:{}c}{}  {}  {:>6s}   AG {:4d} {:40s}", ' ', indent, empirical_radius, theoretical_radius, theoretical_data.titer, antigen_index, antigens[antigen_index]->full_name(), empirical_report);
        if (!empirical_report.empty())
            fmt::format_to(report, " -- {}", empirical_report);
        else if (!theoretical_report.empty())
            fmt::format_to(report, " -- {}", theoretical_report);
        fmt::format_to(report, "\n");
    }
}

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
    const auto verb = verbose_from(rjson::v3::read_bool(getenv("verbose"sv), false));
    const auto& antigen_selector{getenv("antigen"sv, "antigens"sv)};
    fmt::memory_buffer report;
    const size_t indent{2};
    for (auto serum_index : serum_indexes) {
        fmt::format_to(report, "{:{}c}SR {} {}\n", ' ', indent, serum_index, sera->at(serum_index)->full_name());
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

            report_circles(report, indent * 2, *antigens, antigen_indexes, empirical, theoretical);

            // mark antigen
            // mark serum
        }
        else
            fmt::format_to(report, "{:{}c}*** no homologous antigens selected (selector: {})\n", ' ', indent, antigen_selector);
        fmt::format_to(report, "\n");
    }
    if (rjson::v3::read_bool(getenv("report"sv), false))
        AD_INFO("Serum circles for {} sera\n{}", serum_indexes.size(), fmt::to_string(report));

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
        chart.set_homologous(acmacs::chart::find_homologous::all);
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
