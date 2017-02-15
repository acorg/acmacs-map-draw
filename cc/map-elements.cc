#include "acmacs-draw/surface.hh"
#include "acmacs-draw/continent-map.hh"
#include "map-elements.hh"

// ----------------------------------------------------------------------

MapElements::MapElements()
{
    operator[]("background-border-grid");

} // MapElements::MapElements

// ----------------------------------------------------------------------

MapElement& MapElements::operator[](std::string aKeyword)
{
    for (const auto& element: mElements) {
        if (element->keyword() == aKeyword)
            return *element;
    }
    return add(aKeyword);

} // MapElements::operator[]

// ----------------------------------------------------------------------

MapElement& MapElements::add(std::string aKeyword)
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
    else {
        throw std::runtime_error("Don't know how to make map element " + aKeyword);
    }
    return *mElements.back();

} // MapElements::add

// ----------------------------------------------------------------------

void MapElements::draw(Surface& aSurface, Order aOrder) const
{
    for (const auto& element: mElements) {
        if (element->order() == aOrder)
            element->draw(aSurface);
    }

} // MapElements::draw

// ----------------------------------------------------------------------

MapElement::~MapElement()
{

} // MapElement::~MapElement

// ----------------------------------------------------------------------

void BackgroundBorderGrid::draw(Surface& aSurface) const
{
    aSurface.background(mBackgroud);
    aSurface.grid(Scaled{1}, mGridColor, mGridLineWidth);
    aSurface.border(mBorderColor, mBorderWidth);

} // BackgroundBorderGrid::draw

// ----------------------------------------------------------------------

void ContinentMap::draw(Surface& aSurface) const
{
    Location origin = mOrigin;
    if (origin.x < 0)
        origin.x += aSurface.width_in_pixels();
    if (origin.y < 0)
        origin.y += aSurface.height_in_pixels();
    Surface& continent_surface = aSurface.subsurface(origin, mWidthInParent, continent_map_size(), true);
    continent_map_draw(continent_surface);

} // ContinentMap::draw

// ----------------------------------------------------------------------

LegendPointLabel::LegendPointLabel()
    : MapElement("legend-point-label", MapElements::AfterPoints), mOrigin{-10, -10}, mWidthInParent(0),
      mBackgroud("white"), mBorderColor("black"), mBorderWidth(0.3), mPointSize(8),
      mLabelColor("black"), mLabelSize(12), mInterline(2.0)
{
} // LegendPointLabel::LegendPointLabel

// ----------------------------------------------------------------------

void LegendPointLabel::draw(Surface& aSurface) const
{
    double width = 0, height = 0;
    for (const auto& line: mLines) {
        const Size line_size = aSurface.text_size(line.label, mLabelSize, mLabelStyle);
        if (line_size.width > width)
            width = line_size.width;
        if (line_size.height > height)
            height = line_size.height;
    }
    const Size padding = aSurface.text_size("O", mLabelSize, mLabelStyle);
    const double scaled_point_size = aSurface.convert(mPointSize).value();

    const double legend_surface_height = height * (mLines.size() - 1) * mInterline + height + padding.height * 2;
    const double legend_surface_width = width + padding.width * 3 + scaled_point_size;
    Location legend_surface_origin{aSurface.convert(Pixels{mOrigin.x}).value(), aSurface.convert(Pixels{mOrigin.y}).value()};
    const Size& surface_size = aSurface.viewport().size;
    if (legend_surface_origin.x < 0)
        legend_surface_origin.x += surface_size.width - legend_surface_width;
    if (legend_surface_origin.y < 0)
        legend_surface_origin.y += surface_size.height - legend_surface_height;

    Surface& legend_surface = aSurface.subsurface(legend_surface_origin, Scaled{legend_surface_width}, Size{legend_surface_width, legend_surface_height}, false);
    legend_surface.background(mBackgroud);
    legend_surface.border(mBorderColor, mBorderWidth);
    const double point_x = padding.width + scaled_point_size / 2;
    const double text_x = padding.width * 2 + scaled_point_size;
    double y = padding.height + height;
    for (const auto& line: mLines) {
        legend_surface.circle_filled({point_x, y - height / 2}, mPointSize, AspectNormal, NoRotation, line.outline, Pixels{1}, line.fill);
        legend_surface.text({text_x, y}, line.label, mLabelColor, mLabelSize, mLabelStyle);
        y += height * mInterline;
    }

} // LegendPointLabel::draw

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
