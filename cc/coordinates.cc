#include "acmacs-map-draw/coordinates.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

acmacs::PointCoordinates map_elements::v2::Coordinates::viewport::get_transformed(const ChartDraw& chart_draw) const
{
    chart_draw.calculate_viewport();
    const auto coord = chart_draw.viewport("map_elements::v2::Coordinates::viewport::get_transformed").origin + *this;
    chart_draw.viewport_reset_used_by();
    return coord;

} // map_elements::v2::Coordinates::viewport::get_transformed

// ----------------------------------------------------------------------

acmacs::PointCoordinates map_elements::v2::Coordinates::not_transformed::get_transformed(const ChartDraw& /*chart_draw*/) const
{
    return *this;

} // map_elements::v2::Coordinates::not_transformed::get_transformed

// ----------------------------------------------------------------------

acmacs::PointCoordinates map_elements::v2::Coordinates::transformed::get_transformed(const ChartDraw& chart_draw) const
{
    return chart_draw.chart(0).modified_transformation().transform(*this);

} // map_elements::v2::Coordinates::transformed::get_transformed

// ----------------------------------------------------------------------

acmacs::PointCoordinates map_elements::v2::Coordinates::points::get_transformed(const ChartDraw& chart_draw) const
{
    auto layout = chart_draw.chart(0).modified_transformed_layout();
    acmacs::PointCoordinates coord{layout->number_of_dimensions(), 0.0};
    for (const auto point_index : get())
        coord += layout->at(point_index);
    coord /= static_cast<double>(size());
    return coord;

} // map_elements::v2::Coordinates::points::get_transformed

// ----------------------------------------------------------------------

acmacs::PointCoordinates map_elements::v2::Coordinates::get_transformed(const ChartDraw& chart_draw) const
{
    return std::visit([&chart_draw](const auto& coord) { return coord.get_transformed(chart_draw); }, coordinates);

} // map_elements::v2::Coordinates::get_transformed

// ======================================================================

acmacs::PointCoordinates map_elements::v2::Coordinates::viewport::get_not_transformed(const ChartDraw& chart_draw) const
{
    return chart_draw.chart(0).modified_inverted_transformation().transform(get_transformed(chart_draw));

} // map_elements::v2::Coordinates::viewport::get_not_transformed

// ----------------------------------------------------------------------

acmacs::PointCoordinates map_elements::v2::Coordinates::not_transformed::get_not_transformed(const ChartDraw& chart_draw) const
{
    return chart_draw.chart(0).modified_inverted_transformation().transform(*this);

} // map_elements::v2::Coordinates::not_transformed::get_not_transformed

// ----------------------------------------------------------------------

acmacs::PointCoordinates map_elements::v2::Coordinates::transformed::get_not_transformed(const ChartDraw& /*chart_draw*/) const
{
    return *this;

} // map_elements::v2::Coordinates::transformed::get_not_transformed

// ----------------------------------------------------------------------

acmacs::PointCoordinates map_elements::v2::Coordinates::points::get_not_transformed(const ChartDraw& chart_draw) const
{
    auto layout = chart_draw.chart(0).modified_layout();
    acmacs::PointCoordinates coord{layout->number_of_dimensions(), 0.0};
    for (const auto point_index : get())
        coord += layout->at(point_index);
    coord /= static_cast<double>(size());
    return coord;

} // map_elements::v2::Coordinates::points::get_not_transformed

// ----------------------------------------------------------------------

acmacs::PointCoordinates map_elements::v2::Coordinates::get_not_transformed(const ChartDraw& chart_draw) const
{
    return std::visit([&chart_draw](const auto& coord) { return coord.get_not_transformed(chart_draw); }, coordinates);

} // map_elements::v2::Coordinates::get_not_transformed


// ----------------------------------------------------------------------
