#include "acmacs-map-draw/labels.hh"

// ----------------------------------------------------------------------

Label::Label(size_t aIndex)
    : mIndex(aIndex), mOffset{0, 1}, mColor{"black"}, mSize{12}
{
} // Label::Label

// ----------------------------------------------------------------------

void Label::draw(Surface& aSurface, const ChartDraw& aChartDraw) const
{

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
