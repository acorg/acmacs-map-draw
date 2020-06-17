#include "acmacs-map-draw/map-elements.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

map_elements::Elements::Elements()
{
    add_basic_elements_v1();

} // map_elements::Elements::Elements

// ----------------------------------------------------------------------

void map_elements::Elements::reset()
{
    mElements.clear();
    add_basic_elements_v1();

} // map_elements::Elements::reset

// ----------------------------------------------------------------------

bool map_elements::Elements::exists(std::string_view keyword) const
{
    return std::find_if(mElements.begin(), mElements.end(), [keyword](const auto& element) { return element->keyword() == keyword; }) != mElements.end();

} // map_elements::Elements::exists

// ----------------------------------------------------------------------

map_elements::Element* map_elements::Elements::find_base(std::string_view keyword)
{
    if (auto found = std::find_if(mElements.begin(), mElements.end(), [&keyword](const auto& element) { return element->keyword() == keyword; }); found != mElements.end())
        return found->get();
    else
        return nullptr;

} // map_elements::Elements::find_base

// ----------------------------------------------------------------------

void map_elements::Elements::remove(std::string_view keyword)
{
    mElements.erase(std::remove_if(mElements.begin(), mElements.end(), [keyword](const auto& e) { return e->keyword() == keyword; }), mElements.end());

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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
