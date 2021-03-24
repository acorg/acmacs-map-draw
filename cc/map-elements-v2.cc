#include "acmacs-map-draw/map-elements-v2.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

void map_elements::v2::Circle::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& chart_draw) const
{
    aDrawElements.circle(center_.get_transformed(chart_draw), radius_ * 2.0, fill_, outline_, outline_width_, aspect_, rotation_);

} // map_elements::v2::Circle::draw

// ----------------------------------------------------------------------

void map_elements::v2::Path::draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& chart_draw) const
{
    if (data().vertices.size() < 2) {
        AD_WARNING("too few vertices in path");
        return;
    }

    chart_draw.viewport_reset_used_by(); // to avoid warning when viewport is later changed (e.g. set)
    aDrawElements.add<acmacs::draw::PathWithArrows>(data().vertices, data().close, fill_, outline_, outline_width_, arrows_);

} // map_elements::v2::Path::draw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
