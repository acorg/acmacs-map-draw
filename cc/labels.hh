#pragma once

#include <string>
#include <vector>

#include "acmacs-base/size-scale.hh"
#include "acmacs-base/color.hh"
#include "acmacs-base/size.hh"
#include "acmacs-base/layout.hh"
#include "acmacs-draw/draw-points.hh"

// ----------------------------------------------------------------------

namespace acmacs::chart
{
    class Chart;
    using Layout = acmacs::LayoutInterface;
    class PlotSpecModify;

} // namespace acmacs::chart

namespace acmacs::surface { class Surface; }
class ChartDraw;

// ----------------------------------------------------------------------

namespace map_elements
{
    class Label : public acmacs::draw::PointLabel
    {
     public:
        using acmacs::draw::PointLabel::PointLabel;
        // Label(size_t aIndex) : mIndex(aIndex), mOffset{0, 1}, mTextColor{BLACK}, mTextSize{12} {}

        void draw(acmacs::surface::Surface& aSurface, const acmacs::chart::Layout& aLayout, const acmacs::chart::PlotSpecModify& aPlotSpec) const;

        // Label& offset(double x, double y) { mOffset.set(x, y); return *this; }
        // Label& display_name(std::string_view aDisplayName) { mDisplayName = aDisplayName; return *this; }
        // Label& color(Color aColor) { mTextColor = aColor; return *this; }
        // Label& size(double aSize) { mTextSize = aSize; return *this; }
        // Label& weight(std::string aWeight) { mTextStyle.weight = aWeight; return *this; }
        // Label& slant(std::string aSlant) { mTextStyle.slant = aSlant; return *this; }
        // Label& font_family(std::string aFamily) { mTextStyle.font_family = aFamily; return *this; }

        // size_t index() const { return mIndex; }

     private:
        // size_t mIndex;
        // acmacs::Location mOffset;
        // std::string mDisplayName;
        // Color mTextColor;
        // Pixels mTextSize;
        // acmacs::TextStyle mTextStyle;

        double text_offset(double offset_hint, double point_size, double text_size, bool text_origin_at_opposite) const;
    };

// ----------------------------------------------------------------------

    class Labels
    {
     public:
        Labels() = default;

        Label& add(size_t aIndex, const acmacs::chart::Chart& aChart);
        void remove(size_t aIndex);

        void draw(acmacs::surface::Surface& aSurface, const acmacs::chart::Layout& aLayout, const acmacs::chart::PlotSpecModify& aPlotSpec) const
            {
                std::for_each(mLabels.begin(), mLabels.end(), [&](const Label& aLabel) { aLabel.draw(aSurface, aLayout, aPlotSpec); });
            }

     private:
        std::vector<Label> mLabels;
    };

} // namespace map_elements

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
