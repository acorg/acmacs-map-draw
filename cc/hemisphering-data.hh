#pragma once

#include <vector>

#include "acmacs-base/string.hh"

// ----------------------------------------------------------------------

namespace in_json
{
    inline namespace v1
    {
        template <typename T> struct in
        {
            std::string_view value;

            in() = default;
            in(std::string_view source) : value{source} {}
            in& operator=(std::string_view source) { value = source; return *this; }
            operator T() const { return string::from_chars<T>(value); }
        };
    }
}

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
