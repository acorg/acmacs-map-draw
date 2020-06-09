#include <memory>
#include <algorithm>

#include "acmacs-base/float.hh"
#include "acmacs-base/fmt.hh"
#include "acmacs-base/enumerate.hh"
#include "acmacs-base/line.hh"
#include "acmacs-chart-2/factory-export.hh"
#include "acmacs-chart-2/bounding-ball.hh"
#include "acmacs-draw/draw-elements.hh"
#include "acmacs-draw/draw-points.hh"
#include "acmacs-draw/surface-cairo.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

void ChartDraw::reset()
{
    ChartSelectInterface::reset();

    viewport_ = MapViewport{};
    mMapElements.reset();
    mLabels.reset();

} // ChartDraw::reset

// ----------------------------------------------------------------------

const acmacs::Viewport& ChartDraw::MapViewport::use(std::string_view by) const
{
    if (recalculate_)
        AD_WARNING("map viewport requires recalculation, but it is used by {}", by);
    used_by_.emplace_back(by);
    return viewport_;

} // ChartDraw::MapViewport::use

// ----------------------------------------------------------------------

void ChartDraw::MapViewport::set(const acmacs::PointCoordinates& origin, double size)
{
    if (!used_by_.empty()) {
        AD_ERROR("map viewport change ({} {}) requested, but it was used by {}", origin, size, used_by_);
        used_by_.clear();
    }
    viewport_.set(origin, size);
    recalculate_ = false;

} // ChartDraw::MapViewport::set

// ----------------------------------------------------------------------

void ChartDraw::MapViewport::set(const acmacs::Viewport& aViewport)
{
    if (!used_by_.empty()) {
        AD_ERROR("map report change ({}) requested, but it was used by {}", aViewport, used_by_);
        used_by_.clear();
    }
    viewport_ = aViewport;
    recalculate_ = false;

} // ChartDraw::MapViewport::set

// ----------------------------------------------------------------------

void ChartDraw::MapViewport::calculate(const acmacs::Layout& layout)
{
    if (recalculate_) {
        if (!used_by_.empty()) {
            AD_ERROR("map report recalculation requested, but it was used by {}", used_by_);
            used_by_.clear();
        }
        if (layout.number_of_dimensions() != acmacs::number_of_dimensions_t{2})
            throw std::runtime_error{fmt::format("{}D maps are not supported", layout.number_of_dimensions())};
        const acmacs::BoundingBall bb{minimum_bounding_ball(layout)};
        acmacs::Viewport viewport;
        viewport.set_from_center_size(bb.center(), bb.diameter());
        viewport.whole_width();
        viewport_ = viewport;
        recalculate_ = false;
    }

} // ChartDraw::MapViewport::calculate

// ----------------------------------------------------------------------

void ChartDraw::calculate_viewport() const
{
    viewport_.calculate(*chart(0).modified_transformed_layout());

} // ChartDraw::calculate_viewport

// ----------------------------------------------------------------------

void ChartDraw::rotate(double aAngle)
{
    if (!float_zero(aAngle)) {
        //     log("rotate radians:", aAngle, " degrees:", 180.0 * aAngle / M_PI, " ", aAngle > 0 ? "counter-" : "", "clockwise");
        chart(0).modified_projection().rotate_radians(aAngle);
        viewport_.set_recalculate();
    }

} // ChartDraw::rotate

// ----------------------------------------------------------------------

void ChartDraw::flip(double aX, double aY)
{
    // log("flip ", aX, " ", aY);
    chart(0).modified_projection().flip(aX, aY); // reflect about a line specified with vector [aX, aY]
    viewport_.set_recalculate();

} // ChartDraw::flip

// ----------------------------------------------------------------------

void ChartDraw::draw(acmacs::surface::Surface& aSurface) const
{
    fmt::print(stderr, "\n\nWARNING: switch signature page to draw-elements interface\nWARNING: remove obsolete ChartDraw::draw(acmacs::surface::Surface&)\nWARNING: remove obsolete acmacs-map-draw interface: map_elements::draw(acmacs::surface::Surface& ...)\n");

      // obsolete
    if (viewport_.empty())
        throw std::runtime_error("Call calculate_viewport() before draw()");

    acmacs::surface::Surface& rescaled_surface = aSurface.subsurface(acmacs::PointCoordinates::zero2D, Scaled{aSurface.viewport().size.width}, viewport_.use("ChartDraw::draw(Surface)"), true);
    mMapElements.draw(rescaled_surface, map_elements::Elements::BeforePoints, *this);

    const auto layout_p = chart(0).modified_transformed_layout();
    const auto& layout = *layout_p;

    for (auto index: chart(0).modified_drawing_order()) {
        auto style = plot_spec().style(index);
        // std::cerr << index << ' ' << style << '\n';
        draw_point(rescaled_surface, style, layout[index]);
        // if (index < number_of_antigens())
        //     std::cout << "AG: " << index << ' ' << layout[index] << ' ' << mPointStyles[index] << " \"" << chart().antigen(index)->full_name() << "\"\n";
    }
    mLabels.draw(rescaled_surface, layout, plot_spec());

    mMapElements.draw(rescaled_surface, map_elements::Elements::AfterPoints, *this);

} // ChartDraw::draw

