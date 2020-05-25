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

namespace rjson::v3
{
    class value;
};

namespace map_elements::v1
{
    class LegendPointLabel;
    class Title;
}

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

        enum class if_null { empty, warn_empty, raise, all }; // return empty, return empty and print warning, throw exception, return all indexes
        acmacs::chart::PointIndexList select_antigens(const rjson::v3::value& select_clause, if_null ifnull = if_null::raise) const;
        acmacs::chart::PointIndexList select_sera(const rjson::v3::value& select_clause, if_null ifnull = if_null::raise) const;

        constexpr ChartDraw& chart_draw() const { return chart_draw_; }

        void filter_inside_path(acmacs::chart::PointIndexList& indexes, const rjson::v3::value& points, size_t index_base) const; // mapi-settings-drawing.cc

      private:
        ChartDraw& chart_draw_;

        void update_env();

        // ----------------------------------------------------------------------
        // mapi-settings-antigens.cc

        bool apply_antigens();
        bool apply_sera();
        template <typename AgSr> acmacs::chart::PointIndexList select(const AgSr& ag_sr, const rjson::v3::value& select_clause, if_null ifnull) const;
        template <typename AgSr> void check_titrated_against(acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& value) const;

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
            std::optional<passage_color_t> passage_fill;
            std::optional<passage_color_t> passage_outline;
        };

        point_style_t style_from_toplevel_environment() const;
        template <typename AgSr> void color_according_to_passage(const AgSr& ag_sr, const acmacs::chart::PointIndexList& indexes, const point_style_t& point_style);

        using modifier_or_passage_t  = std::variant<acmacs::color::Modifier, passage_color_t>;
        modifier_or_passage_t color(const rjson::v3::value& value) const;

        // ----------------------------------------------------------------------
        // mapi-settings-drawing.cc

        bool apply_circle();
        bool apply_path();
        bool apply_rotate();
        bool apply_flip();
        bool apply_viewport();
        bool apply_background();
        bool apply_border();
        bool apply_grid();
        bool apply_point_scale();
        bool apply_connection_lines();
        bool apply_error_lines();

        // ----------------------------------------------------------------------
        // mapi-settings-labels.cc

        void add_labels(const acmacs::chart::PointIndexList& indexes, size_t index_base, const rjson::v3::value& label_data);
        void add_label(size_t index, size_t index_base, const rjson::v3::value& label_data);

        // ----------------------------------------------------------------------
        // mapi-settings-legend.cc

        map_elements::v1::LegendPointLabel& legend();
        bool apply_legend();
        void add_legend_continent_map();
        void add_legend(const acmacs::chart::PointIndexList& indexes, const point_style_t& style, const rjson::v3::value& legend_data);
        void add_legend(const acmacs::chart::PointIndexList& indexes, const point_style_t& style, std::string_view label, const rjson::v3::value& legend_data);

        map_elements::v1::Title& title();
        bool apply_title();

        // bool apply_serum_circles();
        // bool apply_procrustes();
        // bool apply_time_series();

    }; // class Settings


} // namespace acmacs::mapi::inline v1

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
