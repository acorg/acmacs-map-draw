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
    class PlotSpecModify;

} // namespace acmacs::chart

namespace acmacs::surface { class Surface; }
class ChartDraw;

// ----------------------------------------------------------------------

namespace map_elements
{
    class Labels : public acmacs::draw::PointLabels
    {
     public:
        Labels() = default;

        using acmacs::draw::PointLabels::add;
        acmacs::draw::PointLabel& add(size_t aIndex, const acmacs::chart::Chart& aChart);
        void add_all(const acmacs::chart::Chart& aChart);

        // obsolete
        void draw(acmacs::surface::Surface& aSurface, const acmacs::Layout& aLayout, const acmacs::chart::PlotSpecModify& aPlotSpec) const
        {
            std::for_each(begin(), end(), [&, this](const auto& aLabel) { this->draw(aLabel, aSurface, aLayout, aPlotSpec); });
        }

        // obsolete
        void draw(const acmacs::draw::PointLabel& label, acmacs::surface::Surface& aSurface, const acmacs::Layout& aLayout, const acmacs::chart::PlotSpecModify& aPlotSpec) const;

    }; // class Labels

} // namespace map_elements

// ----------------------------------------------------------------------
