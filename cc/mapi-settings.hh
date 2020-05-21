#pragma once

#include "acmacs-base/settings.hh"
#include "acmacs-base/point-style.hh"
#include "acmacs-map-draw/point-style-draw.hh"

// ----------------------------------------------------------------------

namespace acmacs::chart
{
    class PointIndexList;
}

class ChartDraw;

// ----------------------------------------------------------------------

namespace acmacs::mapi::inline v1
{
    using error = settings::error;

    class unrecognized : public error
    {
      public:
        using error::error;
        unrecognized() : error{""} {}
    };

    // ----------------------------------------------------------------------

    class Settings : public settings::Settings
    {
      public:
        Settings(ChartDraw& chart_draw) : chart_draw_{chart_draw} { update_env(); }

        using settings::Settings::load;
        void load(const std::vector<std::string_view>& setting_files, const std::vector<std::string_view>& defines);
        bool apply_built_in(std::string_view name) override; // returns true if built-in command with that name found and applied

      private:
        ChartDraw& chart_draw_;

        constexpr const ChartDraw& chart_draw() const { return chart_draw_; }
        constexpr ChartDraw& chart_draw() { return chart_draw_; }

        void update_env();

        // ----------------------------------------------------------------------
        // mapi-settings-antigens.cc

        bool apply_antigens();
        bool apply_sera();

        acmacs::chart::PointIndexList select_antigens() const;
        acmacs::chart::PointIndexList select_sera() const;
        PointDrawingOrder drawing_order_from_toplevel_environment() const;

        struct passage_color_t
        {
            acmacs::color::Modifier egg{Color{0xFF4040}};
            acmacs::color::Modifier cell{Color{0x4040FF}};
            acmacs::color::Modifier reassortant{Color{0xFF4040}};
        };

        struct point_style_t
        {
            acmacs::PointStyleModified style;
            std::optional<passage_color_t> fill;
            std::optional<passage_color_t> outline;
        };

        point_style_t style_from_toplevel_environment() const;
        template <typename AgSr> void color_according_to_passage(const AgSr& ag_sr, const acmacs::chart::PointIndexList& indexes, const point_style_t& point_style);

        using modifier_or_passage_t  = std::variant<acmacs::color::Modifier, passage_color_t>;
        modifier_or_passage_t color(const rjson::v3::value& value) const;

        // ----------------------------------------------------------------------
        // mapi-settings-drawing.cc

        bool apply_circle();
    };

} // namespace acmacs::mapi::inline v1

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
