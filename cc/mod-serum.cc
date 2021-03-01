#include "acmacs-base/enumerate.hh"
#include "acmacs-base/statistics.hh"
//#include "acmacs-base/timeit.hh"
#include "acmacs-chart-2/serum-line.hh"
#include "acmacs-chart-2/serum-circle.hh"
#include "acmacs-map-draw/mod-serum.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/select.hh"

// ----------------------------------------------------------------------

void ModSera::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto report = rjson::get_or(args(), "report", false);
    const auto report_names_threshold = rjson::get_or(args(), "report_names_threshold", 30UL);
    if (const auto& select = args()["select"]; !select.is_null()) {
        const auto indices = SelectSera(acmacs::verbose_from(report), report_names_threshold).select(aChartDraw, select);
        const auto styl = style();
        aChartDraw.modify_sera(indices, styl, drawing_order());
        if (const auto& label = args()["label"]; !label.is_null())
            add_labels(aChartDraw, indices, aChartDraw.chart().number_of_antigens(), label);
    }
    else {
        throw unrecognized_mod{fmt::format("no \"select\" in \"sera\" mod: {}", args())};
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

acmacs::chart::PointIndexList ModSerumHomologous::select_mark_sera(ChartDraw& aChartDraw, bool aVerbose)
{
    const auto serum_indexes = SelectSera(acmacs::verbose_from(aVerbose)).select(aChartDraw, args()["serum"]);
    if (serum_indexes->empty())
        fmt::print(stderr, "WARNING: no sera selected by {}\n", args()["serum"]);
    for (auto index : serum_indexes)
        mark_serum(aChartDraw, index);
    return serum_indexes;

} // ModSerumHomologous::select_mark_sera

// ----------------------------------------------------------------------

size_t ModSerumHomologous::select_serum(ChartDraw& aChartDraw, bool aVerbose) const
{
    const auto sera = SelectSera(acmacs::verbose_from(aVerbose)).select(aChartDraw, args()["serum"]);
    if (sera->size() != 1)
        throw unrecognized_mod{fmt::format("\"serum\" does not select single serum, mod: {}", args())};
    return sera->front();
}

// ----------------------------------------------------------------------

void ModSerumHomologous::mark_serum(ChartDraw& aChartDraw, size_t serum_index)
{
    if (const auto& mark_serum = args()["mark_serum"]; !mark_serum.is_null()) {
        aChartDraw.modify_serum(serum_index, point_style_from_json(mark_serum, aChartDraw.chart().serum(serum_index)->is_egg(acmacs::chart::reassortant_as_egg::yes) ? Color("#FF4040") : Color("#4040FF")), drawing_order_from_json(mark_serum));
        if (const auto& label = mark_serum["label"]; !label.is_null())
            add_label(aChartDraw, serum_index, aChartDraw.chart().number_of_antigens(), label);
    }

} // ModSerumHomologous::mark_serum

// ----------------------------------------------------------------------

acmacs::chart::PointIndexList ModSerumHomologous::select_mark_antigens(ChartDraw& aChartDraw, size_t aSerumIndex, acmacs::chart::find_homologous find_homologous_options, bool aVerbose)
{
    const auto antigen_indices = select_antigens(aChartDraw, aSerumIndex, find_homologous_options, aVerbose);
    if (const auto& mark_antigen = args()["mark_antigen"]; !mark_antigen.is_null()) {
        aChartDraw.modify(antigen_indices, point_style_from_json(mark_antigen), drawing_order_from_json(mark_antigen));
        if (const auto& label = mark_antigen["label"]; !label.is_null())
            add_labels(aChartDraw, antigen_indices, 0, label);
    }
    return antigen_indices;
}

// ----------------------------------------------------------------------

acmacs::chart::PointIndexList ModSerumHomologous::select_antigens(ChartDraw& aChartDraw, size_t aSerumIndex, acmacs::chart::find_homologous find_homologous_options, bool aVerbose) const
{
    if (const auto& antigen_select = args()["antigen"]; !antigen_select.is_null()) {
        const auto antigens = SelectAntigens(acmacs::verbose_from(aVerbose)).select(aChartDraw, antigen_select);
        if (antigens->empty())
            throw unrecognized_mod{fmt::format("\"antigen\" does not select an antigen, mod: {}", args())};
        return antigens;
    }
    else {
        return select_homologous_antigens(aChartDraw, aSerumIndex, find_homologous_options, aVerbose);
    }
}

// ----------------------------------------------------------------------

acmacs::chart::PointIndexList ModSerumHomologous::select_homologous_antigens(ChartDraw& aChartDraw, size_t aSerumIndex, acmacs::chart::find_homologous find_homologous_options, bool aVerbose) const
{
    aChartDraw.chart().set_homologous(find_homologous_options);
    const auto antigen_indexes = aChartDraw.chart().serum(aSerumIndex)->homologous_antigens();
    if (antigen_indexes->empty()) {
        std::cerr << "WARNING: no homologous antigens for serum " << aSerumIndex << '\n';
        // throw unrecognized_mod{fmt::format("no homologous antigens for serum, mod: {}", args())};
    }
    if (aVerbose) {
        auto antigens = aChartDraw.chart().antigens();
        std::cerr << "INFO: homologous antigens selected: " << std::setfill(' ') << std::setw(4) << antigen_indexes->size() << '\n';
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
    const auto antigen_indices = select_mark_antigens(aChartDraw, serum_index, acmacs::chart::find_homologous::all, verbose);
    if (const auto& circle = args()["circle"]; circle.is_null()) { // new spec
        const double fold = rjson::get_or(args(), "fold", 2.0);
        if (const auto& homologous_titer = args()["homologous_titer"]; !homologous_titer.is_null())
            make_serum_circle(aChartDraw, serum_index, antigen_indices, acmacs::chart::Titer(homologous_titer.to<std::string_view>()), args()["empirical"], args()["theoretical"], args()["fallback"], fold, verbose);
        else
            make_serum_circle(aChartDraw, serum_index, antigen_indices, args()["empirical"], args()["theoretical"], args()["fallback"], fold, verbose);
    }
    else                        // old spec
        make_serum_circle(aChartDraw, serum_index, antigen_indices, radius_type_from_string(std::string(rjson::get_or(args(), "type", "empirical"))), args()["circle"], 2.0, verbose);
}

// ----------------------------------------------------------------------

void ModSerumCircles::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = rjson::get_or(args(), "report", false);
    const auto serum_indexes = select_mark_sera(aChartDraw, verbose);
    for (auto serum_index : serum_indexes) {
        const auto antigen_indices = select_mark_antigens(aChartDraw, serum_index, acmacs::chart::find_homologous::all, verbose);
        const double fold = rjson::get_or(args(), "fold", 2.0);
        if (const auto& homologous_titer = args()["homologous_titer"]; !homologous_titer.is_null())
            make_serum_circle(aChartDraw, serum_index, antigen_indices, acmacs::chart::Titer(homologous_titer.to<std::string_view>()), args()["empirical"], args()["theoretical"], args()["fallback"], fold, verbose);
        else
            make_serum_circle(aChartDraw, serum_index, antigen_indices, args()["empirical"], args()["theoretical"], args()["fallback"], fold, verbose);
    }

} // ModSerumCircles::apply

// ----------------------------------------------------------------------

void ModSerumCircle::make_serum_circle(ChartDraw& aChartDraw, size_t serum_index, const acmacs::chart::PointIndexList& antigen_indices, acmacs::chart::Titer aHomologousTiter, const rjson::value& empirical_plot_spec, const rjson::value& theoretical_plot_spec, const rjson::value& fallback_plot_spec, double fold, bool verbose) const
{
    bool empirical_or_theoretical_shown = false;
    if (rjson::get_or(empirical_plot_spec, "show", true)) {
        if (const auto radius = calculate_radius(aChartDraw, serum_index, antigen_indices, aHomologousTiter, serum_circle_radius_type::empirical, fold, verbose); radius > 0) {
            make_serum_circle(aChartDraw, serum_index, Scaled{radius}, empirical_plot_spec);
            empirical_or_theoretical_shown = true;
        }
    }
    if (rjson::get_or(theoretical_plot_spec, "show", true)) {
        if (const auto radius = calculate_radius(aChartDraw, serum_index, antigen_indices, aHomologousTiter, serum_circle_radius_type::theoretical, fold, verbose); radius > 0) {
            make_serum_circle(aChartDraw, serum_index, Scaled{radius}, theoretical_plot_spec);
            empirical_or_theoretical_shown = true;
        }
    }
    if (!empirical_or_theoretical_shown && rjson::get_or(fallback_plot_spec, "show", true)) {
        make_serum_circle(aChartDraw, serum_index, Scaled{rjson::get_or(fallback_plot_spec, "radius", 3.0)}, fallback_plot_spec);
    }

} // ModSerumCircle::make_serum_circle

// ----------------------------------------------------------------------

void ModSerumCircle::make_serum_circle(ChartDraw& aChartDraw, size_t serum_index, const acmacs::chart::PointIndexList& antigen_indices, const rjson::value& empirical_plot_spec, const rjson::value& theoretical_plot_spec, const rjson::value& fallback_plot_spec, double fold, bool verbose) const
{
    bool empirical_or_theoretical_shown = false;
    if (rjson::get_or(empirical_plot_spec, "show", true)) {
        if (const auto radius = calculate_radius(aChartDraw, serum_index, antigen_indices, serum_circle_radius_type::empirical, fold, verbose); radius > 0) {
            make_serum_circle(aChartDraw, serum_index, Scaled{radius}, empirical_plot_spec);
            empirical_or_theoretical_shown = true;
        }
    }
    if (rjson::get_or(theoretical_plot_spec, "show", true)) {
        if (const auto radius = calculate_radius(aChartDraw, serum_index, antigen_indices, serum_circle_radius_type::theoretical, fold, verbose); radius > 0) {
            make_serum_circle(aChartDraw, serum_index, Scaled{radius}, theoretical_plot_spec);
            empirical_or_theoretical_shown = true;
        }
    }
    if (!empirical_or_theoretical_shown && rjson::get_or(fallback_plot_spec, "show", true)) {
        make_serum_circle(aChartDraw, serum_index, Scaled{rjson::get_or(fallback_plot_spec, "radius", 3.0)}, fallback_plot_spec);
    }

} // ModSerumCircle::make_serum_circle

// ----------------------------------------------------------------------

void ModSerumCircle::make_serum_circle(ChartDraw& aChartDraw, size_t serum_index, const acmacs::chart::PointIndexList& antigen_indices, serum_circle_radius_type radius_type, const rjson::value& circle_plot_spec, double fold, bool verbose) const
{
    make_serum_circle(aChartDraw, serum_index, Scaled{calculate_radius(aChartDraw, serum_index, antigen_indices, radius_type, fold, verbose)}, circle_plot_spec);

} // ModSerumCircle::make_serum_circle

// ----------------------------------------------------------------------

void ModSerumCircle::make_serum_circle(ChartDraw& aChartDraw, size_t serum_index, const acmacs::chart::PointIndexList& antigen_indices, const rjson::value& homologous_titer,
                                       serum_circle_radius_type radius_type, const rjson::value& circle_plot_spec, double fold, bool verbose) const
{
    if (!homologous_titer.is_null())
        make_serum_circle(aChartDraw, serum_index, Scaled{calculate_radius(aChartDraw, serum_index, antigen_indices, acmacs::chart::Titer(homologous_titer.to<std::string_view>()), radius_type, fold, verbose)}, circle_plot_spec);
    else
        make_serum_circle(aChartDraw, serum_index, Scaled{calculate_radius(aChartDraw, serum_index, antigen_indices, radius_type, fold, verbose)}, circle_plot_spec);

} // ModSerumCircle::make_serum_circle

// ----------------------------------------------------------------------

void ModSerumCircle::make_serum_circle(ChartDraw& aChartDraw, size_t aSerumIndex, Scaled aRadius, const rjson::value& circle_plot_spec) const
{
    auto& circle = aChartDraw.serum_circle(aSerumIndex, aRadius);
    if (!circle_plot_spec.empty()) {
        circle.fill(Color(rjson::get_or(circle_plot_spec, "fill", "transparent")));
        auto outline = rjson::get_or(circle_plot_spec, "outline", "pink");
        if (outline == "passage") {
            if (aChartDraw.chart().serum(aSerumIndex)->is_egg(acmacs::chart::reassortant_as_egg::yes))
                outline = "#FF4040";
            else
                outline = "#4040FF";
        }
        const auto outline_width = rjson::get_or(circle_plot_spec, "outline_width", 1.0);
        if (const auto outline_dash = rjson::get_or(circle_plot_spec, "outline_dash", ""); outline_dash == "dash1")
            circle.outline_dash1();
        else if (outline_dash == "dash2")
            circle.outline_dash2();
        else if (outline_dash == "dash3")
            circle.outline_dash3();
        else
            circle.outline_no_dash();
        circle.outline(Color(outline), Pixels{outline_width});
        if (const auto& angles = circle_plot_spec["angle_degrees"]; !angles.is_null()) {
            const double pi_180 = std::acos(-1) / 180.0;
            circle.angles(Rotation{angles[0].to<double>() * pi_180}, Rotation{angles[1].to<double>() * pi_180});
        }
        if (const auto line_dash = rjson::get_or(circle_plot_spec, "radius_line_dash", ""); line_dash == "dash1")
            circle.radius_line_dash1();
        else if (line_dash == "dash2")
            circle.radius_line_dash2();
        else if (line_dash == "dash3")
            circle.radius_line_dash3();
        else
            circle.radius_line_no_dash();
        circle.radius_line(Color(rjson::get_or(circle_plot_spec, "radius_line_color", outline)), Pixels{rjson::get_or(circle_plot_spec, "radius_line_width", outline_width)});
    }
}

// ----------------------------------------------------------------------

double ModSerumCircle::calculate_radius(ChartDraw& aChartDraw, size_t aSerumIndex, const acmacs::chart::PointIndexList& aAntigenIndices, acmacs::chart::Titer aHomologousTiter, serum_circle_radius_type radius_type, double fold, bool aVerbose) const
{
    auto get_circle_data = [&]() {
        switch (radius_type) {
            case serum_circle_radius_type::empirical:
                return acmacs::chart::serum_circle_empirical(aAntigenIndices, aHomologousTiter, aSerumIndex, aChartDraw.chart(), aChartDraw.chart(0).projection_no(), fold);
            case serum_circle_radius_type::theoretical:
                return acmacs::chart::serum_circle_theoretical(aAntigenIndices, aHomologousTiter, aSerumIndex, aChartDraw.chart(), aChartDraw.chart(0).projection_no(), fold);
        }
        throw std::exception{};
    };

    const auto circle_data = get_circle_data();
    if (aVerbose) {
        std::cout << "INFO: " << (radius_type == serum_circle_radius_type::empirical ? "empirical" : "theoretical");
        if (circle_data.valid())
            std::cout << " serum circle radius: " << circle_data.radius();
        else
            std::cout << " NO serum circle radius";
        std::cout << "\n  SR " << aSerumIndex << ' ' << aChartDraw.chart().serum(aSerumIndex)->full_name() << '\n';
        std::cout << "  Forced homologous titer: " << *aHomologousTiter << '\n';
    }
    return circle_data.valid() ? circle_data.radius() : 0.0;

} // ModSerumCircle::calculate_radius

// ----------------------------------------------------------------------

double ModSerumCircle::calculate_radius(ChartDraw& aChartDraw, size_t aSerumIndex, const acmacs::chart::PointIndexList& aAntigenIndices, serum_circle_radius_type radius_type, double fold, bool aVerbose) const
{
    auto get_circle_data = [&]() {
        switch (radius_type) {
            case serum_circle_radius_type::empirical:
                return acmacs::chart::serum_circle_empirical(aAntigenIndices, aSerumIndex, aChartDraw.chart(), aChartDraw.chart(0).projection_no(), fold);
            case serum_circle_radius_type::theoretical:
                return acmacs::chart::serum_circle_theoretical(aAntigenIndices, aSerumIndex, aChartDraw.chart(), aChartDraw.chart(0).projection_no(), fold);
        }
        throw std::exception{};
    };

    const auto circle_data = get_circle_data();
    if (aVerbose) {
        std::cout << "INFO: " << (radius_type == serum_circle_radius_type::empirical ? "empirical" : "theoretical");
        if (circle_data.valid())
            std::cout << " serum circle radius: " << circle_data.radius();
        else
            std::cout << " NO serum circle radius";
        std::cout << "\n  SR " << aSerumIndex << ' ' << aChartDraw.chart().serum(aSerumIndex)->full_name() << '\n';
        for (const auto& per_antigen : circle_data.per_antigen()) {
            std::cout << "    ";
            if (per_antigen.valid())
                std::cout << "radius:" << *per_antigen.radius;
            else
                std::cout << per_antigen.report_reason();
            std::cout << "  AG " << per_antigen.antigen_no << ' ' << aChartDraw.chart().antigen(per_antigen.antigen_no)->full_name() << " titer:" << *per_antigen.titer << '\n';
        }
    }
    return circle_data.valid() ? circle_data.radius() : 0.0;
}

// ----------------------------------------------------------------------

void ModSerumCoverage::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = rjson::get_or(args(), "report", false);
    const size_t serum_index = select_mark_serum(aChartDraw, verbose);
    const auto antigen_indices = select_mark_antigens(aChartDraw, serum_index, acmacs::chart::find_homologous::all, verbose);
    apply(aChartDraw, serum_index, antigen_indices, args()["homologous_titer"], rjson::get_or(args(), "fold", 2.0), args()["within_4fold"], args()["outside_4fold"], verbose);
}

