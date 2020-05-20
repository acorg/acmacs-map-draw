#include "acmacs-draw/draw-legend.hh"
#include "acmacs-map-draw/map-elements.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

map_elements::Elements::Elements()
{
    add_basic_elements_v1();

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

map_elements::Element& map_elements::Elements::add(std::string_view aKeyword)
{
    using namespace std::string_view_literals;
    if (add_v1(aKeyword))
        return *mElements.back();
    // else if (aKeyword == "background-border-grid"sv)
    //     return add<BackgroundBorderGrid>();
    else if (aKeyword == "title"sv)
        return add<Title>();
    else if (aKeyword == "serum-circle"sv)
        return add<SerumCircle>();
    else
        throw std::runtime_error{fmt::format("Don't know how to make map element {}", aKeyword)};

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

acmacs::PointCoordinates map_elements::Element::subsurface_origin(acmacs::surface::Surface& aSurface, const acmacs::PointCoordinates& aPixelOrigin, const acmacs::Size& aScaledSubsurfaceSize) const
{
    acmacs::PointCoordinates subsurface_origin{aSurface.convert(Pixels{aPixelOrigin.x()}).value(), aSurface.convert(Pixels{aPixelOrigin.y()}).value()};
    const acmacs::Size& surface_size = aSurface.viewport().size;
    if (subsurface_origin.x() < 0)
        subsurface_origin.x(subsurface_origin.x() + surface_size.width - aScaledSubsurfaceSize.width);
    if (subsurface_origin.y() < 0)
        subsurface_origin.y(subsurface_origin.y() + surface_size.height - aScaledSubsurfaceSize.height);
    return subsurface_origin;

} // map_elements::Element::subsurface_origin

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
        std::cerr << "ERROR: map_elements::Title::draw(Surface&): " << err.what() << " (ignored)\n";
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
        const auto& coord = transformed_layout->at(mSerumNo + aChartDraw.number_of_antigens());
        if (coord.exists()) {
            if (mStart == mEnd) {
                aSurface.circle_filled(coord, mRadius * 2.0, AspectNormal, NoRotation, mOutlineColor, mOutlineWidth, mOutlineDash, mFillColor);
            }
            else {
                aSurface.sector_filled(coord, mRadius * 2.0, mStart, mEnd, mOutlineColor, mOutlineWidth, mRadiusColor, mRadiusWidth, mRadiusDash, mFillColor);
            }
        }
        else
            std::cerr << ">> SerumCircle::draw(surface): cannot draw serum circle, center coordinates: " << coord << '\n';
    }

} // map_elements::SerumCircle::draw

// ----------------------------------------------------------------------

void map_elements::SerumCircle::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const
{
    if (const auto& coord = aChartDraw.layout()->at(mSerumNo + aChartDraw.number_of_antigens()); coord.exists())
        aDrawElements.serum_circle(coord, aChartDraw.transformation(), mRadius * 2.0, mFillColor, mOutlineColor, mOutlineWidth, mOutlineDash, mRadiusColor, mRadiusWidth, mRadiusDash, mStart, mEnd);
    else
        std::cerr << ">> SerumCircle::draw(draw_elements): cannot draw serum circle, center coordinates: " << coord << '\n';

} // map_elements::SerumCircle::draw


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
