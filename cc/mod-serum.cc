#include "acmacs-base/enumerate.hh"
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
    if (auto [present, mark_serum] = args().get_object_if("mark_serum"); present) {
        aChartDraw.modify_serum(serum_index, point_style_from_json(mark_serum), drawing_order_from_json(mark_serum));
        try { add_label(aChartDraw, serum_index, aChartDraw.number_of_antigens(), mark_serum["label"]); } catch (rjson::field_not_found&) {}
    }
    return serum_index;
}

// ----------------------------------------------------------------------

size_t ModSerumHomologous::select_serum(ChartDraw& aChartDraw, bool aVerbose) const
{
    const auto sera = SelectSera(aVerbose).select(aChartDraw, args()["serum"]);
    if (sera.size() != 1)
        throw unrecognized_mod{"\"serum\" does not select single serum, mod: " + args().to_json()};
    return sera[0];
}

// ----------------------------------------------------------------------

std::vector<size_t> ModSerumHomologous::select_mark_antigens(ChartDraw& aChartDraw, size_t aSerumIndex, bool aVerbose)
{
    const auto antigen_indices = select_antigens(aChartDraw, aSerumIndex, aVerbose);
    if (auto [present, mark_antigen] = args().get_object_if("mark_antigen"); present) {
        aChartDraw.modify(antigen_indices, point_style_from_json(mark_antigen), drawing_order_from_json(mark_antigen));
        try { add_labels(aChartDraw, antigen_indices, 0, mark_antigen["label"]); } catch (rjson::field_not_found&) {}
    }
    return antigen_indices;
}

// ----------------------------------------------------------------------

std::vector<size_t> ModSerumHomologous::select_antigens(ChartDraw& aChartDraw, size_t aSerumIndex, bool aVerbose) const
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

std::vector<size_t> ModSerumHomologous::select_homologous_antigens(ChartDraw& aChartDraw, size_t aSerumIndex, bool aVerbose) const
{
    aChartDraw.chart().set_homologous(true);
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

void ModSerumCircle::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = args().get_or_default("report", false);
    const size_t serum_index = select_mark_serum(aChartDraw, verbose);
    const auto antigen_indices = select_mark_antigens(aChartDraw, serum_index, verbose);
    make_serum_circle(aChartDraw, serum_index, Scaled{calculate_radius(aChartDraw, serum_index, antigen_indices, verbose)});
}

// ----------------------------------------------------------------------

void ModSerumCircle::make_serum_circle(ChartDraw& aChartDraw, size_t aSerumIndex, Scaled aRadius) const
{
    auto& circle = aChartDraw.serum_circle(aSerumIndex, aRadius);
    if (auto [present, circle_data] = args().get_object_if("circle"); present) {
        circle.fill(Color(circle_data.get_or_default("fill", "transparent")));
        const auto outline = circle_data.get_or_default("outline", "pink");
        const auto outline_width = circle_data.get_or_default("outline_width", 1.0);
        circle.outline(Color(outline), outline_width);
        if (auto [angles_present, angles] = circle_data.get_array_if("angle_degrees"); angles_present) {
            const double pi_180 = std::acos(-1) / 180.0;
            circle.angles(static_cast<double>(angles[0]) * pi_180, static_cast<double>(angles[1]) * pi_180);
        }
        const auto line_dash = circle_data.get_or_default("radius_line_dash", "");
        if (line_dash == "dash1")
            circle.radius_line_dash1();
        else if (line_dash == "dash2")
            circle.radius_line_dash2();
        else
            circle.radius_line_no_dash();
        circle.radius_line(Color(circle_data.get_or_default("radius_line_color", outline)), circle_data.get_or_default("radius_line_width", outline_width));
    }
}

// ----------------------------------------------------------------------

double ModSerumCircle::calculate_radius(ChartDraw& aChartDraw, size_t aSerumIndex, const std::vector<size_t>& aAntigenIndices, bool aVerbose) const
{
    std::vector<double> radii;
    std::transform(std::begin(aAntigenIndices), std::end(aAntigenIndices), std::back_inserter(radii),
                   [&](size_t antigen_index) -> double { return aChartDraw.chart().serum_circle_radius(antigen_index, aSerumIndex, aChartDraw.projection_no(), false); });
    double radius = 0;
    for (auto rad: radii) {
        if (rad > 0 && (radius <= 0 || rad < radius))
            radius = rad;
    }
    if (aVerbose) {
        std::cerr << "INFO: serum_circle radius: " << radius << "\n  SR " << aSerumIndex << ' ' << aChartDraw.chart().serum(aSerumIndex)->full_name() << '\n';
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
    const auto antigen_indices = select_antigens(aChartDraw, serum_index, verbose);

    std::vector<size_t> within, outside;
    aChartDraw.chart().serum_coverage(antigen_indices[0], serum_index, within, outside);
    if (verbose) {
        std::cerr << "INFO: serum coverage\n  SR " << serum_index << ' ' << aChartDraw.chart().serum(serum_index)->full_name()
                  << "\n  AG " << antigen_indices[0] << ' ' << aChartDraw.chart().antigen(antigen_indices[0])->full_name()
                  << "\n  within 4fold:  " << within.size()
                  << "\n  outside 4fold: " << outside.size() << '\n';
    }
    if (!within.empty()) {
        const auto& within_4fold = args().get_or_empty_object("within_4fold");
        aChartDraw.modify(within, point_style_from_json(within_4fold), drawing_order_from_json(within_4fold));
    }
    if (!outside.empty()) {
        const auto& outside_4fold = args().get_or_empty_object("outside_4fold");
        aChartDraw.modify(outside, point_style_from_json(outside_4fold), drawing_order_from_json(outside_4fold));
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
