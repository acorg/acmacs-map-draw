#include "acmacs-base/rjson-v3-helper.hh"
#include "acmacs-chart-2/serum-circle.hh"
#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/select-filter.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

static void report_circles(fmt::memory_buffer& report, const acmacs::chart::Antigens& antigens, const acmacs::chart::PointIndexList& antigen_indexes, const acmacs::chart::SerumCircle& empirical,
                           const acmacs::chart::SerumCircle& theoretical);


// ----------------------------------------------------------------------

//  "?homologous_titer": "1280",

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
        auto serum = sera->at(serum_index);
        fmt::format_to(report, "{:{}c}SR {} {} {}\n", ' ', indent, serum_index, serum->full_name(), serum->passage_type(acmacs::chart::reassortant_as_egg::no));
        if (!layout->point_has_coordinates(serum_index + antigens->size())) {
            fmt::format_to(report, "{:{}c}  *** serum is disconnected\n", ' ', indent);
        }
        else if (const auto antigen_indexes = select_antigens_for_serum_circle(serum_index, antigen_selector); !antigen_indexes.empty()) {
            const auto column_basis = chart.column_basis(serum_index, chart_draw().projection_no());
            acmacs::chart::SerumCircle empirical, theoretical;
            if (forced_homologous_titer.has_value()) {
                empirical = acmacs::chart::serum_circle_empirical(antigen_indexes, *forced_homologous_titer, serum_index, *layout, column_basis, *titers, fold, verb);
                theoretical = acmacs::chart::serum_circle_theoretical(antigen_indexes, *forced_homologous_titer, serum_index, column_basis, fold);
            }
            else {
                empirical = acmacs::chart::serum_circle_empirical(antigen_indexes, serum_index, *layout, column_basis, *titers, fold, verb);
                theoretical = acmacs::chart::serum_circle_theoretical(antigen_indexes, serum_index, column_basis, *titers, fold);
            }
            report_circles(report, *antigens, antigen_indexes, empirical, theoretical);

            const auto serum_passage = serum->passage_type(acmacs::chart::reassortant_as_egg::no);
            std::optional<size_t> mark_antigen;
            bool mark_serum{false};
            if (empirical.valid()) {
                make_circle(chart_draw().serum_circle(serum_index, Scaled{empirical.radius()}), serum_passage, getenv("empirical"sv));
                mark_antigen = empirical.per_antigen().front().antigen_no;
                mark_serum = true;
            }
            if (theoretical.valid()) {
                make_circle(chart_draw().serum_circle(serum_index, Scaled{theoretical.radius()}), serum_passage, getenv("theoretical"sv));
                if (!mark_antigen.has_value())
                    mark_antigen = theoretical.per_antigen().front().antigen_no;
                mark_serum = true;
            }
            if (!empirical.valid() && !theoretical.valid()) {
                if (const auto& fallback = getenv("fallback"sv); !fallback.is_null() && rjson::v3::read_bool(fallback["show"sv], true)) {
                    make_circle(chart_draw().serum_circle(serum_index, rjson::v3::read_number(fallback["radius"sv], Scaled{3.0})), serum_passage, fallback);
                    mark_serum = true;
                }
            }

            // mark antigen
            if (const auto& antigen_style = getenv("mark_antigen"sv); mark_antigen.has_value() && !antigen_style.is_null()) {
                const auto style = style_from(antigen_style);
                chart_draw().modify(*mark_antigen, style.style, drawing_order_from(antigen_style));
                const acmacs::chart::PointIndexList indexes{*mark_antigen};
                color_according_to_passage(*antigens, indexes, style);
                if (const auto& label = antigen_style["label"sv]; !label.is_null())
                    add_labels(indexes, 0, label);
            }

            // mark serum
            if (const auto& serum_style = getenv("mark_serum"sv); mark_serum && !serum_style.is_null()) {
                const auto style = style_from(serum_style);
                const acmacs::chart::PointIndexList indexes{serum_index};
                chart_draw().modify_sera(indexes, style.style, drawing_order_from(serum_style));
                color_according_to_passage(*sera, indexes, style);
                if (const auto& label = serum_style["label"sv]; !label.is_null())
                    add_labels(indexes, antigens->size(), label);
            }
        }
        else
            fmt::format_to(report, "{:{}c}  *** no homologous antigens selected (selector: {})\n", ' ', indent, antigen_selector);
        fmt::format_to(report, "\n");
    }
    if (rjson::v3::read_bool(getenv("report"sv), false))
        AD_INFO("Serum circles for {} sera\n{}", serum_indexes.size(), fmt::to_string(report));

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
    return antigen_indexes;

} // acmacs::mapi::v1::Settings::select_antigens_for_serum_circle

// ----------------------------------------------------------------------

