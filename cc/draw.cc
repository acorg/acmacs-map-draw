#include <memory>
#include <algorithm>

#include "acmacs-base/throw.hh"
#include "acmacs-base/log.hh"
#include "acmacs-base/float.hh"
#include "acmacs-base/enumerate.hh"
#include "acmacs-chart-1/lispmds.hh"
#include "acmacs-chart-1/ace.hh"
#include "acmacs-map-draw/draw.hh"

#ifdef ACMACS_TARGET_OS
#include "acmacs-draw/surface-cairo.hh"
#endif

#ifdef ACMACS_TARGET_BROWSER
#include "acmacs-draw/surface.hh"
#endif

// ----------------------------------------------------------------------

DrawingOrder::DrawingOrder(Chart_Type& aChart)
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

ChartDraw::ChartDraw(Chart_Type& aChart, size_t aProjectionNo)
    : mChart(aChart),
      mProjectionNo(aProjectionNo),
      mTransformation(mChart.projection(mProjectionNo).transformation()),
      mPointStyles(mChart.number_of_points()),
      mDrawingOrder(mChart)
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
    modify(mChart.reference_antigen_indices(), PointStyleDraw(PointStyle::Empty).fill("transparent").size(Pixels{8}), PointDrawingOrder::Lower);
    modify(mChart.serum_indices(), PointStyleDraw(PointStyle::Empty).shape(PointStyle::Shape::Box).fill("transparent").size(Pixels{8}), PointDrawingOrder::Lower);

} // ChartDraw::prepare

// ----------------------------------------------------------------------

size_t ChartDraw::number_of_antigens() const
{
    return mChart.number_of_antigens();

} // ChartDraw::number_of_antigens

// ----------------------------------------------------------------------

size_t ChartDraw::number_of_sera() const
{
    return mChart.number_of_sera();

} // ChartDraw::number_of_sera

// ----------------------------------------------------------------------

const acmacs::Viewport& ChartDraw::calculate_viewport(bool verbose)
{
    std::unique_ptr<BoundingBall> bb(transformed_layout().minimum_bounding_ball());
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
        THROW_OR_CERR(std::runtime_error("Call calculate_viewport() before draw()"));
    }

    Surface& rescaled_surface = aSurface.subsurface({0, 0}, Scaled{aSurface.viewport().size.width}, mViewport, true);
    mMapElements.draw(rescaled_surface, map_elements::Elements::BeforePoints, *this);

    const auto& layout = transformed_layout();
    if (mDrawingOrder.empty())
        acmacs::fill_with_indexes(const_cast<ChartDraw*>(this)->mDrawingOrder, number_of_points());
    for (auto index: mDrawingOrder) {
        mPointStyles[index].draw(rescaled_surface, layout[index]);
        // if (index < number_of_antigens())
        //     std::cout << "AG: " << index << ' ' << layout[index] << ' ' << mPointStyles[index].fill_raw() << " \"" << mChart.antigen(index).full_name() << "\"\n";
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
    PointStyleDraw style(PointStyle::Empty);
    style.hide();
    for (size_t index = 0; index < mPointStyles.size(); ++index) {
        if (std::find(aNotHide.begin(), aNotHide.end(), index) == aNotHide.end())
            mPointStyles[index] = style;
    }

} // ChartDraw::hide_all_except

// ----------------------------------------------------------------------

void ChartDraw::mark_egg_antigens()
{
    modify(mChart.egg_antigen_indices(), PointStyleDraw(PointStyle::Empty).aspect(AspectEgg));

} // ChartDraw::mark_egg_antigens

// ----------------------------------------------------------------------

void ChartDraw::mark_reassortant_antigens()
{
    modify(mChart.reassortant_antigen_indices(), PointStyleDraw(PointStyle::Empty).rotation(RotationReassortant));

} // ChartDraw::mark_reassortant_antigens

// ----------------------------------------------------------------------

void ChartDraw::modify_all_sera(const PointStyle& aStyle, PointDrawingOrder aPointDrawingOrder)
{
    modify(mChart.serum_indices(), aStyle, aPointDrawingOrder);

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
    modify(mChart.reference_antigen_indices(), PointStyleDraw(PointStyle::Empty).outline(aColor));
    modify(mChart.test_antigen_indices(), PointStyleDraw(PointStyle::Empty).fill(aColor).outline(aColor));
    modify(mChart.serum_indices(), PointStyleDraw(PointStyle::Empty).outline(aColor));

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

void ChartDraw::export_ace(std::string aFilename)
{
    if (mChart.number_of_projections()) {
        mChart.projection(0).transformation(transformation());
    }
    auto& plot_spec = mChart.plot_spec();
    plot_spec.reset(mChart);
    for (auto [index, new_style]: acmacs::enumerate(point_styles_base())) {
        ChartPlotSpecStyle style(new_style.fill(), new_style.outline(), ChartPlotSpecStyle::Circle, new_style.size().value() / 5);
        switch (new_style.shape()) {
          case PointStyle::Shape::NoChange:
              break;
          case PointStyle::Shape::Circle:
              style.set_shape(ChartPlotSpecStyle::Circle);
              break;
          case PointStyle::Shape::Box:
              style.set_shape(ChartPlotSpecStyle::Box);
              break;
          case PointStyle::Shape::Triangle:
              style.set_shape(ChartPlotSpecStyle::Triangle);
              break;
        }
        style.shown(new_style.shown());
        style.outline_width(new_style.outline_width().value());
        style.rotation(new_style.rotation().value());
        style.aspect(new_style.aspect().value());
          // style.label() =
        plot_spec.set(index, style);
    }
    plot_spec.drawing_order() = drawing_order();
    export_chart(aFilename, mChart, report_time::Yes);

} // ChartDraw::export_ace

// ----------------------------------------------------------------------

void ChartDraw::export_lispmds(std::string aFilename)
{
    export_chart_lispmds(aFilename, chart(), point_styles_base(), transformation());

} // ChartDraw::export_lispmds

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
