#pragma once

#include <string>
#include <vector>
#include <memory>

#include "acmacs-base/range.hh"
#include "acmacs-base/timeit.hh"
#include "acmacs-draw/viewport.hh"
#include "acmacs-map-draw/point-style-draw.hh"
#include "acmacs-map-draw/map-elements.hh"
#include "acmacs-map-draw/map-elements-v1.hh"
#include "acmacs-map-draw/labels.hh"
#include "acmacs-map-draw/chart-select-interface.hh"

namespace acmacs { class LineDefinedByEquation; }
namespace acmacs::surface { class Surface; }
namespace acmacs::draw { class DrawElements; }

// ----------------------------------------------------------------------

class MapViewport
{
  public:
    void calculate(const acmacs::Layout& layout, std::string_view by);

    void set(const acmacs::PointCoordinates& origin, double size);
    void set(const acmacs::Viewport& aViewport);
    void set_recalculate() { recalculate_ = true; }

    bool empty() const { return viewport_.empty(); }
    const acmacs::Viewport& use(std::string_view by) const;
    acmacs::Viewport& use(std::string_view by);

  private:
    acmacs::Viewport viewport_;
    bool recalculate_{true};
    mutable std::vector<std::string> used_by_;

}; // class MapViewport

// ----------------------------------------------------------------------

class ChartDraw : public ChartSelectInterface
{
 public:
    using ChartSelectInterface::ChartSelectInterface;

    enum class apply_map_transformation { no, yes };

    void draw(acmacs::surface::Surface& aSurface) const;
    void draw(std::string_view aFilename, double aSize, report_time aTimer = report_time::no) const;
    std::string draw_json(report_time aTimer = report_time::no) const;
    std::string draw_pdf(double aSize, report_time aTimer = report_time::no) const;
    void calculate_viewport(std::string_view by) const;

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

    void modify(const acmacs::chart::Indexes& aPoints, const acmacs::PointStyleModified& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange)
        {
            plot_spec().modify(aPoints, aStyle);
            modify_drawing_order(aPoints, aPointDrawingOrder);
        }

    void modify(size_t aPointNo, const acmacs::PointStyleModified& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange)
        {
            plot_spec().modify(aPointNo, aStyle);
            modify_drawing_order(aPointNo, aPointDrawingOrder);
        }

    void modify_sera(const acmacs::chart::Indexes& aSera, const acmacs::PointStyleModified& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange)
        {
            plot_spec().modify_sera(aSera, aStyle);
            modify_sera_drawing_order(aSera, aPointDrawingOrder);
        }

    void modify_serum(size_t aSerumNo, const acmacs::PointStyleModified& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange)
        {
            plot_spec().modify_serum(aSerumNo, aStyle);
            modify_sera_drawing_order(aSerumNo, aPointDrawingOrder);
        }

    void hide_all_except(const acmacs::chart::Indexes& aNotHide);
    void mark_egg_antigens();
    void mark_reassortant_antigens();
    void mark_all_grey(Color aColor);
    void scale_points(double aPointScale, double aOulineScale) { plot_spec().scale_all(aPointScale, aOulineScale); }
    void modify_all_sera(const acmacs::PointStyleModified& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange) { modify_sera(chart().sera()->all_indexes(), aStyle, aPointDrawingOrder); }

    void rotate(double aAngle);
    void flip(double aX, double aY);

    constexpr acmacs::Viewport& viewport(std::string_view by) { return viewport_.use(by); }
    constexpr const acmacs::Viewport& viewport(std::string_view by) const { return viewport_.use(by); }
    void set_viewport(const acmacs::Viewport& viewport) { viewport_.set(viewport); }

    void background_color(Color aBackground) { dynamic_cast<map_elements::v1::BackgroundBorderGrid&>(mMapElements["background-border-grid"]).background_color(aBackground); }
    void grid(Color aGridColor, double aGridLineWidth) { dynamic_cast<map_elements::v1::BackgroundBorderGrid&>(mMapElements["background-border-grid"]).grid(aGridColor, aGridLineWidth); }
    void border(Color aBorderColor, double aBorderWidth) { dynamic_cast<map_elements::v1::BackgroundBorderGrid&>(mMapElements["background-border-grid"]).border(aBorderColor, aBorderWidth); }
    auto& continent_map(const acmacs::PointCoordinates& aOffset, Pixels aWidth) { return dynamic_cast<map_elements::v1::ContinentMap&>(mMapElements["continent-map"]).offset_width(aOffset, aWidth); }
    auto& continent_map() { return dynamic_cast<map_elements::v1::ContinentMap&>(mMapElements["continent-map"]); }
    map_elements::v1::LegendPointLabel& legend_point_label(const acmacs::PointCoordinates& aOffset) { auto& legend = dynamic_cast<map_elements::v1::LegendPointLabel&>(mMapElements["legend-point-label"]); legend.offset(aOffset); return legend; }
    map_elements::v1::LegendPointLabel& legend_point_label() { return dynamic_cast<map_elements::v1::LegendPointLabel&>(mMapElements["legend-point-label"]); }
    void remove_legend() { mMapElements.remove("legend-point-label"); }
    map_elements::v1::Title& title(const acmacs::PointCoordinates& aOffset) { auto& title = dynamic_cast<map_elements::v1::Title&>(mMapElements["title"]); title.offset(aOffset); return title; }
    map_elements::v1::Title& title() { return dynamic_cast<map_elements::v1::Title&>(mMapElements["title"]); }
    bool has_title() const { return mMapElements.exists("title"); }
    map_elements::Labels& labels() { return mLabels; }
    acmacs::draw::PointLabel& add_label(size_t aIndex) { return mLabels.add(aIndex, chart()); }
    void add_all_labels() const { const_cast<ChartDraw*>(this)->mLabels.add_all(chart()); }
    void remove_label(size_t aIndex) { return mLabels.remove(aIndex); }
    map_elements::v1::SerumCircle& serum_circle(size_t aSerumNo, Scaled aRadius);
    map_elements::v1::Line& line(const acmacs::PointCoordinates& aBegin, const acmacs::PointCoordinates& aEnd);
    map_elements::v1::Line& line(const acmacs::LineDefinedByEquation& line, apply_map_transformation a_apply_map_transformation);
    map_elements::v1::Path& path();
    map_elements::v1::Arrow& arrow(const acmacs::PointCoordinates& aBegin, const acmacs::PointCoordinates& aEnd);
    map_elements::v1::Point& point(const acmacs::PointCoordinates& aCenter, Pixels aSize);
    map_elements::v1::Rectangle& rectangle(const acmacs::PointCoordinates& aCorner1, const acmacs::PointCoordinates& aCorner2);
    map_elements::v1::Circle& circle(const acmacs::PointCoordinates& aCenter, Scaled aSize);
    void remove_serum_circles();

    void save(std::string_view aFilename, std::string_view aProgramName);

    constexpr auto& map_elements() { return mMapElements; }
    constexpr const auto& map_elements() const { return mMapElements; }

 private:
    mutable MapViewport viewport_;
    map_elements::Elements mMapElements;
    map_elements::Labels mLabels;

    void draw(acmacs::draw::DrawElements& painter) const;

}; // class ChartDraw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
