#include <memory>
#include <algorithm>

#include "acmacs-base/log.hh"
#include "acmacs-base/float.hh"
#include "acmacs-base/enumerate.hh"
#include "acmacs-chart-2/factory-export.hh"
#include "acmacs-chart-2/bounding-ball.hh"
#include "acmacs-draw/surface-cairo.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

const acmacs::Viewport& ChartDraw::calculate_viewport(bool verbose)
{
    auto layout = transformed_layout();
    std::unique_ptr<acmacs::BoundingBall> bb{minimum_bounding_ball(*layout)};
    acmacs::Viewport viewport;
    viewport.set_from_center_size(bb->center(), bb->diameter());
    viewport.whole_width();
    if (verbose)
        std::cout << "[Calculated]:     " << viewport << '\n';
    if (mViewport.empty())
        mViewport = viewport;
    if (verbose) {
        std::cout << "[Used]:           " << mViewport << '\n';
        std::cout << "[Transformation]: " << transformation() << '\n';
    }
    return mViewport;

} // ChartDraw::calculate_viewport

// ----------------------------------------------------------------------

void ChartDraw::draw(Surface& aSurface) const
{
    if (mViewport.empty()) {
        throw std::runtime_error("Call calculate_viewport() before draw()");
    }

    Surface& rescaled_surface = aSurface.subsurface({0, 0}, Scaled{aSurface.viewport().size.width}, mViewport, true);
    mMapElements.draw(rescaled_surface, map_elements::Elements::BeforePoints, *this);

    const auto layout_p = transformed_layout();
    const auto& layout = *layout_p;

    for (auto index: drawing_order()) {
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

void ChartDraw::draw(std::string aFilename, double aSize, report_time aTimer) const
{
    Timeit ti("drawing map to " + aFilename + ": ", aTimer);
    PdfCairo surface(aFilename, aSize, aSize);
    draw(surface);

} // ChartDraw::draw

// ----------------------------------------------------------------------

void ChartDraw::hide_all_except(const acmacs::chart::Indexes& aNotHide)
{
    acmacs::PointStyle style;
    style.shown = false;
    for (size_t index = 0; index < number_of_points(); ++index ) {
        if (std::find(aNotHide.begin(), aNotHide.end(), index) == aNotHide.end())
            plot_spec().modify(index, style);
    }

} // ChartDraw::hide_all_except

// ----------------------------------------------------------------------

void ChartDraw::mark_egg_antigens()
{
    acmacs::PointStyle style;
    style.aspect = AspectEgg;
    plot_spec().modify(chart().antigens()->egg_indexes(), style);

} // ChartDraw::mark_egg_antigens

// ----------------------------------------------------------------------

void ChartDraw::mark_reassortant_antigens()
{
    acmacs::PointStyle style;
    style.rotation = RotationReassortant;
    plot_spec().modify(chart().antigens()->reassortant_indexes(), style);

} // ChartDraw::mark_reassortant_antigens

// ----------------------------------------------------------------------

void ChartDraw::mark_all_grey(Color aColor)
{
    acmacs::PointStyle ref_antigen;
    ref_antigen.outline = aColor;
    plot_spec().modify(chart().antigens()->reference_indexes(), ref_antigen);
    acmacs::PointStyle test_antigen;
    test_antigen.fill = aColor;
    test_antigen.outline = aColor;
    plot_spec().modify(chart().antigens()->test_indexes(), test_antigen);
    plot_spec().modify(chart().sera()->all_indexes(), ref_antigen);

} // ChartDraw::mark_all_grey

// ----------------------------------------------------------------------

map_elements::SerumCircle& ChartDraw::serum_circle(size_t aSerumNo, Scaled aRadius)
{
    auto& serum_circle = dynamic_cast<map_elements::SerumCircle&>(mMapElements.add("serum-circle"));
    serum_circle.serum_no(aSerumNo);
    serum_circle.radius(aRadius);
    return serum_circle;

} // ChartDraw::serum_circle

// ----------------------------------------------------------------------

map_elements::Line& ChartDraw::line(const acmacs::Location& aBegin, const acmacs::Location& aEnd)
{
    auto& line = dynamic_cast<map_elements::Line&>(mMapElements.add("line"));
    line.from_to(aBegin, aEnd);
    return line;

} // ChartDraw::line

// ----------------------------------------------------------------------

map_elements::Arrow& ChartDraw::arrow(const acmacs::Location& aBegin, const acmacs::Location& aEnd)
{
    auto& arrow = dynamic_cast<map_elements::Arrow&>(mMapElements.add("arrow"));
    arrow.from_to(aBegin, aEnd);
    return arrow;

} // ChartDraw::arrow

// ----------------------------------------------------------------------

map_elements::Point& ChartDraw::point(const acmacs::Location& aCenter, Pixels aSize)
{
    auto& point = dynamic_cast<map_elements::Point&>(mMapElements.add("point"));
    point.center(aCenter);
    point.size(aSize);
    return point;

} // ChartDraw::point

// ----------------------------------------------------------------------

map_elements::Rectangle& ChartDraw::rectangle(const acmacs::Location& aCorner1, const acmacs::Location& aCorner2)
{
    auto& rectangle = dynamic_cast<map_elements::Rectangle&>(mMapElements.add("rectangle"));
    rectangle.corners(aCorner1, aCorner2);
    return rectangle;

} // ChartDraw::rectangle

// ----------------------------------------------------------------------

map_elements::Circle& ChartDraw::circle(const acmacs::Location& aCenter, Scaled aSize)
{
    auto& point = dynamic_cast<map_elements::Circle&>(mMapElements.add("circle"));
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

void ChartDraw::save(std::string aFilename, std::string aProgramName)
{
    acmacs::chart::export_factory(chart(), aFilename, aProgramName, report_time::No);

} // ChartDraw::save

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
