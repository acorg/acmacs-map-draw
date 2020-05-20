#include "acmacs-map-draw/map-elements-v1.hh"
#include "acmacs-draw/draw-elements.hh"

// ----------------------------------------------------------------------

bool map_elements::Elements::add_v1(std::string_view aKeyword)
{
    using namespace std::string_view_literals;
    using namespace map_elements::v1;

    if (aKeyword == "line"sv || aKeyword == "line_from_to"sv)
        add<LineFromTo>();
    else if (aKeyword == "path"sv)
        add<Path>();
    else if (aKeyword == "line_slope"sv)
        add<LineSlope>();
    else if (aKeyword == "arrow"sv)
        add<Arrow>();
    else if (aKeyword == "point"sv)
        add<Point>();
    else if (aKeyword == "rectangle"sv)
        add<Rectangle>();
    else if (aKeyword == "circle"sv)
        add<Circle>();
    else
        return false;

    return true;

} // map_elements::Elements::add_v1

// ----------------------------------------------------------------------

// obsolete
void map_elements::v1::LineFromTo::draw(acmacs::surface::Surface& aSurface, const ChartDraw& /*aChartDraw*/) const
{
    aSurface.line(mBegin, mEnd, mLineColor, mLineWidth);

} // map_elements::v1::LineFromTo::draw

// ----------------------------------------------------------------------

void map_elements::v1::LineFromTo::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw&) const
{
    aDrawElements.line(mBegin, mEnd, mLineColor, mLineWidth);

} // map_elements::v1::LineFromTo::draw

// ----------------------------------------------------------------------

void map_elements::v1::LineSlope::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw&) const
{
    aDrawElements.line(line_, mLineColor, mLineWidth, apply_map_transformation_);

} // map_elements::v1::Line::draw

// ----------------------------------------------------------------------

// obsolete
void map_elements::v1::Rectangle::draw(acmacs::surface::Surface& aSurface, const ChartDraw& /*aChartDraw*/) const
{
    const std::vector<acmacs::PointCoordinates> path{mCorner1, {mCorner1.x(), mCorner2.y()}, mCorner2, {mCorner2.x(), mCorner1.y()}};
    if (mFilled)
        aSurface.path_fill(path.begin(), path.end(), mColor);
    else
        aSurface.path_outline(path.begin(), path.end(), mColor, mLineWidth, true);

} // map_elements::v1::Rectangle::draw

// ----------------------------------------------------------------------

void map_elements::v1::Rectangle::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw&) const
{
    aDrawElements.rectangle(mCorner1, mCorner2, mColor, mFilled, mLineWidth);

} // map_elements::v1::Rectangle::draw

// ----------------------------------------------------------------------

// obsolete
void map_elements::v1::Arrow::draw(acmacs::surface::Surface& aSurface, const ChartDraw& /*aChartDraw*/) const
{
    const bool x_eq = float_equal(mEnd.x(), mBegin.x());
    const double sign2 = x_eq ? (mBegin.y() < mEnd.y() ? 1.0 : -1.0) : (mEnd.x() < mBegin.x() ? 1.0 : -1.0);
    const double angle = x_eq ? -M_PI_2 : std::atan((mEnd.y() - mBegin.y()) / (mEnd.x() - mBegin.x()));
    const auto end = aSurface.arrow_head(mEnd, angle, sign2, mArrowHeadColor, mArrowWidth, mArrowHeadFilled);
      // std::cerr << "DEBUG: Arrow " << mBegin << ' ' << mEnd << ' ' << end << " angle:" << angle << " sign2:" << sign2 << ' ' << mArrowHeadColor << '\n';
    aSurface.line(mBegin, end, mLineColor, mLineWidth);

} // map_elements::v1::Arrow::draw

// ----------------------------------------------------------------------

void map_elements::v1::Arrow::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw&) const
{
    aDrawElements.arrow(mBegin, mEnd, mLineColor, mLineWidth, mArrowHeadColor, mArrowHeadFilled, mArrowWidth);

} // map_elements::v1::Arrow::draw

// ----------------------------------------------------------------------

// obsolete
void map_elements::v1::Circle::draw(acmacs::surface::Surface& aSurface, const ChartDraw& /*aChartDraw*/) const
{
    aSurface.circle_filled(mCenter, mSize, mAspect, mRotation, mOutlineColor, mOutlineWidth, acmacs::surface::Dash::NoDash, mFillColor);

} // map_elements::v1::Circle::draw

// ----------------------------------------------------------------------

void map_elements::v1::Circle::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw&) const
{
    aDrawElements.circle(mCenter, mSize, mFillColor, mOutlineColor, mOutlineWidth, mAspect, mRotation);

} // map_elements::v1::Circle::draw

// ----------------------------------------------------------------------

// obsolete
void map_elements::v1::Path::draw(acmacs::surface::Surface& /*aSurface*/, const ChartDraw& /*aChartDraw*/) const
{
    std::cerr << ">> WARNING: map_elements::Path::draw(surface) obsolete and not implemented\n";

} // map_elements::v1::Path::draw


void map_elements::v1::Path::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& /*aChartDraw*/) const
{
    aDrawElements.path(mPath, mLineColor, mLineWidth, close_and_fill_);

} // map_elements::v1::Path::draw

// ----------------------------------------------------------------------

// obsolete
void map_elements::v1::Point::draw(acmacs::surface::Surface& aSurface, const ChartDraw& /*aChartDraw*/) const
{
    aSurface.circle_filled(mCenter, mSize, mAspect, mRotation, mOutlineColor, mOutlineWidth, acmacs::surface::Dash::NoDash, mFillColor);

} // map_elements::v1::Point::draw

// ----------------------------------------------------------------------

void map_elements::v1::Point::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw&) const
{
    aDrawElements.point(mCenter, mSize, mFillColor, mOutlineColor, mOutlineWidth, mAspect, mRotation, mLabel);

} // map_elements::v1::Point::draw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
