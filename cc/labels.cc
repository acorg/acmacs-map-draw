#include "acmacs-draw/surface.hh"
#include "acmacs-chart-2/chart-modify.hh"
#include "acmacs-map-draw/labels.hh"
#include "acmacs-map-draw/point-style-draw.hh"

using namespace map_elements;

// ----------------------------------------------------------------------

void Label::draw(acmacs::surface::Surface& aSurface, const acmacs::chart::Layout& aLayout, const acmacs::chart::PlotSpecModify& aPlotSpec) const
{
      // obsolete
    const auto style = aPlotSpec.style(index());
    if (*style.shown) {
        auto text_origin = aLayout[index()];
        if (!text_origin.empty()) { // point is not disconnected
            const double scaled_point_size = aSurface.convert(Pixels{*style.size}).value();
            const acmacs::Size ts = aSurface.text_size(display_name(), text_size(), text_style());
            text_origin[0] += text_offset(offset().x, scaled_point_size, ts.width, false);
            text_origin[1] += text_offset(offset().y, scaled_point_size, ts.height, true);
            aSurface.text(text_origin, display_name(), text_color(), text_size(), text_style());
        }
    }

} // Label::draw

// ----------------------------------------------------------------------

double Label::text_offset(double offset_hint, double point_size, double text_size, bool text_origin_at_opposite) const
{
    double offset = 0;
    if (offset_hint < -1) {
        offset += point_size * (offset_hint + 0.5) - (text_origin_at_opposite ? 0 : text_size);
    }
    else if (offset_hint < 1) {
        offset += point_size * offset_hint / 2 + (text_origin_at_opposite ? (text_size * (offset_hint + 1) / 2) : (text_size * (offset_hint - 1) / 2));
    }
    else {
        offset += point_size * (offset_hint - 0.5) + (text_origin_at_opposite ? text_size : 0);
    }
    return offset;

} // Label::text_offset

// ----------------------------------------------------------------------

Label& Labels::add(size_t aIndex, const acmacs::chart::Chart& aChart)
{
    auto found = std::find_if(mLabels.begin(), mLabels.end(), [&aIndex](const auto& label) { return label.index() == aIndex; });
    if (found == mLabels.end()) {
        mLabels.emplace_back(aIndex);
        found = mLabels.end() - 1;
        const auto number_of_antigens = aChart.number_of_antigens();
        if (aIndex < number_of_antigens)
            found->display_name(aChart.antigen(aIndex)->full_name());
        else
            found->display_name(aChart.serum(aIndex - number_of_antigens)->full_name());
    }
    return *found;

} // Labels::add

// ----------------------------------------------------------------------

void Labels::remove(size_t aIndex)
{
    const auto found = std::find_if(mLabels.begin(), mLabels.end(), [&aIndex](const auto& label) { return label.index() == aIndex; });
    if (found != mLabels.end())
        mLabels.erase(found);

} // Labels::remove

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
