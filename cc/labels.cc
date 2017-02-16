#include "acmacs-draw/surface.hh"
#include "acmacs-chart/chart.hh"
#include "acmacs-map-draw/labels.hh"
#include "acmacs-map-draw/point-style.hh"

// ----------------------------------------------------------------------

Label::Label(size_t aIndex)
    : mIndex(aIndex), mOffset{0, 1}, mTextColor{"black"}, mTextSize{12}
{
} // Label::Label

// ----------------------------------------------------------------------

void Label::draw(Surface& aSurface, const Layout& aLayout, const std::vector<PointStyle>& aPointStyles) const
{
    const auto& style = aPointStyles[mIndex];
    if (style.shown()) {
        Coordinates text_origin = aLayout[mIndex];
        const double scaled_point_size = aSurface.convert(style.size()).value();
        const Size text_size = aSurface.text_size(mDisplayName, mTextSize, mTextStyle);
        text_origin[0] += text_offset(mOffset.x, scaled_point_size, text_size.width, false);
        text_origin[1] += text_offset(mOffset.y, scaled_point_size, text_size.height, true);
        aSurface.text(text_origin, mDisplayName, mTextColor, mTextSize, mTextStyle);
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

Label& Labels::add(std::string aName, const Chart& aChart)
{

} // Labels::add

// ----------------------------------------------------------------------

Label& Labels::add(size_t aIndex, const Chart& aChart)
{
    mLabels.emplace_back(aIndex);
    if (aIndex < aChart.number_of_antigens())
        mLabels.back().display_name(aChart.antigens()[aIndex].full_name());
    else
        mLabels.back().display_name(aChart.sera()[aIndex - aChart.number_of_antigens()].full_name());
    return mLabels.back();

} // Labels::add

// ----------------------------------------------------------------------

void Labels::draw(Surface& aSurface, const Layout& aLayout, const std::vector<PointStyle>& aPointStyles) const
{
    for (const Label& label: mLabels) {
        label.draw(aSurface, aLayout, aPointStyles);
    }

} // Labels::draw

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
