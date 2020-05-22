#include "acmacs-draw/draw-elements.hh"
#include "acmacs-draw/draw-arrow.hh"
#include "acmacs-map-draw/map-elements-v2.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

acmacs::PointCoordinates map_elements::v2::Coordinates::viewport::get(const ChartDraw& chart_draw) const
{
    return chart_draw.viewport().origin + *this;

} // map_elements::v2::Coordinates::viewport::get

// ----------------------------------------------------------------------

acmacs::PointCoordinates map_elements::v2::Coordinates::layout::get(const ChartDraw& /*chart_draw*/) const
{
    return *this;

} // map_elements::v2::Coordinates::layout::get

// ----------------------------------------------------------------------

acmacs::PointCoordinates map_elements::v2::Coordinates::transformed::get(const ChartDraw& chart_draw) const
{
    return chart_draw.transformation().transform(*this);

} // map_elements::v2::Coordinates::transformed::get

// ----------------------------------------------------------------------

acmacs::PointCoordinates map_elements::v2::Coordinates::points::get(const ChartDraw& chart_draw) const
{
    auto layout = chart_draw.transformed_layout();
    acmacs::PointCoordinates coord{layout->number_of_dimensions(), 0.0};
    for (const auto point_index : get())
        coord += layout->at(point_index);
    coord /= static_cast<double>(size());
    return coord;

} // map_elements::v2::Coordinates::points::get

// ----------------------------------------------------------------------

acmacs::PointCoordinates map_elements::v2::Coordinates::get(const ChartDraw& chart_draw) const
{
    return std::visit([&chart_draw](const auto& coord) { return coord.get(chart_draw); }, coordinates);

} // map_elements::v2::Coordinates::get

// ----------------------------------------------------------------------

void map_elements::v2::Circle::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& chart_draw) const
{
    aDrawElements.circle(center_.get(chart_draw), radius_ * 2.0, fill_, outline_, outline_width_, aspect_, rotation_);

} // map_elements::v2::Circle::draw

// ----------------------------------------------------------------------

void map_elements::v2::Path::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& chart_draw) const
{
    if (data().vertices.size() < 2) {
        AD_WARNING("too few vertices in path");
        return;
    }

    std::vector<acmacs::PointCoordinates> path;
    std::transform(std::begin(data().vertices), std::end(data().vertices), std::back_inserter(path), [&chart_draw](const auto& vertex) { return vertex.get(chart_draw); });
    auto& path_element = aDrawElements.add<acmacs::draw::PathWithArrows>(path, data().close, fill_, outline_, outline_width_);

} // map_elements::v2::Path::draw

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
