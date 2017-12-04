#include <memory>
#include <algorithm>

#include "acmacs-base/throw.hh"
#include "acmacs-base/log.hh"
#include "acmacs-base/float.hh"
#include "acmacs-base/enumerate.hh"
#include "acmacs-chart-2/factory-export.hh"
#include "acmacs-chart-2/bounding-ball.hh"
#include "acmacs-draw/surface-cairo.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

DrawingOrder::DrawingOrder(acmacs::chart::Chart& aChart)
    : std::vector<size_t>(acmacs::incrementer<size_t>::begin(0), acmacs::incrementer<size_t>::end(aChart.number_of_points()))
{
} // DrawingOrder::DrawingOrder

// ----------------------------------------------------------------------

void DrawingOrder::raise(size_t aPointNo)
{
    auto p = std::find(begin(), end(), aPointNo);
    if (p != end())
        std::rotate(p, p + 1, end());

} // DrawingOrder::raise

// ----------------------------------------------------------------------

void DrawingOrder::lower(size_t aPointNo)
{
    auto p = std::find(rbegin(), rend(), aPointNo);
    if (p != rend())
        std::rotate(p, p + 1, rend());

} // DrawingOrder::lower

// ----------------------------------------------------------------------

ChartDraw::ChartDraw(acmacs::chart::ChartModifyP aChart, size_t aProjectionNo)
    : mChart(aChart),
      mProjectionNo(aProjectionNo),
      mTransformation(mChart->projection(mProjectionNo)->transformation()),
      mPointStyles(mChart->number_of_points()),
      mDrawingOrder(*mChart)
{
      // std::cerr << "DrawingOrder: " << mDrawingOrder << std::endl;
    // auto ag_ind = aChart.antigen_indices(), sr_ind = aChart.serum_indices();
    // std::vector<size_t> ag(ag_ind.begin(), ag_ind.end());
    // std::cerr << "AG " << ag << std::endl;
    // std::vector<size_t> sr(sr_ind.begin(), sr_ind.end());
    // std::cerr << "SR " << sr << std::endl;
    // std::cerr << "AGref " << aChart.reference_antigen_indices() << std::endl;
    // std::cerr << "AGegg " << aChart.egg_antigen_indices() << std::endl;
}

// ----------------------------------------------------------------------

void ChartDraw::prepare()
{
    PointStyleDraw ref_antigen;
    ref_antigen.fill = "transparent";
    ref_antigen.size = Pixels{8};
    modify(chart().antigens()->reference_indexes(), ref_antigen, PointDrawingOrder::Lower);
    PointStyleDraw serum;
    serum.shape = acmacs::PointShape::Box;
    serum.fill = "transparent";
    serum.size = Pixels{8};
    modify_sera(chart().sera()->all_indexes(), serum, PointDrawingOrder::Lower);

} // ChartDraw::prepare

// ----------------------------------------------------------------------

const acmacs::Viewport& ChartDraw::calculate_viewport(bool verbose)
{
    std::unique_ptr<acmacs::BoundingBall> bb{transformed_layout().minimum_bounding_ball()};
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

    const auto& layout = transformed_layout();
    if (mDrawingOrder.empty())
        acmacs::fill_with_indexes(const_cast<ChartDraw*>(this)->mDrawingOrder, number_of_points());
    for (auto index: mDrawingOrder) {
        mPointStyles[index].draw(rescaled_surface, layout[index]);
        // if (index < number_of_antigens())
        //     std::cout << "AG: " << index << ' ' << layout[index] << ' ' << mPointStyles[index] << " \"" << chart().antigen(index)->full_name() << "\"\n";
    }
    mLabels.draw(rescaled_surface, layout, mPointStyles);

    mMapElements.draw(rescaled_surface, map_elements::Elements::AfterPoints, *this);

} // ChartDraw::draw

// ----------------------------------------------------------------------

#ifdef ACMACS_TARGET_OS
void ChartDraw::draw(std::string aFilename, double aSize, report_time aTimer) const
{
    Timeit ti("drawing map to " + aFilename + ": ", aTimer);
    PdfCairo surface(aFilename, aSize, aSize);
    draw(surface);

} // ChartDraw::draw
#endif

// ----------------------------------------------------------------------

