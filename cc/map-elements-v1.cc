#include "acmacs-draw/continent-map.hh"
#include "acmacs-draw/draw-legend.hh"
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
    else if (aKeyword == "continent-map"sv)
        add<ContinentMap>();
    else if (aKeyword == "legend-point-label"sv)
        add<LegendPointLabel>();
    else
        return false;

    return true;

} // map_elements::Elements::add_v1

// ----------------------------------------------------------------------

void map_elements::Elements::add_basic_elements_v1()
{
    add<map_elements::v1::BackgroundBorderGrid>();

} // map_elements::Elements::add_basic_elements_v1

// ----------------------------------------------------------------------

// obsolete
void map_elements::v1::BackgroundBorderGrid::draw(acmacs::surface::Surface& aSurface, const ChartDraw&) const
{
    const auto& v = aSurface.viewport();
    aSurface.rectangle_filled(v.origin, v.size, mBackground, Pixels{0}, mBackground);
    aSurface.grid(Scaled{1}, mGridColor, mGridLineWidth);
    aSurface.rectangle(v.origin, v.size, mBorderColor, mBorderWidth);

} // map_elements::v1::BackgroundBorderGrid::draw

// ----------------------------------------------------------------------

void map_elements::v1::BackgroundBorderGrid::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw&) const
{
    aDrawElements.border(mBorderColor, mBorderWidth);
    aDrawElements.background(mBackground);
    aDrawElements.grid(Scaled{1}, mGridColor, mGridLineWidth);

} // map_elements::v1::BackgroundBorderGrid::draw

// ----------------------------------------------------------------------

// obsolete
void map_elements::v1::ContinentMap::draw(acmacs::surface::Surface& aSurface, const ChartDraw&) const
{
    acmacs::PointCoordinates origin = mOrigin;
    if (origin.x() < 0)
        origin.x(origin.x() + aSurface.width_in_pixels() - mWidthInParent.value());
    if (origin.y() < 0)
        origin.y(origin.y() + aSurface.height_in_pixels() - mWidthInParent.value() / continent_map_aspect());
    acmacs::surface::Surface& continent_surface = aSurface.subsurface(origin, mWidthInParent, continent_map_size(), true);
    continent_map_draw(continent_surface);

} // map_elements::ContinentMap::draw

// ----------------------------------------------------------------------

void map_elements::v1::ContinentMap::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw&) const
{
    aDrawElements.continent_map(mOrigin, mWidthInParent);

} // map_elements::v1::ContinentMap::draw

// ----------------------------------------------------------------------

map_elements::v1::LegendPointLabel::LegendPointLabel()
    : Element("legend-point-label", Elements::AfterPoints), mOrigin{-10, -10},
      mBackground("white"), mBorderColor(BLACK), mBorderWidth(0.3), mPointSize(8),
      mLabelColor(BLACK), mLabelSize(12), mInterline(2.0)
{
} // map_elements::v1::LegendPointLabel::LegendPointLabel

// ----------------------------------------------------------------------

// obsolete
void map_elements::v1::LegendPointLabel::draw(acmacs::surface::Surface& aSurface, const ChartDraw&) const
{
    if (!mLines.empty()) {
        double width = 0, height = 0;
        for (const auto& line: mLines) {
            const acmacs::Size line_size = aSurface.text_size(line.label, mLabelSize, mLabelStyle);
            if (line_size.width > width)
                width = line_size.width;
            if (line_size.height > height)
                height = line_size.height;
        }
        const acmacs::Size padding = aSurface.text_size("O", mLabelSize, mLabelStyle);
        const double scaled_point_size = aSurface.convert(mPointSize).value();

        const acmacs::Size legend_surface_size{width + padding.width * 3 + scaled_point_size,
                                       height * static_cast<double>(mLines.size() - 1) * mInterline + height + padding.height * 2};
        const acmacs::PointCoordinates legend_surface_origin = subsurface_origin(aSurface, mOrigin, legend_surface_size);

        acmacs::surface::Surface& legend_surface = aSurface.subsurface(legend_surface_origin, Scaled{legend_surface_size.width}, legend_surface_size, false);
        const auto& legend_v = legend_surface.viewport();
        legend_surface.rectangle_filled(legend_v.origin, legend_v.size, mBackground, Pixels{0}, mBackground);
        legend_surface.rectangle(legend_v.origin, legend_v.size, mBorderColor, mBorderWidth);
        const double point_x = padding.width + scaled_point_size / 2;
        const double text_x = padding.width * 2 + scaled_point_size;
        double y = padding.height + height;
        for (const auto& line: mLines) {
            legend_surface.circle_filled({point_x, y - height / 2}, mPointSize, AspectNormal, NoRotation, line.outline, Pixels{1}, acmacs::surface::Dash::NoDash, line.fill);
            legend_surface.text({text_x, y}, line.label, mLabelColor, mLabelSize, mLabelStyle);
            y += height * mInterline;
        }
    }

} // map_elements::v1::LegendPointLabel::draw

// ----------------------------------------------------------------------

void map_elements::v1::LegendPointLabel::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw&) const
{
    auto& legend = aDrawElements.legend();
    legend.origin(mOrigin)
            .background(mBackground)
            .border_color(mBorderColor)
            .border_width(mBorderWidth);
    legend.interline(mInterline);
    for (const auto& line : mLines)
        legend.add(line.label, mLabelColor, mLabelSize, mLabelStyle, mPointSize, line.outline, line.fill);

} // map_elements::v1::LegendPointLabel::draw

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
