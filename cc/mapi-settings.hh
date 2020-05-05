#pragma once

// #include <memory>

#include "acmacs-base/settings.hh"
#include "acmacs-base/point-style.hh"
//#include "acmacs-chart-2/chart.hh"

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

        void update_env();

        // ----------------------------------------------------------------------
        // mapi-settings-antigens.cc

        bool apply_antigens();
        bool apply_sera();

        acmacs::chart::PointIndexList select_antigens() const;
        acmacs::chart::PointIndexList select_sera() const;
        acmacs::PointStyleModified style_from_toplevel_environment() const;
        Color color(const rjson::v3::value& value) const;

        // ----------------------------------------------------------------------

    };

} // namespace acmacs::mapi::inline v1

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
