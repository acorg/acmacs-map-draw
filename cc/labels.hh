#pragma once

#include <string>
#include <vector>

#include "acmacs-draw/color.hh"
#include "acmacs-draw/size.hh"
#include "acmacs-draw/size-scale.hh"
#include "acmacs-draw/text-style.hh"

class Chart;
class Surface;
class ChartDraw;

// ----------------------------------------------------------------------

class Label
{
 public:
    inline Label(size_t aIndex);

    void draw(Surface& aSurface, const ChartDraw& aChartDraw) const;

 private:
    size_t mIndex;
    Location mOffset;
    std::string mDisplayName;
    Color mColor;
    Pixels mSize;
    TextStyle mStyle;
};

// ----------------------------------------------------------------------

class Labels
{
 public:
    Labels();

    Label& add(std::string aName, const Chart& aChart);
    void draw(Surface& aSurface, const ChartDraw& aChartDraw) const;

 private:
    std::vector<Label> mLabels;
};

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
