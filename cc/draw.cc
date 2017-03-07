#include <memory>
#include <algorithm>

#include "acmacs-base/range.hh"
#include "acmacs-base/float.hh"
#include "acmacs-chart/chart.hh"
#include "acmacs-draw/surface-cairo.hh"

#include "draw.hh"

// ----------------------------------------------------------------------

DrawingOrder::DrawingOrder(Chart& aChart)
    : std::vector<size_t>(Range<size_t>::begin(aChart.number_of_points()), Range<size_t>::end())
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

ChartDraw::ChartDraw(Chart& aChart, size_t aProjectionNo)
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
    modify(mChart.reference_antigen_indices(), PointStyleDraw(PointStyle::Empty).fill("transparent").size(Pixels{8}), false, true); // lower
    modify(mChart.serum_indices(), PointStyleDraw(PointStyle::Empty).shape(PointStyle::Shape::Box).fill("transparent").size(Pixels{8}), false, true); // lower

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

void ChartDraw::draw(Surface& aSurface)
{
    const Layout& layout = transformed_layout();

    std::unique_ptr<BoundingBall> bb(layout.minimum_bounding_ball());
    Viewport viewport;
    viewport.set_from_center_size(bb->center(), bb->diameter());
    viewport.whole_width();
    std::cout << "INFO: [Calculated]: " << viewport << std::endl;
    if (mViewport.empty())
        mViewport = viewport;
    std::cout << "INFO: [Used]:       " << mViewport << std::endl;
    Surface& rescaled_surface = aSurface.subsurface({0, 0}, Scaled{aSurface.viewport().size.width}, mViewport, true);

    mMapElements.draw(rescaled_surface, MapElements::BeforePoints, *this);

    for (size_t index: mDrawingOrder) {
        mPointStyles[index].draw(rescaled_surface, layout[index]);
    }
    mLabels.draw(rescaled_surface, layout, mPointStyles);

    mMapElements.draw(rescaled_surface, MapElements::AfterPoints, *this);

} // ChartDraw::draw

// ----------------------------------------------------------------------

void ChartDraw::draw(std::string aFilename, double aSize)
{
    PdfCairo surface(aFilename, aSize, aSize);
    draw(surface);

} // ChartDraw::draw

// ----------------------------------------------------------------------

void ChartDraw::modify(IndexGenerator&& aGen, const PointStyle& aStyle, bool aRaise, bool aLower)
{
    for (auto index: aGen)
        modify_point_by_index(index, aStyle, aRaise, aLower);

} // ChartDraw::modify

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

SerumCircle& ChartDraw::serum_circle(size_t aSerumNo, Scaled aRadius)
{
    auto& serum_circle = dynamic_cast<SerumCircle&>(mMapElements.add("serum-circle"));
    serum_circle.serum_no(aSerumNo);
    serum_circle.radius(aRadius);
    return serum_circle;

} // ChartDraw::serum_circle

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
