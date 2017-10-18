#include "acmacs-draw/surface.hh"
#include "acmacs-chart/chart-base.hh"
#include "acmacs-map-draw/labels.hh"
#include "acmacs-map-draw/point-style-draw.hh"

using namespace map_elements;

// ----------------------------------------------------------------------

Label::Label(size_t aIndex)
    : mIndex(aIndex), mOffset{0, 1}, mTextColor{"black"}, mTextSize{12}
{
} // Label::Label

// ----------------------------------------------------------------------

void Label::draw(Surface& aSurface, const LayoutBase& aLayout, const std::vector<PointStyleDraw>& aPointStyles) const
{
    const auto& style = aPointStyles[mIndex];
    if (style.shown()) {
        Coordinates text_origin = aLayout[mIndex];
        if (!text_origin.empty()) { // point is not disconnected
            const double scaled_point_size = aSurface.convert(style.size()).value();
            const Size text_size = aSurface.text_size(mDisplayName, mTextSize, mTextStyle);
            text_origin[0] += text_offset(mOffset.x, scaled_point_size, text_size.width, false);
            text_origin[1] += text_offset(mOffset.y, scaled_point_size, text_size.height, true);
            aSurface.text(text_origin, mDisplayName, mTextColor, mTextSize, mTextStyle);
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

Labels::Labels()
{

} // Labels::Labels

// ----------------------------------------------------------------------

Label& Labels::add(size_t aIndex, const ChartBase& aChart)
{
    auto found = std::find_if(mLabels.begin(), mLabels.end(), [&aIndex](const auto& label) { return label.index() == aIndex; });
    if (found == mLabels.end()) {
        mLabels.emplace_back(aIndex);
        found = mLabels.end() - 1;
        if (aIndex < aChart.number_of_antigens())
            found->display_name(aChart.antigen(aIndex).full_name());
        else
            found->display_name(aChart.serum(aIndex - aChart.number_of_antigens()).full_name());
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

void Labels::draw(Surface& aSurface, const LayoutBase& aLayout, const std::vector<PointStyleDraw>& aPointStyles) const
{
    for (const Label& label: mLabels) {
        label.draw(aSurface, aLayout, aPointStyles);
    }

} // Labels::draw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
