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

} // LegendPointLabel::draw

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
