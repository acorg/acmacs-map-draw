#include "acmacs-base/rjson-v3.hh"
#include "acmacs-draw/continent-map.hh"
#include "acmacs-draw/draw-legend.hh"
#include "acmacs-draw/draw-elements.hh"
#include "acmacs-map-draw/map-elements-v1.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/mapi-settings.hh"

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
    for (const auto& line : mLines) {
        legend.add(line.label, mLabelColor, mLabelSize, mLabelStyle, mPointSize, line.outline, line.outline_width, line.fill);
    }

} // map_elements::v1::LegendPointLabel::draw

// ----------------------------------------------------------------------

void map_elements::v1::Title::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& chart_draw) const
{
    using namespace std::string_view_literals;
    if (mShow && (mLines.size() > 1 || (!mLines.empty() && !mLines.front().empty()))) {
        // AD_DEBUG("env stress: {}", chart_draw.settings().getenv("stress"sv));
        std::vector<std::string> lines(mLines.size());
        std::transform(std::begin(mLines), std::end(mLines), std::begin(lines), [&chart_draw](const auto& line) -> std::string { return chart_draw.settings().substitute(line); });
        aDrawElements.title(lines)
            .text_color(mTextColor)
            .text_size(mTextSize)
            .text_style(mTextStyle)
            .interline(mInterline)
            .padding(mPadding)
            .origin(mOrigin)
            .background(mBackground)
            .border_color(mBorderColor)
            .border_width(mBorderWidth);
    }

} // map_elements::v1::Title::draw

// ----------------------------------------------------------------------

// !!! obsolete !!!
void map_elements::v1::Title::draw(acmacs::surface::Surface& aSurface) const
{
    // obsolete
    try {
        if (mShow && (mLines.size() > 1 || (!mLines.empty() && !mLines.front().empty()))) {
            double width = 0, height = 0;
            for (const auto& line : mLines) {
                const acmacs::Size line_size = aSurface.text_size(line, mTextSize, mTextStyle);
                if (line_size.width > width)
                    width = line_size.width;
                if (line_size.height > height)
                    height = line_size.height;
            }

            const double padding = aSurface.convert(mPadding).value();
            if (std::isnan(padding))
                throw std::runtime_error("padding is NaN");
            const acmacs::Size legend_surface_size{width + padding * 2, height * static_cast<double>(mLines.size() - 1) * mInterline + height + padding * 2};
            const acmacs::PointCoordinates legend_surface_origin = subsurface_origin(aSurface, mOrigin, legend_surface_size);

            acmacs::surface::Surface& legend_surface = aSurface.subsurface(legend_surface_origin, Scaled{legend_surface_size.width}, legend_surface_size, false);
            const auto& legend_v = legend_surface.viewport();
            legend_surface.rectangle_filled(legend_v.origin, legend_v.size, mBackground, Pixels{0}, mBackground);
            legend_surface.rectangle(legend_v.origin, legend_v.size, mBorderColor, mBorderWidth);
            const double text_x = padding;
            double y = padding + height;
            for (const auto& line : mLines) {
                legend_surface.text({text_x, y}, line, mTextColor, mTextSize, mTextStyle);
                y += height * mInterline;
            }
        }
    }
    catch (std::exception& err) {
        AD_ERROR("map_elements::Title::draw(Surface&): {} (ignored)", err);
    }

} // map_elements::v1::Title::draw

// ----------------------------------------------------------------------

// !!! obsolete !!!
void map_elements::v1::SerumCircle::draw(acmacs::surface::Surface& aSurface, const ChartDraw& aChartDraw) const
{
    if (mSerumNo != static_cast<size_t>(-1)) {
        auto transformed_layout = aChartDraw.chart(0).modified_transformed_layout();
        const auto& coord = transformed_layout->at(mSerumNo + aChartDraw.chart().number_of_antigens());
        if (coord.exists()) {
            if (mStart == mEnd) {
                aSurface.circle_filled(coord, mRadius * 2.0, AspectNormal, NoRotation, mOutlineColor, mOutlineWidth, mOutlineDash, mFillColor);
            }
            else {
                aSurface.sector_filled(coord, mRadius * 2.0, mStart, mEnd, mOutlineColor, mOutlineWidth, mRadiusColor, mRadiusWidth, mRadiusDash, mFillColor);
            }
        }
        else
            AD_WARNING("SerumCircle::draw(surface): cannot draw serum circle, center coordinates: {}", coord);
    }

} // map_elements::v1::SerumCircle::draw

// ----------------------------------------------------------------------

void map_elements::v1::SerumCircle::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const
{
    if (const auto& coord = aChartDraw.chart(0).modified_layout()->at(mSerumNo + aChartDraw.chart().number_of_antigens()); coord.exists())
        aDrawElements.serum_circle(coord, aChartDraw.chart(0).modified_transformation(), mRadius * 2.0, mFillColor, mOutlineColor, mOutlineWidth, mOutlineDash, mRadiusColor, mRadiusWidth, mRadiusDash, mStart, mEnd);
    else
        AD_WARNING("SerumCircle::draw(draw_elements): cannot draw serum circle, center coordinates: {}", coord);

} // map_elements::v1::SerumCircle::draw

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
    AD_WARNING("map_elements::Path::draw(surface) obsolete and not implemented");

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
