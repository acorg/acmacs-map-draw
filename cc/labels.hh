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
      // obsolete
    class Labels : public acmacs::draw::PointLabels
    {
     public:
        Labels() = default;

        acmacs::draw::PointLabel& add(size_t aIndex, const acmacs::chart::Chart& aChart);

        void draw(acmacs::surface::Surface& aSurface, const acmacs::chart::Layout& aLayout, const acmacs::chart::PlotSpecModify& aPlotSpec) const
            {
                std::for_each(begin(), end(), [&,this](const auto& aLabel) { this->draw(aLabel, aSurface, aLayout, aPlotSpec); });
            }

        void draw(const acmacs::draw::PointLabel& label, acmacs::surface::Surface& aSurface, const acmacs::chart::Layout& aLayout, const acmacs::chart::PlotSpecModify& aPlotSpec) const;

    }; // class Labels

} // namespace map_elements

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
