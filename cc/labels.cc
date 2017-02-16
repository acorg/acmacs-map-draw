#include "acmacs-draw/surface.hh"
#include "acmacs-map-draw/labels.hh"

// ----------------------------------------------------------------------

Label::Label(size_t aIndex)
    : mIndex(aIndex), mOffset{0, 1}, mTextColor{"black"}, mTextSize{12}
{
} // Label::Label

// ----------------------------------------------------------------------

void Label::draw(Surface& aSurface, const ChartDraw& aChartDraw) const
{
    aSurface.text({-8, 0}, "LABEL", mTextColor, mTextSize, mTextStyle);

} // Label::draw

// ----------------------------------------------------------------------

Labels::Labels()
{

} // Labels::Labels

// ----------------------------------------------------------------------

Label& Labels::add(std::string aName, const Chart& aChart)
{

} // Labels::add

// ----------------------------------------------------------------------

Label& Labels::add(size_t aIndex, const Chart& /*aChart*/)
{
    mLabels.emplace_back(aIndex);
    return mLabels.back();

} // Labels::add

// ----------------------------------------------------------------------

void Labels::draw(Surface& aSurface, const ChartDraw& aChartDraw) const
{
    for (const Label& label: mLabels) {
        label.draw(aSurface, aChartDraw);
    }

} // Labels::draw

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
