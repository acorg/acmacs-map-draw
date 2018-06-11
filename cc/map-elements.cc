#include "acmacs-draw/continent-map.hh"
#include "acmacs-draw/draw-elements.hh"
#include "acmacs-draw/draw-legend.hh"
#include "acmacs-draw/draw-arrow.hh"
#include "acmacs-map-draw/map-elements.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

map_elements::Elements::Elements()
{
    operator[]("background-border-grid");

} // map_elements::Elements::Elements

// ----------------------------------------------------------------------

bool map_elements::Elements::exists(std::string aKeyword) const
{
    return std::find_if(mElements.begin(), mElements.end(), [&aKeyword](const auto& element) { return element->keyword() == aKeyword; }) != mElements.end();

} // map_elements::Elements::exists

// ----------------------------------------------------------------------

map_elements::Element& map_elements::Elements::operator[](std::string aKeyword)
{
    if (auto found = std::find_if(mElements.begin(), mElements.end(), [&aKeyword](const auto& element) { return element->keyword() == aKeyword; }); found != mElements.end())
        return **found;
    else
        return add(aKeyword);

} // map_elements::Elements::operator[]

// ----------------------------------------------------------------------

map_elements::Element& map_elements::Elements::add(std::string aKeyword)
{
    if (aKeyword == "background-border-grid") {
        mElements.emplace_back(new BackgroundBorderGrid{});
    }
    else if (aKeyword == "continent-map") {
        mElements.emplace_back(new ContinentMap{});
    }
    else if (aKeyword == "legend-point-label") {
        mElements.emplace_back(new LegendPointLabel{});
    }
    else if (aKeyword == "title") {
        mElements.emplace_back(new Title{});
    }
    else if (aKeyword == "serum-circle") {
        mElements.emplace_back(new SerumCircle{});
    }
    else if (aKeyword == "line" || aKeyword == "line_from_to") {
        mElements.emplace_back(new LineFromTo{});
    }
    else if (aKeyword == "line_slope") {
        mElements.emplace_back(new LineSlope{});
    }
    else if (aKeyword == "arrow") {
        mElements.emplace_back(new Arrow{});
    }
    else if (aKeyword == "point") {
        mElements.emplace_back(new Point{});
    }
    else if (aKeyword == "rectangle") {
        mElements.emplace_back(new Rectangle{});
    }
    else if (aKeyword == "circle") {
        mElements.emplace_back(new Circle{});
    }
    else {
        throw std::runtime_error("Don't know how to make map element " + aKeyword);
    }
    return *mElements.back();

} // map_elements::Elements::add

// ----------------------------------------------------------------------

void map_elements::Elements::remove(std::string aKeyword)
{
    mElements.erase(std::remove_if(mElements.begin(), mElements.end(), [&aKeyword](const auto& e) { return e->keyword() == aKeyword; }), mElements.end());

} // map_elements::Elements::remove

// ----------------------------------------------------------------------

// obsolete
void map_elements::Elements::draw(acmacs::surface::Surface& aSurface, Order aOrder, const ChartDraw& aChartDraw) const
{
      // obsolete
    for (const auto& element: mElements) {
        if (element->order() == aOrder)
            element->draw(aSurface, aChartDraw);
    }

} // map_elements::Elements::draw

// ----------------------------------------------------------------------

void map_elements::Elements::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const
{
    for (const auto& element: mElements)
        element->draw(aDrawElements, aChartDraw);

} // map_elements::Elements::draw

// ----------------------------------------------------------------------

map_elements::Element::~Element()
{

} // Element::~Element

// ----------------------------------------------------------------------

acmacs::Location map_elements::Element::subsurface_origin(acmacs::surface::Surface& aSurface, const acmacs::Location& aPixelOrigin, const acmacs::Size& aScaledSubsurfaceSize) const
{
    acmacs::Location subsurface_origin{aSurface.convert(Pixels{aPixelOrigin.x}).value(), aSurface.convert(Pixels{aPixelOrigin.y}).value()};
    const acmacs::Size& surface_size = aSurface.viewport().size;
    if (subsurface_origin.x < 0)
        subsurface_origin.x += surface_size.width - aScaledSubsurfaceSize.width;
    if (subsurface_origin.y < 0)
        subsurface_origin.y += surface_size.height - aScaledSubsurfaceSize.height;
    return subsurface_origin;

} // map_elements::Element::subsurface_origin

