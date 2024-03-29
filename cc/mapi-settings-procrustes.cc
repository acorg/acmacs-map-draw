#include "acmacs-base/rjson-v3-helper.hh"
#include "acmacs-base/string-compare.hh"
#include "acmacs-base/quicklook.hh"
#include "acmacs-base/timeit.hh"
#include "acmacs-chart-2/procrustes.hh"
#include "acmacs-chart-2/serum-line.hh"
#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/map-elements-v2.hh"
#include "acmacs-map-draw/mapi-procrustes.hh"

constexpr const std::string_view sProcrustesArrowElementKeyword{"procrustes-arrow"};

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_reset()
{
    chart_draw().reset();
    return true;

} // acmacs::mapi::v1::Settings::apply_reset

// ----------------------------------------------------------------------

std::string acmacs::mapi::v1::Settings::substitute_in_filename(std::string_view filename) const
{
    if (filename.empty())
        filename = chart_draw().chart(0).filename(); // can draw just the chart 0 // get_chart(getenv("chart"sv), 0);
    return substitute(filename);

    // const auto& chart_access = chart_draw().chart(0); // can draw just the chart 0 // get_chart(getenv("chart"sv), 0);
    // if (filename.empty())
    //     filename = chart_access.filename();
    // return chart_access.substitute_metadata(filename);

} // sacmacs::mapi::v1::Settings::ubstitute_in_filename

// ----------------------------------------------------------------------

std::string acmacs::mapi::v1::Settings::get_filename() const
{
    using namespace std::string_view_literals;
    return substitute_in_filename(getenv("filename"sv).to<std::string_view>());

} // acmacs::mapi::v1::Settings::get_filename

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_export() const
{
    chart_draw().chart(0).export_chart(get_filename());
    return true;

} // acmacs::mapi::v1::Settings::apply_reset

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_pdf() const
{
    using namespace std::string_view_literals;
    make_pdf(get_filename(), rjson::v3::read_number(getenv("width"sv), 800.0), rjson::v3::read_bool(getenv("open"sv), false));
    return true;

} // acmacs::mapi::v1::Settings::apply_pdf

// ----------------------------------------------------------------------

void acmacs::mapi::v1::Settings::make_pdf(std::string_view filename, double width, bool open) const
{
    using namespace std::string_view_literals;
    std::string fn;
    if (!acmacs::string::endswith_ignore_case(filename, ".pdf"sv))
        fn = fmt::format("{}.pdf", filename);
    else
        fn = filename;
    AD_INFO("generating \"{}\"", fn);
    chart_draw().calculate_viewport();
    chart_draw().draw(fn, width, report_time::no);
    if (open)
        acmacs::open_or_quicklook(true, false, fn);

} // acmacs::mapi::v1::Settings::make_pdf

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_relax()
{
    using namespace std::string_view_literals;
    auto& projection = chart_draw().chart(0).modified_projection();
    const auto status = projection.relax(acmacs::chart::optimization_options{});
    AD_INFO("relaxed {:.3f} <-- {:.3f} (elapsed: {})", status.final_stress, status.initial_stress, acmacs::format_duration(status.time));
    if (rjson::v3::read_bool(getenv("re-orient"sv), false)) {
        acmacs::chart::CommonAntigensSera common(chart_draw().chart(0).chart());
        auto master_projection = (*chart_draw().chart(0).chart().projections())[chart_draw().chart(0).projection_no()];
        const auto procrustes_data = acmacs::chart::procrustes(*master_projection, projection, common.points(), acmacs::chart::procrustes_scaling_t::no);
        projection.transformation(procrustes_data.transformation);
    }
    update_env_upon_projection_change();
    return true;

} // acmacs::mapi::v1::Settings::apply_relax

// ----------------------------------------------------------------------