// ----------------------------------------------------------------------

void ModSerumCoverage::apply(ChartDraw& aChartDraw, size_t serum_index, const acmacs::chart::PointIndexList& antigen_indices, const rjson::value& homologous_titer, double fold,
                             const rjson::value& within_4fold, const rjson::value& outside_4fold, bool verbose)
{
    auto titers = aChartDraw.chart().titers();

    const auto serum_coverage = [&]() {
        if (!homologous_titer.is_null()) {
            return acmacs::chart::serum_coverage(*titers, acmacs::chart::Titer(homologous_titer.to<std::string_view>()), serum_index, fold);
        }
        else {
            for (const auto antigen_index : antigen_indices) {
                try {
                    return acmacs::chart::serum_coverage(*titers, antigen_index, serum_index, fold);
                }
                catch (acmacs::chart::serum_coverage_error& err) {
                    AD_WARNING("cannot use homologous AG {}: {} ", antigen_index, err);
                }
            }
            AD_WARNING("cannot apply serum_coverage_mod for SR {}: no suitable antigen found?", serum_index);
            return acmacs::chart::SerumCoverageIndexes{};
        }
    };

    const auto serum_coverage_data = serum_coverage();
    if (verbose) {
        fmt::print("INFO: serum coverage\n  SR {} {}\n", serum_index, aChartDraw.chart().serum(serum_index)->full_name());
        if (serum_coverage_data.antigen_index.has_value())
            fmt::print("  AG {} {}\n", *serum_coverage_data.antigen_index, aChartDraw.chart().antigen(*serum_coverage_data.antigen_index)->full_name());
        else
            fmt::print("  forced homologous titer: {}\n", homologous_titer);
        fmt::print("  within 4fold:  {}\n  outside 4fold: \n", serum_coverage_data.within->size(), serum_coverage_data.outside->size());
    }
    if (!serum_coverage_data.within->empty())
        aChartDraw.modify(serum_coverage_data.within, point_style_from_json(within_4fold), drawing_order_from_json(within_4fold));
    if (!serum_coverage_data.outside->empty())
        aChartDraw.modify(serum_coverage_data.outside, point_style_from_json(outside_4fold), drawing_order_from_json(outside_4fold));
    mark_serum(aChartDraw, serum_index);

} // ModSerumCoverage::apply