// ----------------------------------------------------------------------

void ChartDraw::draw(std::string_view aFilename, double aSize, report_time aTimer) const
{
    Timeit ti(fmt::format("drawing map to {}: ", aFilename), aTimer);

    acmacs::draw::DrawElementsToFile painter(aFilename, aSize);
    draw(painter);

    // }
    // else {
    //     if (std::string_view(aFilename.data() + aFilename.size() - 4, 4) == ".pdf") {
    //         acmacs::surface::PdfCairo surface(aFilename, aSize, aSize);
    //         draw(surface);
    //     }
    //     // Obsolete
    //     // else if (std::string_view(aFilename.data() + aFilename.size() - 5, 5) == ".html") {
    //     //     acmacs::surface::Html surface(aFilename, aSize, aSize);
    //     //     draw(surface);
    //     // }
    //     else {
    //         throw std::runtime_error("Unrecognized filename suffix: " + aFilename);
    //     }
    // }

} // ChartDraw::draw

// ----------------------------------------------------------------------

std::string ChartDraw::draw_pdf(double aSize, report_time aTimer) const
{
    Timeit ti("drawing map to pdf: ", aTimer);
    acmacs::surface::PdfBufferCairo surface(aSize, aSize);
    draw(surface);
    surface.flush();
    std::string d = surface.data();
    return surface.data();

} // ChartDraw::draw_pdf

// ----------------------------------------------------------------------

std::string ChartDraw::draw_json(report_time aTimer) const
{
    Timeit ti("drawing map to json: ", aTimer);
    acmacs::draw::DrawElementsToFile painter("//.json", 0);
    draw(painter);
    return painter.output();

} // ChartDraw::draw_json

// ----------------------------------------------------------------------

void ChartDraw::draw(acmacs::draw::DrawElements& painter) const
{
    if (viewport_.empty())
        throw std::runtime_error("Call calculate_viewport() before draw()");
    painter.viewport(viewport_.use("ChartDraw::draw(DrawElements)"));
    mMapElements.draw(painter, *this);
    auto& points = painter.points(chart(0).modified_layout(), chart(0).modified_transformation()).drawing_order(*chart(0).modified_drawing_order()).styles(chart(0).modified_plot_spec_ptr()).labels(mLabels);
    if (painter.add_all_labels()) {
        add_all_labels();
        points.labels(mLabels);
    }
    painter.draw();

} // ChartDraw::draw

// ----------------------------------------------------------------------

// void ChartDraw::hide_all_except(const acmacs::chart::Indexes& aNotHide)
// {
//     acmacs::PointStyleModified style;
//     style.shown(false);
//     for (size_t index = 0; index < chart(0).number_of_points(); ++index ) {
//         if (std::find(aNotHide.begin(), aNotHide.end(), index) == aNotHide.end())
//             plot_spec().modify(index, style);
//     }

// } // ChartDraw::hide_all_except

// ----------------------------------------------------------------------

void ChartDraw::mark_egg_antigens()
{
    acmacs::PointStyleModified style;
    style.aspect(AspectEgg);
    plot_spec().modify(chart().antigens()->egg_indexes(), style);

} // ChartDraw::mark_egg_antigens

// ----------------------------------------------------------------------

void ChartDraw::mark_reassortant_antigens()
{
    acmacs::PointStyleModified style;
    style.rotation(RotationReassortant);
    plot_spec().modify(chart().antigens()->reassortant_indexes(), style);

} // ChartDraw::mark_reassortant_antigens

// ----------------------------------------------------------------------

