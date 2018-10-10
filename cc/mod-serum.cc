#include "acmacs-base/enumerate.hh"
#include "acmacs-base/statistics.hh"
//#include "acmacs-base/timeit.hh"
#include "acmacs-chart-2/serum-line.hh"
#include "acmacs-map-draw/mod-serum.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/select.hh"

// ----------------------------------------------------------------------

void ModSera::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = rjson::get_or(args(), "report", false);
    const auto report_names_threshold = rjson::get_or(args(), "report_names_threshold", 10UL);
    if (const auto& select = args()["select"]; !select.is_null()) {
        const auto indices = SelectSera(verbose, report_names_threshold).select(aChartDraw, select);
        const auto styl = style();
        aChartDraw.modify_sera(indices, styl, drawing_order());
        if (const auto& label = args()["label"]; !label.is_null())
            add_labels(aChartDraw, indices, aChartDraw.number_of_antigens(), label);
    }
    else {
        throw unrecognized_mod{"no \"select\" in \"sera\" mod: " + rjson::to_string(args()) };
    }

} // ModSera::apply

// ----------------------------------------------------------------------

size_t ModSerumHomologous::select_mark_serum(ChartDraw& aChartDraw, bool aVerbose)
{
    const size_t serum_index = select_serum(aChartDraw, aVerbose);
    mark_serum(aChartDraw, serum_index);
    return serum_index;
}

// ----------------------------------------------------------------------

size_t ModSerumHomologous::select_serum(ChartDraw& aChartDraw, bool aVerbose) const
{
    const auto sera = SelectSera(aVerbose).select(aChartDraw, args()["serum"]);
    if (sera.size() != 1)
        throw unrecognized_mod{"\"serum\" does not select single serum, mod: " + rjson::to_string(args())};
    return sera.front();
}

// ----------------------------------------------------------------------

void ModSerumHomologous::mark_serum(ChartDraw& aChartDraw, size_t serum_index)
{
    if (const auto& mark_serum = args()["mark_serum"]; !mark_serum.is_null()) {
        aChartDraw.modify_serum(serum_index, point_style_from_json(mark_serum), drawing_order_from_json(mark_serum));
        if (const auto& label = mark_serum["label"]; !label.is_null())
            add_label(aChartDraw, serum_index, aChartDraw.number_of_antigens(), label);
    }

} // ModSerumHomologous::mark_serum

// ----------------------------------------------------------------------

acmacs::chart::PointIndexList ModSerumHomologous::select_mark_antigens(ChartDraw& aChartDraw, size_t aSerumIndex, bool aVerbose)
{
    const auto antigen_indices = select_antigens(aChartDraw, aSerumIndex, aVerbose);
    if (const auto& mark_antigen = args()["mark_antigen"]; !mark_antigen.is_null()) {
        aChartDraw.modify(antigen_indices, point_style_from_json(mark_antigen), drawing_order_from_json(mark_antigen));
        if (const auto& label = mark_antigen["label"]; !label.is_null())
            add_labels(aChartDraw, antigen_indices, 0, label);
    }
    return antigen_indices;
}

// ----------------------------------------------------------------------

acmacs::chart::PointIndexList ModSerumHomologous::select_antigens(ChartDraw& aChartDraw, size_t aSerumIndex, bool aVerbose) const
{
    if (const auto& antigen_select = args()["antigen"]; !antigen_select.is_null()) {
        const auto antigens = SelectAntigens(aVerbose).select(aChartDraw, antigen_select);
        if (antigens.empty())
            throw unrecognized_mod{"\"antigen\" does not select an antigen, mod: " + rjson::to_string(args())};
        return antigens;
    }
    else {
        return select_homologous_antigens(aChartDraw, aSerumIndex, aVerbose);
    }
}

// ----------------------------------------------------------------------

acmacs::chart::PointIndexList ModSerumHomologous::select_homologous_antigens(ChartDraw& aChartDraw, size_t aSerumIndex, bool aVerbose) const
{
    aChartDraw.chart().set_homologous(acmacs::chart::Chart::find_homologous_for_big_chart::yes);
    const auto antigen_indexes = aChartDraw.chart().serum(aSerumIndex)->homologous_antigens();
    if (antigen_indexes.empty())
        throw unrecognized_mod{"no homologous antigens for serum, mod: " + rjson::to_string(args())};
    if (aVerbose) {
        auto antigens = aChartDraw.chart().antigens();
        std::cerr << "INFO: homologous antigens selected: " << std::setfill(' ') << std::setw(4) << antigen_indexes.size() << '\n';
        for (auto index: antigen_indexes)
            std::cerr << "  AG " << std::setw(5) << index << ' ' << (*antigens)[index]->full_name() << '\n';
    }
    return antigen_indexes;
}