// ----------------------------------------------------------------------

// obsolete
void map_elements::BackgroundBorderGrid::draw(acmacs::surface::Surface& aSurface, const ChartDraw&) const
{
    const auto& v = aSurface.viewport();
    aSurface.rectangle_filled(v.origin, v.size, mBackground, Pixels{0}, mBackground);
    aSurface.grid(Scaled{1}, mGridColor, mGridLineWidth);
    aSurface.rectangle(v.origin, v.size, mBorderColor, mBorderWidth);

} // map_elements::BackgroundBorderGrid::draw

// ----------------------------------------------------------------------

void map_elements::BackgroundBorderGrid::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw&) const
{
    aDrawElements.border(mBorderColor, mBorderWidth);
    aDrawElements.background(mBackground);
    aDrawElements.grid(Scaled{1}, mGridColor, mGridLineWidth);

} // map_elements::BackgroundBorderGrid::draw

// ----------------------------------------------------------------------

// obsolete
void map_elements::ContinentMap::draw(acmacs::surface::Surface& aSurface, const ChartDraw&) const
{
    acmacs::Location origin = mOrigin;
    if (origin.x < 0)
        origin.x += aSurface.width_in_pixels() - mWidthInParent.value();
    if (origin.y < 0)
        origin.y += aSurface.height_in_pixels() - mWidthInParent.value() / continent_map_aspect();
    acmacs::surface::Surface& continent_surface = aSurface.subsurface(origin, mWidthInParent, continent_map_size(), true);
    continent_map_draw(continent_surface);

} // map_elements::ContinentMap::draw

// ----------------------------------------------------------------------

void map_elements::ContinentMap::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw&) const
{
    aDrawElements.continent_map(mOrigin, mWidthInParent);

} // map_elements::ContinentMap::draw

// ----------------------------------------------------------------------

map_elements::LegendPointLabel::LegendPointLabel()
    : Element("legend-point-label", Elements::AfterPoints), mOrigin{-10, -10},
      mBackground("white"), mBorderColor(BLACK), mBorderWidth(0.3), mPointSize(8),
      mLabelColor(BLACK), mLabelSize(12), mInterline(2.0)
{
} // map_elements::LegendPointLabel::LegendPointLabel

// ----------------------------------------------------------------------

// obsolete
void map_elements::LegendPointLabel::draw(acmacs::surface::Surface& aSurface, const ChartDraw&) const
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
                                       height * (mLines.size() - 1) * mInterline + height + padding.height * 2};
        const acmacs::Location legend_surface_origin = subsurface_origin(aSurface, mOrigin, legend_surface_size);

        acmacs::surface::Surface& legend_surface = aSurface.subsurface(legend_surface_origin, Scaled{legend_surface_size.width}, legend_surface_size, false);
        const auto& legend_v = legend_surface.viewport();
        legend_surface.rectangle_filled(legend_v.origin, legend_v.size, mBackground, Pixels{0}, mBackground);
        legend_surface.rectangle(legend_v.origin, legend_v.size, mBorderColor, mBorderWidth);
        const double point_x = padding.width + scaled_point_size / 2;
        const double text_x = padding.width * 2 + scaled_point_size;
        double y = padding.height + height;
        for (const auto& line: mLines) {
            legend_surface.circle_filled({point_x, y - height / 2}, mPointSize, AspectNormal, NoRotation, line.outline, Pixels{1}, line.fill);
            legend_surface.text({text_x, y}, line.label, mLabelColor, mLabelSize, mLabelStyle);
            y += height * mInterline;
        }
    }

} // map_elements::LegendPointLabel::draw

// ----------------------------------------------------------------------