const ChartAccess& acmacs::mapi::v1::Settings::get_chart(const rjson::v3::value& source, size_t dflt) const
{
    return source.visit([dflt, this]<typename Val>(const Val& val) -> const ChartAccess& {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::null>) {
            if (chart_draw().number_of_charts() > dflt)
                return chart_draw().chart(dflt);
            else
                return chart_draw().chart(0); // internal procrustes
        }
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::number>) {
            if (const auto no{val.template to<size_t>()}; chart_draw().number_of_charts() > no)
                return chart_draw().chart(no);
            else
                throw error{fmt::format("cannot get alterantive chart {}: just {} charts available", no, chart_draw().number_of_charts())};
        }
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::string>) {
            return chart_draw().chart(substitute_in_filename(val.template to<std::string_view>()));
        }
        else
            throw error{fmt::format("unrecognized \"chart\" for procrustes: {} (expected integer or name)", val)};
    });

} // acmacs::mapi::v1::Settings::get_chart

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_procrustes()
{
    using namespace std::string_view_literals;
    using namespace acmacs::chart;

    const auto scaling = rjson::v3::read_bool(getenv("scaling"sv), false) ? procrustes_scaling_t::yes : procrustes_scaling_t::no;
    const auto& secondary_chart = get_chart(getenv("chart"sv), 1).chart();
    const auto secondary_projection_no = rjson::v3::read_number(getenv("projection"sv), 0ul);
    if (secondary_projection_no >= secondary_chart.number_of_projections())
        throw error{fmt::format("invalid secondary chart projection number {} (chart has just {} projection(s))", secondary_projection_no, secondary_chart.number_of_projections())};

    ArrowPlotSpec arrow_plot_spec;
    arrow_plot_spec.threshold = rjson::v3::read_number(getenv("threshold"sv), arrow_plot_spec.threshold);
    if (const auto& arrow_data = getenv("arrow"sv); arrow_data.is_object()) {
        arrow_plot_spec.line_width = rjson::v3::read_number(arrow_data["line_width"sv], arrow_plot_spec.line_width);
        arrow_plot_spec.outline = rjson::v3::read_color(arrow_data["outline"sv], arrow_plot_spec.outline);
        if (const auto& head_data = arrow_data["head"sv]; head_data.is_object()) {
            arrow_plot_spec.arrow_width = rjson::v3::read_number(head_data["width"sv], arrow_plot_spec.arrow_width);
            arrow_plot_spec.arrow_outline_width = rjson::v3::read_number(head_data["outline_width"sv], arrow_plot_spec.arrow_outline_width);
            arrow_plot_spec.arrow_outline = rjson::v3::read_color(head_data["outline"sv], arrow_plot_spec.arrow_outline);
            arrow_plot_spec.arrow_fill = rjson::v3::read_color(head_data["fill"sv], arrow_plot_spec.arrow_fill);
        }
        else if (!head_data.is_null())
            AD_WARNING("invalid \"arrow\" \"head\": {} (object expected)", head_data);
    }
    else if (!arrow_data.is_null())
        AD_WARNING("invalid \"arrow\": {} (object expected)", arrow_data);

    // common points
    const auto match_level = CommonAntigensSera::match_level(rjson::v3::read_string(getenv("match"sv), "auto"sv));
    CommonAntigensSera common(chart_draw().chart(), secondary_chart, match_level);
    common.keep_only(select_antigens(getenv("antigens"sv), if_null::all), select_sera(getenv("sera"sv), if_null::all));
    // const std::vector<CommonAntigensSera::common_t> common_points = common.points(CommonAntigensSera::subset::all);

    auto [distances, procrustes_data] = procrustes_arrows(chart_draw(), *secondary_chart.projection(secondary_projection_no), common, scaling, arrow_plot_spec);

    auto& titl = title();
    if (titl.number_of_lines() == 0)
        titl.add_line(chart_draw().chart().make_name(chart_draw().chart(0).projection_no()));
    titl.add_line(secondary_chart.make_name(secondary_projection_no));
    titl.add_line(fmt::format("RMS: {:.6f}", procrustes_data.rms));

    if (rjson::v3::read_bool(getenv("report"sv), false))
        AD_INFO("Procrustes  AG:{}  SR:{}  RMS: {:.6f}\n\n{}", common.common_antigens(), common.common_sera(), procrustes_data.rms, common.report(2));
    else
        AD_INFO("Procrustes  AG:{}  SR:{}  RMS: {:.6f}", common.common_antigens(), common.common_sera(), procrustes_data.rms);

    if (const auto report_longest_arrows = rjson::v3::read_number(getenv("report_longest_arrows"sv), 0ul); report_longest_arrows > 0) {
        std::sort(std::begin(distances), std::end(distances), [](const auto& e1, const auto& e2) { return e1.second > e2.second; }); // longest first
        auto antigens = chart_draw().chart(0).chart().antigens();
        auto sera = chart_draw().chart(0).chart().sera();
        fmt::memory_buffer arrows;
        for (size_t p_no{0}; p_no < std::min(report_longest_arrows, distances.size()); ++p_no) {
            if (distances[p_no].first < antigens->size())
                fmt::format_to_mb(arrows, "    {:.2f} AG {:4d} {}\n", distances[p_no].second, distances[p_no].first, antigens->at(distances[p_no].first)->name_full());
            else
                fmt::format_to_mb(arrows, "    {:.2f} AG {:4d} {}\n", distances[p_no].second, distances[p_no].first - antigens->size(), sera->at(distances[p_no].first - antigens->size())->name_full());
        }
        AD_INFO("Longest arrows (max {} requested of {} available)\n{}", report_longest_arrows, distances.size(), fmt::to_string(arrows));
    }

    return true;

} // acmacs::mapi::v1::Settings::apply_procrustes

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_remove_procrustes()
{
    using namespace std::string_view_literals;

    chart_draw().map_elements().remove(sProcrustesArrowElementKeyword);

    if (rjson::v3::read_bool(getenv("clear-title"sv), true))
        title().remove_all_lines();

    return true;

} // acmacs::mapi::v1::Settings::apply_remove_procrustes