// ----------------------------------------------------------------------

ModSerumCircle::serum_circle_radius_type ModSerumCircle::radius_type_from_string(std::string radius_type_s)
{
    if (radius_type_s == "theoretical") {
        return serum_circle_radius_type::theoretical;
    }
    else if (radius_type_s == "empirical") {
        return serum_circle_radius_type::empirical;
    }
    else {
        std::cerr << "WARNING: unrecognized serum circle \"type\": \"" << radius_type_s << "\", \"empirical\" (default, currently used) or \"theoretical\" expected\n";
        return serum_circle_radius_type::empirical;
    }

} // ModSerumCircle::radius_type_from_string

// ----------------------------------------------------------------------

void ModSerumCircle::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = rjson::get_or(args(), "report", false);
    const size_t serum_index = select_mark_serum(aChartDraw, verbose);
    const auto antigen_indices = select_mark_antigens(aChartDraw, serum_index, verbose);
    make_serum_circle(aChartDraw, serum_index, antigen_indices, radius_type_from_string(std::string(rjson::get_or(args(), "type", "empirical"))), args()["circle"], verbose);
}

// ----------------------------------------------------------------------

void ModSerumCircle::make_serum_circle(ChartDraw& aChartDraw, size_t serum_index, const acmacs::chart::PointIndexList& antigen_indices, serum_circle_radius_type radius_type, const rjson::value& circle_plot_spec, bool verbose) const
{
    make_serum_circle(aChartDraw, serum_index, Scaled{calculate_radius(aChartDraw, serum_index, antigen_indices, radius_type, verbose)}, circle_plot_spec);

} // ModSerumCircle::make_serum_circle

// ----------------------------------------------------------------------

void ModSerumCircle::make_serum_circle(ChartDraw& aChartDraw, size_t aSerumIndex, Scaled aRadius, const rjson::value& circle_plot_spec) const
{
    auto& circle = aChartDraw.serum_circle(aSerumIndex, aRadius);
    if (!circle_plot_spec.empty()) {
        circle.fill(Color(rjson::get_or(circle_plot_spec, "fill", "transparent")));
        const auto outline = rjson::get_or(circle_plot_spec, "outline", "pink");
        const auto outline_width = rjson::get_or(circle_plot_spec, "outline_width", 1.0);
        circle.outline(Color(outline), outline_width);
        if (const auto& angles = circle_plot_spec["angle_degrees"]; !angles.is_null()) {
            const double pi_180 = std::acos(-1) / 180.0;
            circle.angles(static_cast<double>(angles[0]) * pi_180, static_cast<double>(angles[1]) * pi_180);
        }
        const auto line_dash = rjson::get_or(circle_plot_spec, "radius_line_dash", "");
        if (line_dash == "dash1")
            circle.radius_line_dash1();
        else if (line_dash == "dash2")
            circle.radius_line_dash2();
        else
            circle.radius_line_no_dash();
        circle.radius_line(Color(rjson::get_or(circle_plot_spec, "radius_line_color", outline)), rjson::get_or(circle_plot_spec, "radius_line_width", outline_width));
    }
}

// ----------------------------------------------------------------------

double ModSerumCircle::calculate_radius(ChartDraw& aChartDraw, size_t aSerumIndex, const acmacs::chart::PointIndexList& aAntigenIndices, serum_circle_radius_type radius_type, bool aVerbose) const
{
      // Timeit it("DEBUG: serum circle radius calculation for " + std::to_string(aSerumIndex) + ' ', aVerbose ? report_time::Yes : report_time::No);
    std::vector<double> radii;
    const char* radius_type_s;
    switch (radius_type) {
      case serum_circle_radius_type::theoretical:
          std::transform(std::begin(aAntigenIndices), std::end(aAntigenIndices), std::back_inserter(radii),
                         [&](size_t antigen_index) -> double { return aChartDraw.chart().serum_circle_radius_theoretical(antigen_index, aSerumIndex, aChartDraw.projection_no(), aVerbose); });
          radius_type_s = "theoretical";
          break;
      case serum_circle_radius_type::empirical:
          std::transform(std::begin(aAntigenIndices), std::end(aAntigenIndices), std::back_inserter(radii),
                         [&](size_t antigen_index) -> double { return aChartDraw.chart().serum_circle_radius_empirical(antigen_index, aSerumIndex, aChartDraw.projection_no(), aVerbose); });
          radius_type_s = "empirical";
          break;
    }
    double radius = 0;
    for (auto rad: radii) {
        if (rad > 0 && (radius <= 0 || rad < radius))
            radius = rad;
    }
    if (aVerbose) {
        std::cerr << "INFO: " << radius_type_s << " serum circle radius: " << radius << "\n  SR " << aSerumIndex << ' ' << aChartDraw.chart().serum(aSerumIndex)->full_name() << '\n';
        for (auto [no, antigen_index]: acmacs::enumerate(aAntigenIndices))
            std::cerr << "    radius: " << radii[no] << "  AG " << antigen_index << ' ' << aChartDraw.chart().antigen(antigen_index)->full_name() << '\n';
    }
    return radius;
}

