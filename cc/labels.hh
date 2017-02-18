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
class Layout;
class PointStyle;

// ----------------------------------------------------------------------

class Label
{
 public:
    inline Label(size_t aIndex);

    void draw(Surface& aSurface, const Layout& aLayout, const std::vector<PointStyle>& aPointStyles) const;

    inline Label& offset(double x, double y) { mOffset.set(x, y); return *this; }
    inline Label& display_name(std::string aDisplayName) { mDisplayName = aDisplayName; return *this; }
    inline Label& color(Color aColor) { mTextColor = aColor; return *this; }
    inline Label& size(double aSize) { mTextSize = aSize; return *this; }

 private:
    size_t mIndex;
    Location mOffset;
    std::string mDisplayName;
    Color mTextColor;
    Pixels mTextSize;
    TextStyle mTextStyle;

    double text_offset(double offset_hint, double point_size, double text_size, bool text_origin_at_opposite) const;
};

// ----------------------------------------------------------------------

class Labels
{
 public:
    Labels();

    Label& add(size_t aIndex, const Chart& aChart);
    void draw(Surface& aSurface, const Layout& aLayout, const std::vector<PointStyle>& aPointStyles) const;

 private:
    std::vector<Label> mLabels;
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
