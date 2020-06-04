#pragma once

#include "acmacs-base/settings.hh"
#include "acmacs-base/point-style.hh"
#include "acmacs-base/time-series.hh"
#include "seqdb-3/sequence.hh"
#include "acmacs-map-draw/point-style-draw.hh"
#include "acmacs-map-draw/coordinates.hh"

// ----------------------------------------------------------------------

namespace acmacs::chart
{
    class PointIndexList;
    class Antigens;
    class Sera;
    // class Chart;
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
    class SerumCircle;
}

namespace map_elements::v2
{
    struct PathData;
}

class ChartAccess;

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

        std::string substitute_chart_metadata(std::string_view pattern, const ChartAccess& chart_access) const;
        void chart_metadata(fmt::dynamic_format_arg_store<fmt::format_context>& store, const ChartAccess& chart_access) const;

        void update_env();

        // ----------------------------------------------------------------------
        // mapi-settings-antigens.cc

        bool apply_antigens();
        bool apply_sera();
        template <typename AgSr> acmacs::chart::PointIndexList select(const AgSr& ag_sr, const rjson::v3::value& select_clause, if_null ifnull) const;
        template <typename AgSr> void check_titrated_against(acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& value) const;
        void mark_serum(size_t serum_index, const rjson::v3::value& style);

        PointDrawingOrder drawing_order_from_toplevel_environment() const;
        PointDrawingOrder drawing_order_from(const rjson::v3::value& source) const;
        PointDrawingOrder drawing_order_from(std::string_view key, const rjson::v3::value& val) const;

        struct passage_color_t
        {
            void init_passage_colors();
            void apply(const acmacs::color::Modifier& modifier);
            std::optional<acmacs::color::Modifier> egg;
            std::optional<acmacs::color::Modifier> reassortant;
            std::optional<acmacs::color::Modifier> cell;
            std::optional<acmacs::seqdb::pos1_t> pos; // coloring by aa at pos
            std::vector<acmacs::color::Modifier> color_order; // coloring by aa at pos
        };

        struct point_style_t
        {
            acmacs::PointStyleModified style;
            std::optional<passage_color_t> passage_fill;
            std::optional<passage_color_t> passage_outline;
        };

        point_style_t style_from_toplevel_environment() const;
        point_style_t style_from(const rjson::v3::value& source) const;
        void update_style(point_style_t& style, std::string_view key, const rjson::v3::value& val) const;
        template <typename AgSr> bool color_according_to_passage(const AgSr& ag_sr, const acmacs::chart::PointIndexList& indexes, const point_style_t& point_style);
        bool color_according_to_aa_at_pos(const acmacs::chart::PointIndexList& indexes, const point_style_t& point_style);

        using modifier_or_passage_t  = std::variant<acmacs::color::Modifier, passage_color_t>;
        modifier_or_passage_t color(const rjson::v3::value& value, std::optional<Color> if_null = std::nullopt) const;

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
        std::optional<map_elements::v2::Coordinates> read_coordinates(const rjson::v3::value& source) const;
        void read_path_vertices(std::vector<map_elements::v2::Coordinates>& path, const rjson::v3::value& points) const;
        void read_path_data(map_elements::v2::PathData& path, const rjson::v3::value& points, const rjson::v3::value& close) const;

        // ----------------------------------------------------------------------
        // mapi-settings-labels.cc

        void add_labels(const acmacs::chart::PointIndexList& indexes, size_t index_base, const rjson::v3::value& label_data);
        void add_label(size_t index, size_t index_base, const rjson::v3::value& label_data);

        // ----------------------------------------------------------------------
        // mapi-settings-legend.cc

        map_elements::v1::LegendPointLabel& legend();
        bool apply_legend();
        void add_legend_continent_map();
        void add_legend(const acmacs::chart::PointIndexList& indexes, const acmacs::PointStyleModified& style, const rjson::v3::value& legend_data);
        void add_legend(const acmacs::chart::PointIndexList& indexes, const acmacs::PointStyleModified& style, std::string_view label, const rjson::v3::value& legend_data);

        map_elements::v1::Title& title();
        bool apply_title();

        // ----------------------------------------------------------------------
        // mapi-settings-serum-circles.cc

        bool apply_serum_circles();
        acmacs::chart::PointIndexList select_antigens_for_serum_circle(size_t serum_index, const rjson::v3::value& antigen_selector);
        void make_circle(map_elements::v1::SerumCircle& circle, std::string_view serum_passage, const rjson::v3::value& plot);
        bool apply_serum_coverage();

        // ----------------------------------------------------------------------
        // mapi-settings-procrustes.cc

        bool apply_reset();
        bool apply_export();
        bool apply_pdf();
        bool apply_relax();
        bool apply_procrustes();
        const ChartAccess& get_chart(const rjson::v3::value& source, size_t dflt);
        bool apply_move();
        void make_pdf(std::string_view filename, double width, bool open);

        // ----------------------------------------------------------------------
        // mapi-settings-sequences.cc

        bool apply_compare_sequences();

        // ----------------------------------------------------------------------
        // mapi-settings-vaccine.cc

        bool apply_vaccine();

        // ----------------------------------------------------------------------
        // mapi-settings-time-series.cc

        bool apply_time_series();

        struct time_series_data
        {
            std::string filename_pattern;
            const acmacs::time_series::v2::series series;
        };
        time_series_data time_series_settings();

    }; // class Settings

} // namespace acmacs::mapi::inline v1

extern template bool acmacs::mapi::v1::Settings::color_according_to_passage(const acmacs::chart::Antigens&, const acmacs::chart::PointIndexList& indexes, const point_style_t& style);
extern template bool acmacs::mapi::v1::Settings::color_according_to_passage(const acmacs::chart::Sera&, const acmacs::chart::PointIndexList& indexes, const point_style_t& style);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
