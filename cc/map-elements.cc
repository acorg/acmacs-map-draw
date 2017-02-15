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
    const Scaled point_size = aSurface.convert(mPointSize);

    std::cerr << "LegendPointLabel width:" << width << " height:" << height << " lines:" << mLines.size() << " this " << this << std::endl;

    const double area_height = height * (mLines.size() - 1) * mInterline + height + padding.height * 2;
    const double area_width = width + padding.width * 3 + point_size.value();
      // const double aspect = height / width;
    Surface& legend_surface = aSurface.subsurface({1, 1}, Scaled{area_width}, Size{area_width, area_height}, false);
    legend_surface.background(mBackgroud);
    legend_surface.border(mBorderColor, mBorderWidth);
    std::cerr << "legend_surface " << legend_surface << std::endl;
    double x = padding.width * 2 + point_size.value(), y = padding.height + height;
    for (const auto& line: mLines) {
        legend_surface.circle_filled({padding.width, y - height / 2}, mPointSize, Aspect{1.0}, NoRotation, line.outline, Pixels{1}, line.fill);
          // legend_surface.circle({padding.width, y}, mPointSize, Aspect{1.0}, NoRotation, line.fill, Pixels{1});
        legend_surface.text({x, y}, line.label, mLabelColor, mLabelSize, mLabelStyle);
        y += height * mInterline;
    }

} // LegendPointLabel::draw

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
