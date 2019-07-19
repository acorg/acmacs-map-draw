#pragma once

#include <vector>

#include "acmacs-base/in-json.hh"

// ----------------------------------------------------------------------

namespace acmacs::hemi
{
    inline namespace v1
    {
        struct hemi_point_t
        {
            in_json::in<size_t> point_no;
            std::string_view name;
            in_json::in<double> distance;
            in_json::in<double> contribution_diff;
            std::vector<in_json::in<double>> pos;
        };

        class hemi_data_t
        {
          public:
            std::vector<hemi_point_t> hemi_points;
            std::vector<hemi_point_t> trapped_points;

          private:
            hemi_data_t(std::string&& source) : source_{std::move(source)} {}
            std::string source_;

            friend hemi_data_t parse(std::string&& source);
        };

        hemi_data_t parse(std::string&& source);
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