// ----------------------------------------------------------------------

//    # "flip_over_serum_line": 1 -- scale (1 - mirror, 0.1 - close to serum line, 0 - move to serum line)
//            }

bool acmacs::mapi::v1::Settings::apply_move()
{
    using namespace std::string_view_literals;
    auto antigen_indexes = select_antigens(getenv("antigens"sv), if_null::empty);
    auto serum_indexes = select_sera(getenv("sera"sv), if_null::empty);
    auto& projection = chart_draw().chart(0).modified_projection();
    const auto number_of_antigens{chart_draw().chart(0).number_of_antigens()};

    if (const auto to = read_coordinates(getenv("to"sv)); to.has_value()) {
        const auto move_to{to->get_not_transformed(chart_draw())};
        for (auto index : antigen_indexes)
            projection.move_point(index, move_to);
        for (auto index : serum_indexes)
            projection.move_point(index + number_of_antigens, move_to);
    }
    else if (const auto relative = getenv("relative"sv); !relative.is_null()) {
        if (relative.is_array() && relative.size() == 2) {
            const auto offset{chart_draw().chart(0).modified_inverted_transformation().transform(PointCoordinates{relative[0].to<double>(), relative[1].to<double>()})};
            auto layout = projection.layout();
            for (auto index : antigen_indexes)
                projection.move_point(index, layout->at(index) + offset);
            for (auto index : serum_indexes)
                projection.move_point(index + number_of_antigens, layout->at(index + number_of_antigens) + offset);
        }
        else
            throw error{fmt::format("unrecognized \"move\" \"relative\": {} (expected array of two numbers)", relative)};
    }
    else if (const auto flip_over_line = getenv("flip-over-line"sv); !flip_over_line.is_null()) {
        if (flip_over_line.is_array() && flip_over_line.size() == 2) {
            const auto p1{read_coordinates(flip_over_line[0])->get_not_transformed(chart_draw())}, p2{read_coordinates(flip_over_line[1])->get_not_transformed(chart_draw())};
            const acmacs::LineDefinedByEquation line(p1, p2);
            auto layout = projection.layout();
            for (auto index : antigen_indexes)
                projection.move_point(index, line.flip_over(layout->at(index), 1.0));
            for (auto index : serum_indexes)
                projection.move_point(index + number_of_antigens, line.flip_over(layout->at(index + number_of_antigens), 1.0));
        }
        else
            throw error{fmt::format(fmt::runtime("unrecognized \"move\" \"flip-over-line\": {} (expected array of two point locations, e.g. [{\"v\": [0, 8]}, {\"v\": [1, 8]}])"), flip_over_line)};
    }
    else if (const auto flip_over_serum_line = getenv("flip-over-serum-line"sv); !flip_over_serum_line.is_null()) {
        if (flip_over_serum_line.is_number()) {
            const auto flip_scale{flip_over_serum_line.to<double>()};
            const acmacs::chart::SerumLine serum_line(projection);
            auto layout = projection.layout();
            for (auto index : antigen_indexes)
                projection.move_point(index, serum_line.line().flip_over(layout->at(index), flip_scale));
            for (auto index : serum_indexes)
                projection.move_point(index + number_of_antigens, serum_line.line().flip_over(layout->at(index + number_of_antigens), flip_scale));
        }
        else
            throw error{fmt::format("unrecognized \"move\" \"flip-over-serum-line\": {} (expected scale (number), e.g. 1.0)", flip_over_serum_line)};
    }
    else
        throw error{fmt::format(R"(unrecognized "move": neither of "to", "relative", "flip-over-line", "flip-over-serum-line" provided)")};

    if (rjson::v3::read_bool(getenv("report"sv), false))
        AD_INFO("Moved AG:{} SR:{}", antigen_indexes.size(), serum_indexes.size());

    chart_draw().viewport_reset_used_by(); // to avoid warning when viewport is later changed (e.g. set)
    update_env_upon_projection_change(); // stress changed, update environment

    return true;

} // acmacs::mapi::v1::Settings::apply_move

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
