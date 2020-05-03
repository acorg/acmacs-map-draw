#pragma once

#include <memory>

#include "acmacs-base/settings.hh"

// ----------------------------------------------------------------------

class ChartDraw;

namespace acmacs::mapi::inline v1
{
    using error = settings::error;

    class Settings : public settings::Settings
    {
      public:
        Settings(ChartDraw& chart_draw) : chart_draw_{chart_draw} { update_env(); }

        using settings::Settings::load;
        void load(const std::vector<std::string_view>& setting_files, const std::vector<std::string_view>& defines);
        bool apply_built_in(std::string_view name) override; // returns true if built-in command with that name found and applied

      private:
        ChartDraw& chart_draw_;

        void update_env();
    };

} // namespace acmacs::mapi::inline v1

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