// ----------------------------------------------------------------------

void ModSerumCoverageCircle::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = rjson::get_or(args(), "report", false);
    const size_t serum_index = select_mark_serum(aChartDraw, verbose);
    const auto antigen_indices = select_mark_antigens(aChartDraw, serum_index, acmacs::chart::find_homologous::all, verbose);
    const double fold = rjson::get_or(args(), "fold", 2.0);

    ModSerumCoverage mod_coverage;
    mod_coverage.apply(aChartDraw, serum_index, antigen_indices, args()["homologous_titer"], fold, args()["within_4fold"], args()["outside_4fold"], verbose);

    if (const auto& data = args()["empirical"]; !data.is_null() && rjson::get_or(data, "show", true)) {
        ModSerumCircle mod_circle;
        mod_circle.make_serum_circle(aChartDraw, serum_index, antigen_indices, args()["homologous_titer"], ModSerumCircle::serum_circle_radius_type::empirical, data, fold, verbose);
    }
    if (const auto& data = args()["theoretical"]; !data.is_null() && rjson::get_or(data, "show", true)) {
        ModSerumCircle mod_circle;
        mod_circle.make_serum_circle(aChartDraw, serum_index, antigen_indices, args()["homologous_titer"], ModSerumCircle::serum_circle_radius_type::theoretical, data, fold, verbose);
    }

    mark_serum(aChartDraw, serum_index); // to have serum on top of antigens

} // ModSerumCoverageCircle::apply

// ----------------------------------------------------------------------

void ModSerumLine::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    acmacs::chart::SerumLine serum_line(aChartDraw.chart(0).modified_projection());
    std::cerr << "INFO: " << serum_line << '\n';

    auto& line = aChartDraw.line(serum_line.line(), ChartDraw::apply_map_transformation::yes);
    line.color(Color(rjson::get_or(args(), "color", "red")));
    line.line_width(rjson::get_or(args(), "line_width", 1.0));

} // ModSerumLine::apply

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