static void report_circles(fmt::memory_buffer& report, const acmacs::chart::Antigens& antigens, const acmacs::chart::PointIndexList& antigen_indexes, const acmacs::chart::SerumCircle& empirical,
                           const acmacs::chart::SerumCircle& theoretical)
{
    const auto find_data = [](const acmacs::chart::SerumCircle& data, size_t antigen_index) -> const acmacs::chart::detail::SerumCirclePerAntigen& {
        if (const auto found = find_if(std::begin(data.per_antigen()), std::end(data.per_antigen()), [antigen_index](const auto& en) { return en.antigen_no == antigen_index; });
            found != std::end(data.per_antigen())) {
            return *found;
        }
        else {
            AD_ERROR("per_antigen: {}  looking for antigen {}", data.per_antigen().size(), antigen_index);
            for (const auto& en : data.per_antigen())
                AD_ERROR("AG {} titter:{}", en.antigen_no, en.titer);
            throw std::runtime_error{"internal error in report_circles..find_data"};
        }
    };

    fmt::format_to(report, "     empir   theor   titer\n");
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
        fmt::format_to(report, "    {}  {}  {:>6s}   AG {:4d} {:40s}", empirical_radius, theoretical_radius, theoretical_data.titer, antigen_index, antigens[antigen_index]->full_name(),
                       empirical_report);
        if (!empirical_report.empty())
            fmt::format_to(report, " -- {}", empirical_report);
        else if (!theoretical_report.empty())
            fmt::format_to(report, " -- {}", theoretical_report);
        fmt::format_to(report, "\n");
    }
    std::string empirical_radius(6, ' '), theoretical_radius(6, ' ');
    if (empirical.valid())
        empirical_radius = fmt::format("{:.4f}", empirical.radius());
    if (theoretical.valid())
        theoretical_radius = fmt::format("{:.4f}", theoretical.radius());
    fmt::format_to(report, "  > {}  {}\n", empirical_radius, theoretical_radius);
}

// ----------------------------------------------------------------------

void acmacs::mapi::v1::Settings::make_circle(map_elements::v1::SerumCircle& circle, std::string_view serum_passage, const rjson::v3::value& plot)
{
    using namespace std::string_view_literals;

    if (rjson::v3::read_bool(plot["show"sv], true)) {
        if (const auto outline_dash = rjson::v3::read_string(plot["outline_dash"sv], ""sv); outline_dash == "dash1"sv)
            circle.outline_dash1();
        else if (outline_dash == "dash2"sv)
            circle.outline_dash2();
        else if (outline_dash == "dash3"sv)
            circle.outline_dash3();
        else
            circle.outline_no_dash();

        const auto get_color = [serum_passage, this](const rjson::v3::value& source, Color dflt) -> Color {
            return std::visit(
                [serum_passage, dflt]<typename Val>(const Val& value) -> Color {
                    std::optional<acmacs::color::Modifier> color;
                    if constexpr (std::is_same_v<Val, acmacs::color::Modifier>)
                        color = value;
                    else if (serum_passage == "egg"sv) // Val = passage_color_t
                        color = value.egg;
                    else if (serum_passage == "cell"sv)
                        color = value.cell;
                    else if (serum_passage == "reassortant"sv)
                        color = value.reassortant;
                    if (color.has_value())
                        return *color;
                    else
                        return dflt;
                },
                color(source, dflt));
        };

        const auto outline{get_color(plot["outline"sv], PINK)};
        const auto outline_width{rjson::v3::read_number(plot["outline_width"sv], Pixels{1.0})};
        circle.outline(outline, outline_width);
        circle.fill(get_color(plot["fill"sv], TRANSPARENT));

        if (const auto& angles = plot["angles"sv]; !angles.is_null())
            circle.angles(rjson::v3::read_number<Rotation>(angles[0], Rotation{0}), rjson::v3::read_number<Rotation>(angles[1], Rotation{0}));
        if (const auto& radius_line = plot["radius_line"sv]; !radius_line.is_null()) {
            if (const auto dash = rjson::v3::read_string(radius_line["dash"sv], ""sv); dash == "dash1"sv)
                circle.radius_line_dash1();
            else if (dash == "dash2"sv)
                circle.radius_line_dash2();
            else if (dash == "dash3"sv)
                circle.radius_line_dash3();
            else
                circle.radius_line_no_dash();
            circle.radius_line(get_color(radius_line["color"sv], outline), rjson::v3::read_number(radius_line["line_width"sv], outline_width));
        }

        if (!plot["angle_degrees"sv].is_null() || !plot["radius_line_dash"sv].is_null() || !plot["radius_line_color"sv].is_null())
            AD_WARNING("\"angle_degrees\", \"radius_line_dash\", \"radius_line_color\" are deprecated and not supported");
    }

} // acmacs::mapi::v1::Settings::make_circle

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