void ChartDraw::hide_all_except(const std::vector<size_t>& aNotHide)
{
    PointStyleDraw style;
    style.shown = false;
    for (size_t index = 0; index < mPointStyles.size(); ++index) {
        if (std::find(aNotHide.begin(), aNotHide.end(), index) == aNotHide.end())
            mPointStyles[index] = style;
    }

} // ChartDraw::hide_all_except

// ----------------------------------------------------------------------

void ChartDraw::mark_egg_antigens()
{
    PointStyleDraw style;
    style.aspect = AspectEgg;
    modify(chart().antigens()->egg_indexes(), style);

} // ChartDraw::mark_egg_antigens

// ----------------------------------------------------------------------

void ChartDraw::mark_reassortant_antigens()
{
    PointStyleDraw style;
    style.rotation = RotationReassortant;
    modify(chart().antigens()->reassortant_indexes(), style);

} // ChartDraw::mark_reassortant_antigens

// ----------------------------------------------------------------------

void ChartDraw::modify_all_sera(const acmacs::PointStyle& aStyle, PointDrawingOrder aPointDrawingOrder)
{
    modify_sera(chart().sera()->all_indexes(), aStyle, aPointDrawingOrder);

} // ChartDraw::modify_all_sera

// ----------------------------------------------------------------------

void ChartDraw::scale_points(double aPointScale, double aOulineScale)
{
    // if (float_zero(aOulineScale))
    //     aOulineScale = aPointScale;
    for (auto& style: mPointStyles)
        style.scale(aPointScale).scale_outline(aOulineScale);

} // ChartDraw::scale_points

// ----------------------------------------------------------------------

void ChartDraw::mark_all_grey(Color aColor)
{
    PointStyleDraw ref_antigen;
    ref_antigen.outline = aColor;
    modify(chart().antigens()->reference_indexes(), ref_antigen);
    PointStyleDraw test_antigen;
    test_antigen.fill = aColor;
    test_antigen.outline = aColor;
    modify(chart().antigens()->test_indexes(), test_antigen);
    modify(chart().sera()->all_indexes(), ref_antigen);

} // ChartDraw::mark_all_grey

// ----------------------------------------------------------------------

map_elements::SerumCircle& ChartDraw::serum_circle(size_t aSerumNo, Scaled aRadius)
{
    auto& serum_circle = DYNAMIC_CAST(map_elements::SerumCircle&, (mMapElements.add("serum-circle")));
    serum_circle.serum_no(aSerumNo);
    serum_circle.radius(aRadius);
    return serum_circle;

} // ChartDraw::serum_circle

// ----------------------------------------------------------------------

map_elements::Line& ChartDraw::line(const acmacs::Location& aBegin, const acmacs::Location& aEnd)
{
    auto& line = DYNAMIC_CAST(map_elements::Line&, (mMapElements.add("line")));
    line.from_to(aBegin, aEnd);
    return line;

} // ChartDraw::line

// ----------------------------------------------------------------------

map_elements::Arrow& ChartDraw::arrow(const acmacs::Location& aBegin, const acmacs::Location& aEnd)
{
    auto& arrow = DYNAMIC_CAST(map_elements::Arrow&, (mMapElements.add("arrow")));
    arrow.from_to(aBegin, aEnd);
    return arrow;

} // ChartDraw::arrow

// ----------------------------------------------------------------------

map_elements::Point& ChartDraw::point(const acmacs::Location& aCenter, Pixels aSize)
{
    auto& point = DYNAMIC_CAST(map_elements::Point&, (mMapElements.add("point")));
    point.center(aCenter);
    point.size(aSize);
    return point;

} // ChartDraw::point

// ----------------------------------------------------------------------

map_elements::Rectangle& ChartDraw::rectangle(const acmacs::Location& aCorner1, const acmacs::Location& aCorner2)
{
    auto& rectangle = DYNAMIC_CAST(map_elements::Rectangle&, (mMapElements.add("rectangle")));
    rectangle.corners(aCorner1, aCorner2);
    return rectangle;

} // ChartDraw::rectangle

// ----------------------------------------------------------------------

map_elements::Circle& ChartDraw::circle(const acmacs::Location& aCenter, Scaled aSize)
{
    auto& point = DYNAMIC_CAST(map_elements::Circle&, (mMapElements.add("circle")));
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
    acmacs::chart::export_factory(chart(), aFilename, aProgramName);

} // ChartDraw::save

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