void map_elements::LegendPointLabel::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw&) const
{
    auto& legend = aDrawElements.legend();
    legend.origin(mOrigin)
            .background(mBackground)
            .border_color(mBorderColor)
            .border_width(mBorderWidth);
    legend.interline(mInterline);
    for (const auto& line : mLines)
        legend.add(line.label, mLabelColor, mLabelSize, mLabelStyle, mPointSize, line.outline, line.fill);

} // map_elements::LegendPointLabel::draw

// ----------------------------------------------------------------------

map_elements::Title::Title()
    : Element("title", Elements::AfterPoints), mOrigin{10, 10}, mPadding{10},
      mBackground(GREY97), mBorderColor(BLACK), mBorderWidth(0.1),
      mTextColor(BLACK), mTextSize(12), mInterline(2.0)
{
} // map_elements::Title::Title

// ----------------------------------------------------------------------

// obsolete
void map_elements::Title::draw(acmacs::surface::Surface& aSurface) const
{
      // obsolete
    if (mShow && (mLines.size() > 1 || (!mLines.empty() && !mLines.front().empty()))) {
        double width = 0, height = 0;
        for (const auto& line: mLines) {
            const acmacs::Size line_size = aSurface.text_size(line, mTextSize, mTextStyle);
            if (line_size.width > width)
                width = line_size.width;
            if (line_size.height > height)
                height = line_size.height;
        }

        const double padding = aSurface.convert(mPadding).value();
        const acmacs::Size legend_surface_size{width + padding * 2,
                    height * (mLines.size() - 1) * mInterline + height + padding * 2};
        const acmacs::Location legend_surface_origin = subsurface_origin(aSurface, mOrigin, legend_surface_size);

        acmacs::surface::Surface& legend_surface = aSurface.subsurface(legend_surface_origin, Scaled{legend_surface_size.width}, legend_surface_size, false);
        const auto& legend_v = legend_surface.viewport();
        legend_surface.rectangle_filled(legend_v.origin, legend_v.size, mBackground, Pixels{0}, mBackground);
        legend_surface.rectangle(legend_v.origin, legend_v.size, mBorderColor, mBorderWidth);
        const double text_x = padding;
        double y = padding + height;
        for (const auto& line: mLines) {
            legend_surface.text({text_x, y}, line, mTextColor, mTextSize, mTextStyle);
            y += height * mInterline;
        }
    }

} // map_elements::Title::draw

// ----------------------------------------------------------------------

void map_elements::Title::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw&) const
{
    if (mShow && (mLines.size() > 1 || (!mLines.empty() && !mLines.front().empty()))) {
        aDrawElements.title(mLines)
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

} // map_elements::Title::draw

// ----------------------------------------------------------------------

// obsolete
void map_elements::SerumCircle::draw(acmacs::surface::Surface& aSurface, const ChartDraw& aChartDraw) const
{
    if (mSerumNo != static_cast<size_t>(-1)) {
        auto transformed_layout = aChartDraw.transformed_layout();
        const auto& coord = transformed_layout->get(mSerumNo + aChartDraw.number_of_antigens());
        if (mStart == mEnd) {
            aSurface.circle_filled(coord, mRadius * 2.0, AspectNormal, NoRotation, mOutlineColor, mOutlineWidth, mFillColor);
        }
        else {
            aSurface.sector_filled(coord, mRadius * 2.0, mStart, mEnd, mOutlineColor, mOutlineWidth, mRadiusColor, mRadiusWidth, mRadiusDash, mFillColor);
        }
    }

} // map_elements::SerumCircle::draw

// ----------------------------------------------------------------------

void map_elements::SerumCircle::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const
{
    aDrawElements.serum_circle(aChartDraw.layout()->get(mSerumNo + aChartDraw.number_of_antigens()), aChartDraw.transformation(), mRadius * 2.0, mFillColor, mOutlineColor, mOutlineWidth, mRadiusColor, mRadiusWidth, mRadiusDash, mStart, mEnd);

} // map_elements::SerumCircle::draw

// ----------------------------------------------------------------------

// obsolete
void map_elements::LineFromTo::draw(acmacs::surface::Surface& aSurface, const ChartDraw& /*aChartDraw*/) const
{
    aSurface.line(mBegin, mEnd, mLineColor, mLineWidth);

} // map_elements::LineFromTo::draw

// ----------------------------------------------------------------------

void map_elements::LineFromTo::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw&) const
{
    aDrawElements.line(mBegin, mEnd, mLineColor, mLineWidth);

} // map_elements::LineFromTo::draw