// ----------------------------------------------------------------------

void ModSerumCoverage::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = rjson::get_or(args(), "report", false);
    const size_t serum_index = select_mark_serum(aChartDraw, verbose);
    const auto antigen_indices = select_mark_antigens(aChartDraw, serum_index, verbose);
    apply(aChartDraw, serum_index, antigen_indices, args()["within_4fold"], args()["outside_4fold"], verbose);
}

// ----------------------------------------------------------------------

void ModSerumCoverage::apply(ChartDraw& aChartDraw, size_t serum_index, const acmacs::chart::PointIndexList& antigen_indices, const rjson::value& within_4fold, const rjson::value& outside_4fold, bool verbose)
{
    acmacs::chart::PointIndexList within, outside;
    std::optional<size_t> antigen_index;
    for (auto ai = antigen_indices.begin(); ai != antigen_indices.end() && !antigen_index; ++ai) {
        try {
            aChartDraw.chart().serum_coverage(*ai, serum_index, within, outside);
            antigen_index = *ai;
        }
        catch (acmacs::chart::serum_coverage_error& err) {
            std::cerr << "WARNING: cannot use homologous antigen " << *ai << ": " << err.what() << '\n';
            within.clear();
            outside.clear();
        }
    }
    if (!antigen_index)
        throw std::runtime_error("cannot apply serum_coverage mod: no suitable antigen found?");

    if (verbose) {
        std::cerr << "INFO: serum coverage\n  SR " << serum_index << ' ' << aChartDraw.chart().serum(serum_index)->full_name()
                  << "\n  AG " << *antigen_index << ' ' << aChartDraw.chart().antigen(*antigen_index)->full_name()
                  << "\n  within 4fold:  " << within.size()
                  << "\n  outside 4fold: " << outside.size() << '\n';
    }
    if (!within.empty())
        aChartDraw.modify(within, point_style_from_json(within_4fold), drawing_order_from_json(within_4fold));
    if (!outside.empty())
        aChartDraw.modify(outside, point_style_from_json(outside_4fold), drawing_order_from_json(outside_4fold));
    mark_serum(aChartDraw, serum_index);

} // ModSerumCoverage::apply

// ----------------------------------------------------------------------

void ModSerumCoverageCircle::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = rjson::get_or(args(), "report", false);
    const size_t serum_index = select_mark_serum(aChartDraw, verbose);
    const auto antigen_indices = select_mark_antigens(aChartDraw, serum_index, verbose);

    ModSerumCoverage mod_coverage;
    mod_coverage.apply(aChartDraw, serum_index, antigen_indices, args()["within_4fold"], args()["outside_4fold"], verbose);

    if (const auto& data = args()["empirical"]; !data.is_null() && rjson::get_or(data, "show", true)) {
        ModSerumCircle mod_circle;
        mod_circle.make_serum_circle(aChartDraw, serum_index, antigen_indices, ModSerumCircle::serum_circle_radius_type::empirical, data, verbose);
    }
    if (const auto& data = args()["theoretical"]; !data.is_null() && rjson::get_or(data, "show", true)) {
        ModSerumCircle mod_circle;
        mod_circle.make_serum_circle(aChartDraw, serum_index, antigen_indices, ModSerumCircle::serum_circle_radius_type::theoretical, data, verbose);
    }

} // ModSerumCoverageCircle::apply

// ----------------------------------------------------------------------

void ModSerumLine::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    acmacs::chart::SerumLine serum_line(aChartDraw.projection());
    std::cerr << "INFO: " << serum_line << '\n';

    auto& line = aChartDraw.line(serum_line.line(), ChartDraw::apply_map_transformation::yes);
    line.color(Color(rjson::get_or(args(), "color", "red")));
    line.line_width(rjson::get_or(args(), "line_width", 1.0));

} // ModSerumLine::apply

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
