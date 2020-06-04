#include "acmacs-base/time-series.hh"
#include "acmacs-base/rjson-v3-helper.hh"
#include "acmacs-base/string-compare.hh"
#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

//  "?start": "2019-01", "?end": "2019-11",
//  "interval": {"month": 1}, "?": "month, week, year, day (interval: month also supported)",
//  "output": "/path/name-{ts-name}.pdf",
// "shown-on-all": <Select Antigens>, -- reference antigens and sera are shown on all maps, select here other antigens to show on all the maps
//  "report": true

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

    const auto& chart_access = chart_draw().chart(0); // can draw just the chart 0 // get_chart(getenv("chart"sv), 0);
    const auto ts_stat = acmacs::time_series::stat(ts_params, chart_access.chart().antigens()->all_dates());
    const auto [first, after_last] = acmacs::time_series::suggest_start_end(ts_params, ts_stat);

    auto series = acmacs::time_series::make(ts_params);

    if (!ts_stat.counter().empty())
        AD_INFO("time series full range {} .. {}", ts_stat.counter().begin()->first, ts_stat.counter().rbegin()->first);
    AD_INFO("time series suggested  {} .. {}", first, after_last);
    AD_INFO("time series used       {} .. {}", ts_params.first, ts_params.after_last);
    if (rjson::v3::read_bool(getenv("report"sv), false)) {
        AD_INFO("time series report:\n{}", ts_stat.report("    {value}  {counter:6d}\n"));
    }

    if (const auto filename_pattern = rjson::v3::read_string(getenv("output"sv, toplevel_only::no, throw_if_partial_substitution::no)); filename_pattern.has_value()) {
        auto filename{substitute_chart_metadata(*filename_pattern, chart_access)};
        if (!acmacs::string::endswith_ignore_case(filename, ".pdf"sv))
            filename = fmt::format("{}.pdf", filename);
        chart_draw().calculate_viewport();
        chart_draw().draw(filename, rjson::v3::read_number(getenv("width"sv), 800.0), report_time::no);
    }
    else
        AD_WARNING("Cannot make time series: no \"output\" in {}", getenv_toplevel());
    return true;

} // acmacs::mapi::v1::Settings::apply_time_series

// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
