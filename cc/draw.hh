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
namespace acmacs::mapi::inline v1 { class Settings; }

// ----------------------------------------------------------------------

class ChartDraw : public ChartSelectInterface
{
  public:
    using ChartSelectInterface::ChartSelectInterface;

    enum class apply_map_transformation { no, yes };

    void draw(acmacs::draw::DrawElements& painter) const;
    void draw(std::string_view aFilename, double aSize, report_time aTimer = report_time::no) const;
    [[deprecated("use draw(acmacs::draw::DrawElements&)")]] void draw(acmacs::surface::Surface& aSurface) const;
    std::string draw_json(report_time aTimer = report_time::no) const;
    std::string draw_pdf(double aSize, report_time aTimer = report_time::no) const;

    void reset();

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

    void modify(const acmacs::chart::Indexes& aPoints, const acmacs::PointStyleModified& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange);
    void modify(size_t aPointNo, const acmacs::PointStyleModified& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange);

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

    // void hide_all_except(const acmacs::chart::Indexes& aNotHide);
    void mark_egg_antigens();
    void mark_reassortant_antigens();
    // void mark_all_grey(Color aColor);
    void scale_points(double aPointScale, double aOulineScale) { plot_spec().scale_all(aPointScale, aOulineScale); }
    void modify_all_sera(const acmacs::PointStyleModified& aStyle, PointDrawingOrder aPointDrawingOrder = PointDrawingOrder::NoChange)
    {
        modify_sera(chart().sera()->all_indexes(), aStyle, aPointDrawingOrder);
    }

    void rotate(double aAngle);
    void flip(double aX, double aY);

    void calculate_viewport() const;
    const acmacs::Viewport& viewport(std::string_view by) const { return viewport_.use(by); }
    const acmacs::Viewport& viewport_before_changing() const { return viewport_.use_before_changing(); }
    void viewport_reset_used_by() const { return viewport_.reset_used_by(); }
    void set_viewport(const acmacs::Viewport& viewport) { viewport_.set(viewport); }
    void set_viewport(const acmacs::PointCoordinates& origin, double size) { viewport_.set(origin, size); }

    void background_color(Color aBackground) { mMapElements.find_or_add<map_elements::v1::BackgroundBorderGrid>("background-border-grid").background_color(aBackground); }
    void grid(Color aGridColor, double aGridLineWidth) { mMapElements.find_or_add<map_elements::v1::BackgroundBorderGrid>("background-border-grid").grid(aGridColor, aGridLineWidth); }
    void border(Color aBorderColor, double aBorderWidth) { mMapElements.find_or_add<map_elements::v1::BackgroundBorderGrid>("background-border-grid").border(aBorderColor, aBorderWidth); }
    auto& continent_map(const acmacs::PointCoordinates& aOffset, Pixels aWidth) { return mMapElements.find_or_add<map_elements::v1::ContinentMap>("continent-map").offset_width(aOffset, aWidth); }
    auto& continent_map() { return mMapElements.find_or_add<map_elements::v1::ContinentMap>("continent-map"); }
    map_elements::v1::LegendPointLabel& legend_point_label(const acmacs::PointCoordinates& aOffset)
    {
        auto& legend = mMapElements.find_or_add<map_elements::v1::LegendPointLabel>("legend-point-label");
        legend.offset(aOffset);
        return legend;
    }
    map_elements::v1::LegendPointLabel& legend_point_label() { return mMapElements.find_or_add<map_elements::v1::LegendPointLabel>("legend-point-label"); }
    void remove_legend() { mMapElements.remove("legend-point-label"); }
    map_elements::v1::Title& title(const acmacs::PointCoordinates& aOffset)
    {
        auto& title = mMapElements.find_or_add<map_elements::v1::Title>("title");
        title.offset(aOffset);
        return title;
    }
    map_elements::v1::Title& title() { return mMapElements.find_or_add<map_elements::v1::Title>("title"); }
    bool has_title() const { return mMapElements.exists("title"); }
    constexpr map_elements::Labels& labels() { return mLabels; }
    constexpr const map_elements::Labels& labels() const { return mLabels; }
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
    constexpr const acmacs::mapi::Settings& settings() const { return *settings_; }

  private:
    class MapViewport
    {
      private:
        MapViewport() = default;

        acmacs::Viewport viewport_;
        bool recalculate_{true};
        mutable std::vector<std::string> used_by_;

        bool empty() const { return viewport_.empty(); }

        const acmacs::Viewport& use_before_changing() const { return viewport_; }
        const acmacs::Viewport& use(std::string_view by) const;
        void reset_used_by(std::string_view message = {}) const;

        void set(const acmacs::PointCoordinates& origin, double size);
        void set(const acmacs::Viewport& aViewport);

        void calculate(const acmacs::Layout& layout);
        void set_recalculate() { recalculate_ = true; }

        friend class ChartDraw;

    }; // class MapViewport

    mutable MapViewport viewport_;
    map_elements::Elements mMapElements;
    map_elements::Labels mLabels;
    const acmacs::mapi::Settings* settings_{nullptr};

    friend class acmacs::mapi::Settings;
    constexpr void settings(const acmacs::mapi::Settings* settings) { settings_ = settings; }

}; // class ChartDraw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