// void ChartDraw::mark_all_grey(Color aColor)
// {
//     acmacs::PointStyleModified ref_antigen;
//     ref_antigen.outline(acmacs::color::Modifier{aColor});
//     plot_spec().modify(chart().antigens()->reference_indexes(), ref_antigen);
//     acmacs::PointStyleModified test_antigen;
//     test_antigen.fill(acmacs::color::Modifier{aColor});
//     test_antigen.outline(acmacs::color::Modifier{aColor});
//     plot_spec().modify(chart().antigens()->test_indexes(), test_antigen);
//     plot_spec().modify(chart().sera()->all_indexes(), ref_antigen);

// } // ChartDraw::mark_all_grey

// ----------------------------------------------------------------------

map_elements::v1::SerumCircle& ChartDraw::serum_circle(size_t aSerumNo, Scaled aRadius)
{
    auto& serum_circle = dynamic_cast<map_elements::v1::SerumCircle&>(mMapElements.add("serum-circle"));
    serum_circle.serum_no(aSerumNo);
    serum_circle.radius(aRadius);
    return serum_circle;

} // ChartDraw::serum_circle

// ----------------------------------------------------------------------

map_elements::v1::Line& ChartDraw::line(const acmacs::PointCoordinates& aBegin, const acmacs::PointCoordinates& aEnd)
{
    auto& line = dynamic_cast<map_elements::v1::LineFromTo&>(mMapElements.add("line_from_to"));
    line.from_to(aBegin, aEnd);
    return line;

} // ChartDraw::line

// ----------------------------------------------------------------------

// map_elements::Line& ChartDraw::line(double slope, double intercept, apply_map_transformation a_apply_map_transformation)
// {
//     auto& line = dynamic_cast<map_elements::LineSlope&>(mMapElements.add("line_slope"));
//     line.slope_intercept(slope, intercept);
//     if (a_apply_map_transformation == apply_map_transformation::yes)
//         line.apply_map_transformation(true);
//     return line;

// } // ChartDraw::line

// ----------------------------------------------------------------------

map_elements::v1::Line& ChartDraw::line(const acmacs::LineDefinedByEquation& line, apply_map_transformation a_apply_map_transformation)
{
    auto& line_slope = dynamic_cast<map_elements::v1::LineSlope&>(mMapElements.add("line_slope"));
    line_slope.line(line);
    if (a_apply_map_transformation == apply_map_transformation::yes)
        line_slope.apply_map_transformation(true);
    return line_slope;

} // ChartDraw::line

// ----------------------------------------------------------------------

map_elements::v1::Path& ChartDraw::path()
{
    return dynamic_cast<map_elements::v1::Path&>(mMapElements.add("path"));

} // ChartDraw::path

// ----------------------------------------------------------------------

map_elements::v1::Arrow& ChartDraw::arrow(const acmacs::PointCoordinates& aBegin, const acmacs::PointCoordinates& aEnd)
{
    auto& arrow = dynamic_cast<map_elements::v1::Arrow&>(mMapElements.add("arrow"));
    arrow.from_to(aBegin, aEnd);
    return arrow;

} // ChartDraw::arrow

// ----------------------------------------------------------------------

map_elements::v1::Point& ChartDraw::point(const acmacs::PointCoordinates& aCenter, Pixels aSize)
{
    auto& point = dynamic_cast<map_elements::v1::Point&>(mMapElements.add("point"));
    point.center(aCenter);
    point.size(aSize);
    return point;

} // ChartDraw::point

// ----------------------------------------------------------------------

map_elements::v1::Rectangle& ChartDraw::rectangle(const acmacs::PointCoordinates& aCorner1, const acmacs::PointCoordinates& aCorner2)
{
    auto& rectangle = dynamic_cast<map_elements::v1::Rectangle&>(mMapElements.add("rectangle"));
    rectangle.corners(aCorner1, aCorner2);
    return rectangle;

} // ChartDraw::rectangle

// ----------------------------------------------------------------------

map_elements::v1::Circle& ChartDraw::circle(const acmacs::PointCoordinates& aCenter, Scaled aSize)
{
    auto& point = dynamic_cast<map_elements::v1::Circle&>(mMapElements.add("circle-v1"));
    point.center(aCenter);
    point.size(aSize);
    return point;

} // ChartDraw::circle

// ----------------------------------------------------------------------

void ChartDraw::remove_serum_circles()
{
    mMapElements.remove("serum-circle");

} // ChartDraw::remove_serum_circles

// ----------------------------------------------------------------------

void ChartDraw::save(std::string_view aFilename, std::string_view aProgramName)
{
    acmacs::chart::export_factory(chart(), aFilename, aProgramName, report_time::no);

} // ChartDraw::save

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
