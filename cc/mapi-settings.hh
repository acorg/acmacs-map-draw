#pragma once

#include "acmacs-base/settings-v3.hh"
#include "acmacs-base/point-style.hh"
#include "acmacs-base/time-series.hh"
#include "acmacs-chart-2/selected-antigens-sera.hh"
#include "acmacs-map-draw/point-style-draw.hh"
#include "acmacs-map-draw/coordinates.hh"
#include "acmacs-map-draw/point-style.hh"

// ----------------------------------------------------------------------

namespace acmacs::chart
{
    class PointIndexList;
    class Antigens;
    class Sera;
    class SerumCircle;
}

class ChartDraw;

namespace map_elements::v1
{
    class LegendPointLabel;
    class Title;
    class SerumCircle;
}

class ChartAccess;

// ----------------------------------------------------------------------

namespace acmacs::mapi::inline v1
{
    using error = settings::v3::error;

    struct Figure;

    class unrecognized : public error
    {
      public:
        using error::error;
        unrecognized() : error{""} {}
    };

    // ----------------------------------------------------------------------

    class Settings : public settings::v3::Data
    {
      public:
        // use chart_draw.settings() to create!
        Settings(ChartDraw& chart_draw) : chart_draw_{chart_draw} { update_env(); }

        using settings::v3::Data::load;
        bool apply_built_in(std::string_view name) override; // returns true if built-in command with that name found and applied

        enum class if_null { empty, warn_empty, raise, all }; // return empty, return empty and print warning, throw exception, return all indexes
        acmacs::chart::SelectedAntigensModify selected_antigens(const rjson::v3::value& select_clause, if_null ifnull = if_null::raise) const;
        acmacs::chart::SelectedSeraModify selected_sera(const rjson::v3::value& select_clause, if_null ifnull = if_null::raise) const;
        // OBSOLETE
        acmacs::chart::PointIndexList select_antigens(const rjson::v3::value& select_clause, if_null ifnull = if_null::raise) const;
        acmacs::chart::PointIndexList select_sera(const rjson::v3::value& select_clause, if_null ifnull = if_null::raise) const;

        constexpr ChartDraw& chart_draw() const { return chart_draw_; }

        void filter_inside_path(acmacs::chart::PointIndexList& indexes, const rjson::v3::value& points, size_t index_base) const; // mapi-settings-drawing.cc

        struct time_series_data
        {
            const std::string filename_pattern;
            const std::vector<std::string_view> title;
            const acmacs::time_series::series series;
            const acmacs::chart::PointIndexList shown_on_all;
        };

      protected:
        // ----------------------------------------------------------------------
        // mapi-settings-antigens.cc

        virtual bool apply_antigens();
        virtual bool apply_sera();
        // returns if key has been processed
        virtual bool select(const acmacs::chart::Antigens& antigens, acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& value) const;
        virtual bool select(const acmacs::chart::Sera& sera, acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& value) const;

        void update_env();
        void update_env_upon_projection_change(); // update stress

        // ----------------------------------------------------------------------
        // mapi-settings-antigens.cc

        template <typename AgSr> acmacs::chart::PointIndexList select(const AgSr& ag_sr, const rjson::v3::value& select_clause, if_null ifnull) const;
        template <typename AgSr> void check_titrated_against(acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& value) const;
        void mark_serum(size_t serum_index, const rjson::v3::value& style);

        PointDrawingOrder drawing_order_from_environment() const;
        PointDrawingOrder drawing_order_from(const rjson::v3::value& source) const;
        PointDrawingOrder drawing_order_from(std::string_view key, const rjson::v3::value& val) const;

        point_style_t style_from_environment() const;
        point_style_t style_from(const rjson::v3::value& source) const;
        void update_style(point_style_t& style, std::string_view key, const rjson::v3::value& val) const;
        template <typename AgSr> bool color_according_to_passage(const AgSr& ag_sr, const acmacs::chart::PointIndexList& indexes, const point_style_t& point_style);
        bool color_according_to_aa_at_pos(const acmacs::chart::PointIndexList& indexes, const point_style_t& point_style);

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
        void read_figure(Figure& figure, const rjson::v3::value& points, const rjson::v3::value& close) const;

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

        virtual map_elements::v1::Title& title();
        bool apply_title();

        // ----------------------------------------------------------------------
        // mapi-settings-serum-circles.cc

        bool apply_serum_circles();
        bool hide_serum_circle(const rjson::v3::value& criteria, size_t serum_no, double radius) const;
        acmacs::chart::PointIndexList select_antigens_for_serum_circle(size_t serum_index, const rjson::v3::value& antigen_selector);
        void make_circle(size_t serum_index, Scaled radius, std::string_view serum_passage, const rjson::v3::value& plot);
        bool apply_serum_coverage();
        void report_circles(fmt::memory_buffer& report, size_t serum_index, const acmacs::chart::Antigens& antigens, const acmacs::chart::PointIndexList& antigen_indexes,
                            const acmacs::chart::SerumCircle& empirical, const acmacs::chart::SerumCircle& theoretical, const rjson::v3::value& hide_if,
                            std::optional<std::string_view> forced_homologous_titer) const;

        // ----------------------------------------------------------------------
        // mapi-settings-procrustes.cc

        bool apply_reset();
        bool apply_export() const;
        bool apply_pdf() const;
        bool apply_relax();
        bool apply_procrustes();
        bool apply_remove_procrustes();
        const ChartAccess& get_chart(const rjson::v3::value& source, size_t dflt) const;
        bool apply_move();
        void make_pdf(std::string_view filename, double width, bool open) const;
        std::string get_filename() const;
        std::string substitute_in_filename(std::string_view filename) const;

        // ----------------------------------------------------------------------
        // mapi-settings-sequences.cc

        bool apply_compare_sequences();

        // ----------------------------------------------------------------------
        // mapi-settings-vaccine.cc

        bool apply_vaccine();

        // ----------------------------------------------------------------------
        // mapi-settings-time-series.cc

        bool apply_time_series();

        time_series_data time_series_settings() const;

      private:
        ChartDraw& chart_draw_;

    }; // class Settings

} // namespace acmacs::mapi::inline v1

extern template bool acmacs::mapi::v1::Settings::color_according_to_passage(const acmacs::chart::Antigens&, const acmacs::chart::PointIndexList& indexes, const point_style_t& style);
extern template bool acmacs::mapi::v1::Settings::color_according_to_passage(const acmacs::chart::Sera&, const acmacs::chart::PointIndexList& indexes, const point_style_t& style);

template <> struct fmt::formatter<acmacs::mapi::v1::Settings::time_series_data> : fmt::formatter<acmacs::fmt_helper::default_formatter>
{
    template <typename FormatCtx> auto format(const acmacs::mapi::v1::Settings::time_series_data& value, FormatCtx& ctx)
    {
        return format_to(ctx.out(), "time_series_data {{\n    filename_pattern: {}\n    title: {}\n    series: {}\n}}", //
                         value.filename_pattern, value.title, value.series);
    }
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