// ----------------------------------------------------------------------

void map_elements::LineSlope::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const
{
    const auto& viewport = aChartDraw.viewport();
    aDrawElements.line({viewport.left(), viewport.left() * slope_ + intercept_}, {viewport.right(), viewport.right() * slope_ + intercept_}, mLineColor, mLineWidth);

} // map_elements::Line::draw

// ----------------------------------------------------------------------

// obsolete
void map_elements::Rectangle::draw(acmacs::surface::Surface& aSurface, const ChartDraw& /*aChartDraw*/) const
{
    const std::vector<acmacs::Location> path{mCorner1, {mCorner1.x, mCorner2.y}, mCorner2, {mCorner2.x, mCorner1.y}};
    if (mFilled)
        aSurface.path_fill(path.begin(), path.end(), mColor);
    else
        aSurface.path_outline(path.begin(), path.end(), mColor, mLineWidth, true);

} // map_elements::Rectangle::draw

// ----------------------------------------------------------------------

void map_elements::Rectangle::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw&) const
{
    aDrawElements.rectangle(mCorner1, mCorner2, mColor, mFilled, mLineWidth);

} // map_elements::Rectangle::draw

// ----------------------------------------------------------------------

// obsolete
void map_elements::Arrow::draw(acmacs::surface::Surface& aSurface, const ChartDraw& /*aChartDraw*/) const
{
    const bool x_eq = float_equal(mEnd.x, mBegin.x);
    const double sign2 = x_eq ? (mBegin.y < mEnd.y ? 1.0 : -1.0) : (mEnd.x < mBegin.x ? 1.0 : -1.0);
    const double angle = x_eq ? -M_PI_2 : std::atan((mEnd.y - mBegin.y) / (mEnd.x - mBegin.x));
    const auto end = aSurface.arrow_head(mEnd, angle, sign2, mArrowHeadColor, mArrowWidth, mArrowHeadFilled);
      // std::cerr << "DEBUG: Arrow " << mBegin << ' ' << mEnd << ' ' << end << " angle:" << angle << " sign2:" << sign2 << ' ' << mArrowHeadColor << '\n';
    aSurface.line(mBegin, end, mLineColor, mLineWidth);

} // map_elements::Arrow::draw

// ----------------------------------------------------------------------

void map_elements::Arrow::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw&) const
{
    aDrawElements.arrow(mBegin, mEnd, mLineColor, mLineWidth, mArrowHeadColor, mArrowHeadFilled, mArrowWidth);

} // map_elements::Arrow::draw

// ----------------------------------------------------------------------

// obsolete
void map_elements::Point::draw(acmacs::surface::Surface& aSurface, const ChartDraw& /*aChartDraw*/) const
{
    aSurface.circle_filled(mCenter, mSize, mAspect, mRotation, mOutlineColor, mOutlineWidth, mFillColor);

} // map_elements::Point::draw

// ----------------------------------------------------------------------

void map_elements::Point::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw&) const
{
    aDrawElements.point(mCenter, mSize, mFillColor, mOutlineColor, mOutlineWidth, mAspect, mRotation);

} // map_elements::Point::draw

// ----------------------------------------------------------------------

// obsolete
void map_elements::Circle::draw(acmacs::surface::Surface& aSurface, const ChartDraw& /*aChartDraw*/) const
{
    aSurface.circle_filled(mCenter, mSize, mAspect, mRotation, mOutlineColor, mOutlineWidth, mFillColor);

} // map_elements::Circle::draw

// ----------------------------------------------------------------------

void map_elements::Circle::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw&) const
{
    aDrawElements.circle(mCenter, mSize, mFillColor, mOutlineColor, mOutlineWidth, mAspect, mRotation);

} // map_elements::Circle::draw

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
