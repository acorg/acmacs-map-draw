#pragma once

#include <string>
#include <vector>

#include "acmacs-base/size-scale.hh"
#include "acmacs-base/color-target.hh"
#include "acmacs-base/size.hh"
#include "acmacs-draw/text-style.hh"

// ----------------------------------------------------------------------

class ChartBase;
class Surface;
class ChartDraw;
class Layout;
class PointStyleDraw;

// ----------------------------------------------------------------------

namespace map_elements
{
    class Label
    {
     public:
        inline Label(size_t aIndex);

        void draw(Surface& aSurface, const LayoutBase& aLayout, const std::vector<PointStyleDraw>& aPointStyles) const;

        inline Label& offset(double x, double y) { mOffset.set(x, y); return *this; }
        inline Label& display_name(std::string aDisplayName) { mDisplayName = aDisplayName; return *this; }
        inline Label& color(Color aColor) { mTextColor = aColor; return *this; }
        inline Label& size(double aSize) { mTextSize = aSize; return *this; }
        inline Label& weight(std::string aWeight) { mTextStyle.weight(aWeight); return *this; }
        inline Label& slant(std::string aSlant) { mTextStyle.slant(aSlant); return *this; }
        inline Label& font_family(std::string aFamily) { mTextStyle.font_family(aFamily); return *this; }

        inline size_t index() const { return mIndex; }

     private:
        size_t mIndex;
        acmacs::Location mOffset;
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

        Label& add(size_t aIndex, const ChartBase& aChart);
        void remove(size_t aIndex);
        void draw(Surface& aSurface, const LayoutBase& aLayout, const std::vector<PointStyleDraw>& aPointStyles) const;

     private:
        std::vector<Label> mLabels;
    };

} // namespace map_elements

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
