#include "acmacs-base/time-series.hh"
#include "acmacs-map-draw/mapi-settings.hh"

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_time_series()
{
    using namespace std::string_view_literals;

    auto ts_params = getenv("interval"sv).visit([]<typename Val>(const Val& val) -> acmacs::time_series::parameters {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::object>) {
            for (const auto& interval_n : {"year"sv, "month"sv, "week"sv, "day"sv}) {
                if (const auto& num = val[interval_n]; !num.is_null())
                    return {interval_n, num.template to<date::period_diff_t>()};
            }
            AD_WARNING("unrecognized interval specification: {}, month assumed", val);
            return {"month"sv};
        }
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::string>) {
            return {val.template to<std::string_view>()};
        }
        else {
            AD_WARNING("unrecognized interval specification: {}, month assumed", val);
            return {"month"sv};
        }
    });

    if (const auto& start = getenv("start"sv); !start.is_null())
        ts_params.first = date::from_string(start.to<std::string_view>(), date::allow_incomplete::yes, date::throw_on_error::yes);
    if (const auto& end = getenv("end"sv); !end.is_null())
        ts_params.after_last = date::from_string(end.to<std::string_view>(), date::allow_incomplete::yes, date::throw_on_error::yes);


    // rjson::v3::copy_if_not_null(getenv("report"sv), param.report);

    return true;

} // acmacs::mapi::v1::Settings::apply_time_series

// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
