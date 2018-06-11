#include "acmacs-base/enumerate.hh"
#include "acmacs-base/statistics.hh"
//#include "acmacs-base/timeit.hh"
#include "acmacs-map-draw/mod-serum.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/select.hh"

// ----------------------------------------------------------------------

void ModSera::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = args().get_or_default("report", false);
    const auto report_names_threshold = args().get_or_default("report_names_threshold", 10U);
    try {
        const auto indices = SelectSera(verbose, report_names_threshold).select(aChartDraw, args()["select"]);
        const auto styl = style();
        aChartDraw.modify_sera(indices, styl, drawing_order());
        try { add_labels(aChartDraw, indices, aChartDraw.number_of_antigens(), args()["label"]); } catch (rjson::field_not_found&) {}
    }
    catch (rjson::field_not_found&) {
        throw unrecognized_mod{"no \"select\" in \"sera\" mod: " + args().to_json() };
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
        throw unrecognized_mod{"\"serum\" does not select single serum, mod: " + args().to_json()};
    return sera.front();
}

// ----------------------------------------------------------------------

void ModSerumHomologous::mark_serum(ChartDraw& aChartDraw, size_t serum_index)
{
    if (auto [present, mark_serum] = args().get_object_if("mark_serum"); present) {
        aChartDraw.modify_serum(serum_index, point_style_from_json(mark_serum), drawing_order_from_json(mark_serum));
        try { add_label(aChartDraw, serum_index, aChartDraw.number_of_antigens(), mark_serum["label"]); } catch (rjson::field_not_found&) {}
    }

} // ModSerumHomologous::mark_serum

// ----------------------------------------------------------------------

acmacs::chart::PointIndexList ModSerumHomologous::select_mark_antigens(ChartDraw& aChartDraw, size_t aSerumIndex, bool aVerbose)
{
    const auto antigen_indices = select_antigens(aChartDraw, aSerumIndex, aVerbose);
    if (auto [present, mark_antigen] = args().get_object_if("mark_antigen"); present) {
        aChartDraw.modify(antigen_indices, point_style_from_json(mark_antigen), drawing_order_from_json(mark_antigen));
        try { add_labels(aChartDraw, antigen_indices, 0, mark_antigen["label"]); } catch (rjson::field_not_found&) {}
    }
    return antigen_indices;
}

// ----------------------------------------------------------------------

