#include "acmacs-base/range.hh"
#include "acmacs-draw/surface.hh"
#include "acmacs-chart-2/chart-modify.hh"
#include "acmacs-map-draw/labels.hh"
#include "acmacs-map-draw/point-style-draw.hh"

using namespace map_elements;

// ----------------------------------------------------------------------

void Labels::draw(const acmacs::draw::PointLabel& label, acmacs::surface::Surface& aSurface, const acmacs::Layout& aLayout, const acmacs::chart::PlotSpecModify& aPlotSpec) const
{
      // obsolete
    const auto style = aPlotSpec.style(label.index());
    if (style.shown) {
        auto text_origin = aLayout[label.index()].copy();
        if (text_origin.exists()) { // point is not disconnected
            const double scaled_point_size = aSurface.convert(Pixels{style.size}).value();
            const acmacs::Size ts = aSurface.text_size(label.display_name(), label.text_size(), label.text_style());
            text_origin[acmacs::number_of_dimensions_t{0}] += label.text_offset(label.offset().x(), scaled_point_size, ts.width, false);
            text_origin[acmacs::number_of_dimensions_t{1}] += label.text_offset(label.offset().y(), scaled_point_size, ts.height, true);
            aSurface.text(text_origin, label.display_name(), label.text_color(), label.text_size(), label.text_style());
        }
    }

} // Labels::draw

// ----------------------------------------------------------------------

acmacs::draw::PointLabel& Labels::add(size_t aIndex, const acmacs::chart::Chart& aChart)
{
    acmacs::draw::PointLabel& added = add(aIndex);
    const auto number_of_antigens = aChart.number_of_antigens();
    if (aIndex < number_of_antigens)
        added.display_name(aChart.antigen(aIndex)->full_name());
    else
        added.display_name(aChart.serum(aIndex - number_of_antigens)->full_name());
    return added;

} // Labels::add

// ----------------------------------------------------------------------

void Labels::add_all(const acmacs::chart::Chart& aChart)
{
    for (auto point_no : acmacs::range(aChart.number_of_points()))
        add(point_no, aChart);

} // Labels::add_all

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
