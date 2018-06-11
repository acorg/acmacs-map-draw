#pragma once

#include <string>
#include <vector>
#include <memory>

#include "acmacs-base/range.hh"
#include "acmacs-base/timeit.hh"
#include "acmacs-draw/viewport.hh"
#include "acmacs-map-draw/point-style-draw.hh"
#include "acmacs-map-draw/map-elements.hh"
#include "acmacs-map-draw/labels.hh"
#include "acmacs-map-draw/chart-select-interface.hh"

namespace acmacs::surface { class Surface; }
namespace acmacs::draw { class DrawElements; }

// ----------------------------------------------------------------------

class ChartDraw : public ChartSelectInterface
{
 public:
    using ChartSelectInterface::ChartSelectInterface;

    void draw(acmacs::surface::Surface& aSurface) const;
    void draw(std::string aFilename, double aSize, report_time aTimer = report_time::No) const;
    std::string draw_json(report_time aTimer = report_time::No) const;
    std::string draw_pdf(double aSize, report_time aTimer = report_time::No) const;
    const acmacs::Viewport& calculate_viewport(bool verbose = true);

    template <typename T> void modify_drawing_order(const T& aPoints, PointDrawingOrder aPointDrawingOrder)
        {
            switch (aPointDrawingOrder) {
              case PointDrawingOrder::Raise:
                  plot_spec().raise(aPoints);
                  break;
              case PointDrawingOrder::Lower:
                  plot_spec().lower(aPoints);
                  break;
              case PointDrawingOrder::NoChange:
                  break;
            }
        }

    template <typename T> void modify_sera_drawing_order(const T& aPoints, PointDrawingOrder aPointDrawingOrder)
        {
            switch (aPointDrawingOrder) {
              case PointDrawingOrder::Raise:
                  plot_spec().raise_serum(aPoints);
                  break;
              case PointDrawingOrder::Lower:
                  plot_spec().lower_serum(aPoints);
                  break;
              case PointDrawingOrder::NoChange:
                  break;
            }
        }

    void modify(const acmacs::chart::Indexes& aPoints, const acmacs::PointStyle& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange)
        {
            plot_spec().modify(aPoints, aStyle);
            modify_drawing_order(aPoints, aPointDrawingOrder);
        }

    void modify(size_t aPointNo, const acmacs::PointStyle& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange)
        {
            plot_spec().modify(aPointNo, aStyle);
            modify_drawing_order(aPointNo, aPointDrawingOrder);
        }

    void modify_sera(const acmacs::chart::Indexes& aSera, const acmacs::PointStyle& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange)
        {
            plot_spec().modify_sera(aSera, aStyle);
            modify_sera_drawing_order(aSera, aPointDrawingOrder);
        }

    void modify_serum(size_t aSerumNo, const acmacs::PointStyle& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange)
        {
            plot_spec().modify_serum(aSerumNo, aStyle);
            modify_sera_drawing_order(aSerumNo, aPointDrawingOrder);
        }

    void hide_all_except(const acmacs::chart::Indexes& aNotHide);
    void mark_egg_antigens();
    void mark_reassortant_antigens();
    void mark_all_grey(Color aColor);
    void scale_points(double aPointScale, double aOulineScale) { plot_spec().scale_all(aPointScale, aOulineScale); }
    void modify_all_sera(const acmacs::PointStyle& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange) { modify_sera(chart().sera()->all_indexes(), aStyle, aPointDrawingOrder); }

    void rotate(double aAngle)
        {
            if (!float_zero(aAngle))
                log("rotate radians:", aAngle, " degrees:", 180.0 * aAngle / M_PI, " ", aAngle > 0 ? "counter-" : "", "clockwise");
            projection().rotate_radians(aAngle);
        }

    void flip(double aX, double aY)
        {
            log("flip ", aX, " ", aY);
            projection().flip(aX, aY); // reflect about a line specified with vector [aX, aY]
        }

    void viewport(double aX, double aY, double aSize) { mViewport.set(aX, aY, aSize); }
    void viewport(const acmacs::Viewport& aViewport) { mViewport = aViewport; }
    const acmacs::Viewport& viewport() const { return mViewport; }

    void background_color(Color aBackground) { dynamic_cast<map_elements::BackgroundBorderGrid&>(mMapElements["background-border-grid"]).background_color(aBackground); }
    void grid(Color aGridColor, double aGridLineWidth) { dynamic_cast<map_elements::BackgroundBorderGrid&>(mMapElements["background-border-grid"]).grid(aGridColor, aGridLineWidth); }
    void border(Color aBorderColor, double aBorderWidth) { dynamic_cast<map_elements::BackgroundBorderGrid&>(mMapElements["background-border-grid"]).border(aBorderColor, aBorderWidth); }
    auto& continent_map(const acmacs::Location& aOffset, Pixels aWidth) { return dynamic_cast<map_elements::ContinentMap&>(mMapElements["continent-map"]).offset_width(aOffset, aWidth); }
    auto& continent_map() { return dynamic_cast<map_elements::ContinentMap&>(mMapElements["continent-map"]); }
    map_elements::LegendPointLabel& legend_point_label(const acmacs::Location& aOffset) { auto& legend = dynamic_cast<map_elements::LegendPointLabel&>(mMapElements["legend-point-label"]); legend.offset(aOffset); return legend; }
    map_elements::LegendPointLabel& legend_point_label() { return dynamic_cast<map_elements::LegendPointLabel&>(mMapElements["legend-point-label"]); }
    void remove_legend() { mMapElements.remove("legend-point-label"); }
    map_elements::Title& title(const acmacs::Location& aOffset) { auto& title = dynamic_cast<map_elements::Title&>(mMapElements["title"]); title.offset(aOffset); return title; }
    map_elements::Title& title() { return dynamic_cast<map_elements::Title&>(mMapElements["title"]); }
    bool has_title() const { return mMapElements.exists("title"); }
    map_elements::Labels& labels() { return mLabels; }
    acmacs::draw::PointLabel& add_label(size_t aIndex) { return mLabels.add(aIndex, chart()); }
    void add_all_labels() const { const_cast<ChartDraw*>(this)->mLabels.add_all(chart()); }
    void remove_label(size_t aIndex) { return mLabels.remove(aIndex); }
    map_elements::SerumCircle& serum_circle(size_t aSerumNo, Scaled aRadius);
    map_elements::Line& line(const acmacs::Location& aBegin, const acmacs::Location& aEnd);
    map_elements::Line& line(double slope, double intercept);
    map_elements::Arrow& arrow(const acmacs::Location& aBegin, const acmacs::Location& aEnd);
    map_elements::Point& point(const acmacs::Location& aCenter, Pixels aSize);
    map_elements::Rectangle& rectangle(const acmacs::Location& aCorner1, const acmacs::Location& aCorner2);
    map_elements::Circle& circle(const acmacs::Location& aCenter, Scaled aSize);
    void remove_serum_circles();

    void save(std::string aFilename, std::string aProgramName);

 private:
    acmacs::Viewport mViewport;
    map_elements::Elements mMapElements;
    map_elements::Labels mLabels;

    void draw(acmacs::draw::DrawElements& painter) const;

}; // class ChartDraw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