acmacs::chart::PointIndexList ModSerumHomologous::select_antigens(ChartDraw& aChartDraw, size_t aSerumIndex, bool aVerbose) const
{
    if (auto [antigen_present, antigen_select] = args().get_object_if("antigen"); antigen_present) {
        const auto antigens = SelectAntigens(aVerbose).select(aChartDraw, antigen_select);
        if (antigens.empty())
            throw unrecognized_mod{"\"antigen\" does not select an antigen, mod: " + args().to_json()};
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
        throw unrecognized_mod{"no homologous antigens for serum, mod: " + args().to_json()};
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
    const auto verbose = args().get_or_default("report", false);
    const size_t serum_index = select_mark_serum(aChartDraw, verbose);
    const auto antigen_indices = select_mark_antigens(aChartDraw, serum_index, verbose);
    make_serum_circle(aChartDraw, serum_index, antigen_indices, radius_type_from_string(args().get_or_default("type", "empirical")), args().get_or_empty_object("circle"), verbose);
}

// ----------------------------------------------------------------------

void ModSerumCircle::make_serum_circle(ChartDraw& aChartDraw, size_t serum_index, const acmacs::chart::PointIndexList& antigen_indices, serum_circle_radius_type radius_type, const rjson::object& circle_plot_spec, bool verbose) const
{
    make_serum_circle(aChartDraw, serum_index, Scaled{calculate_radius(aChartDraw, serum_index, antigen_indices, radius_type, verbose)}, circle_plot_spec);

} // ModSerumCircle::make_serum_circle

// ----------------------------------------------------------------------

void ModSerumCircle::make_serum_circle(ChartDraw& aChartDraw, size_t aSerumIndex, Scaled aRadius, const rjson::object& circle_plot_spec) const
{
    auto& circle = aChartDraw.serum_circle(aSerumIndex, aRadius);
    if (!circle_plot_spec.empty()) {
        circle.fill(Color(circle_plot_spec.get_or_default("fill", "transparent")));
        const auto outline = circle_plot_spec.get_or_default("outline", "pink");
        const auto outline_width = circle_plot_spec.get_or_default("outline_width", 1.0);
        circle.outline(Color(outline), outline_width);
        if (auto [angles_present, angles] = circle_plot_spec.get_array_if("angle_degrees"); angles_present) {
            const double pi_180 = std::acos(-1) / 180.0;
            circle.angles(static_cast<double>(angles[0]) * pi_180, static_cast<double>(angles[1]) * pi_180);
        }
        const auto line_dash = circle_plot_spec.get_or_default("radius_line_dash", "");
        if (line_dash == "dash1")
            circle.radius_line_dash1();
        else if (line_dash == "dash2")
            circle.radius_line_dash2();
        else
            circle.radius_line_no_dash();
        circle.radius_line(Color(circle_plot_spec.get_or_default("radius_line_color", outline)), circle_plot_spec.get_or_default("radius_line_width", outline_width));
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
    const auto verbose = args().get_or_default("report", false);
    const size_t serum_index = select_mark_serum(aChartDraw, verbose);
    const auto antigen_indices = select_mark_antigens(aChartDraw, serum_index, verbose);
    apply(aChartDraw, serum_index, antigen_indices, args().get_or_empty_object("within_4fold"), args().get_or_empty_object("outside_4fold"), verbose);
}

// ----------------------------------------------------------------------

void ModSerumCoverage::apply(ChartDraw& aChartDraw, size_t serum_index, const acmacs::chart::PointIndexList& antigen_indices, const rjson::object& within_4fold, const rjson::object& outside_4fold, bool verbose)
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
    const auto verbose = args().get_or_default("report", false);
    const size_t serum_index = select_mark_serum(aChartDraw, verbose);
    const auto antigen_indices = select_mark_antigens(aChartDraw, serum_index, verbose);

    ModSerumCoverage mod_coverage;
    mod_coverage.apply(aChartDraw, serum_index, antigen_indices, args().get_or_empty_object("within_4fold"), args().get_or_empty_object("outside_4fold"), verbose);

    if (auto [present, data] = args().get_object_if("empirical"); present && data.get_or_default("show", true)) {
        ModSerumCircle mod_circle;
        mod_circle.make_serum_circle(aChartDraw, serum_index, antigen_indices, ModSerumCircle::serum_circle_radius_type::empirical, data, verbose);
    }
    if (auto [present, data] = args().get_object_if("theoretical"); present && data.get_or_default("show", true)) {
        ModSerumCircle mod_circle;
        mod_circle.make_serum_circle(aChartDraw, serum_index, antigen_indices, ModSerumCircle::serum_circle_radius_type::theoretical, data, verbose);
    }

} // ModSerumCoverageCircle::apply

// ----------------------------------------------------------------------

void ModSerumLine::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    auto layout = aChartDraw.layout();
    if (layout->number_of_dimensions() != 2)
        throw unrecognized_mod("invalid number of dimensions in projection: " + std::to_string(layout->number_of_dimensions()) + ", only 2 is supported");

    const auto linear_regression = acmacs::statistics::simple_linear_regression(layout->begin_sera_dimension(aChartDraw.number_of_antigens(), 0), layout->end_sera_dimension(aChartDraw.number_of_antigens(), 0), layout->begin_sera_dimension(aChartDraw.number_of_antigens(), 1));
    std::cerr << linear_regression << '\n';

    std::vector<double> distances;
    std::transform(layout->begin_sera(aChartDraw.number_of_antigens()), layout->end_sera(aChartDraw.number_of_antigens()), std::back_inserter(distances),
                   [&linear_regression](const auto& coord) { return linear_regression.distance_to(coord[0], coord[1]); });
    auto sd = acmacs::statistics::standard_deviation(distances.begin(), distances.end());
    std::cerr << "sd: " << sd.sd() << '\n';

    auto& line = aChartDraw.line(linear_regression.slope(), linear_regression.intercept());
    line.color(Color(args().get_or_default("color", "red")));
    line.line_width(args().get_or_default("line_width", 1.0));

} // ModSerumLine::apply

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
