#include "acmacs-base/rjson-v3-helper.hh"
#include "acmacs-base/string-compare.hh"
#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/select-filter.hh"

// ----------------------------------------------------------------------

void acmacs::mapi::v1::Settings::time_series_generate(std::string_view filename_pattern, const acmacs::time_series::slot& slot) const
{
    using namespace std::string_view_literals;
    const auto filename{time_series_substitute(filename_pattern, slot)};
    make_pdf(filename, rjson::v3::read_number(getenv("width"sv), 800.0), false);
    AD_INFO("time series {}: {}", acmacs::time_series::numeric_name(slot), filename);

} // acmacs::mapi::v1::Settings::time_series_generate

// ----------------------------------------------------------------------

std::string acmacs::mapi::v1::Settings::time_series_substitute(std::string_view pattern, const acmacs::time_series::slot& slot) const
{
    try {
        fmt::dynamic_format_arg_store<fmt::format_context> fmt_args;
        chart_metadata(fmt_args, chart_draw().chart(0));
        fmt_args.push_back(fmt::arg("ts_text", acmacs::time_series::text_name(slot)));
        fmt_args.push_back(fmt::arg("ts_numeric", acmacs::time_series::numeric_name(slot)));
        return fmt::vformat(pattern, fmt_args);
    }
    catch (std::exception& err) {
        AD_ERROR("fmt cannot substitute in \"{}\": {}", pattern, err);
        throw;
    }

} // acmacs::mapi::v1::Settings::time_series_substitute

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_time_series()
{
    using namespace std::string_view_literals;
    const auto ts{time_series_settings()};
    if (!ts.filename_pattern.empty()) {
        auto antigens{chart_draw().chart().antigens()};
        auto indexes = antigens->test_indexes();
        acmacs::map_draw::select::filter::shown_in(chart_draw(), indexes);
        if (!ts.shown_on_all.empty())
            indexes.remove(ReverseSortedIndexes{*ts.shown_on_all});

        for (const auto& ts_slot : ts.series) {
            auto to_hide{indexes};
            antigens->filter_date_not_in_range(to_hide, date::display(ts_slot.first), date::display(ts_slot.after_last));

            acmacs::PointStyleModified style;
            style.shown(false);
            chart_draw().modify(to_hide, style);

            if (!ts.title.empty()) {
                auto& title_element = title();
                title_element.remove_all_lines();
                for (const auto& line : ts.title)
                    title_element.add_line(time_series_substitute(line, ts_slot));
            }
            time_series_generate(ts.filename_pattern, ts_slot);

            style.shown(true);
            chart_draw().modify(to_hide, style);
        }
    }
    return true;

} // acmacs::mapi::v1::Settings::apply_time_series

// ----------------------------------------------------------------------

acmacs::mapi::v1::Settings::time_series_data acmacs::mapi::v1::Settings::time_series_settings() const
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

    const auto ts_stat = acmacs::time_series::stat(ts_params, chart_draw().chart().antigens()->all_dates(acmacs::chart::Antigens::include_reference::no));
    const auto [first, after_last] = acmacs::time_series::suggest_start_end(ts_params, ts_stat);
    if (ts_params.first == date::invalid_date())
        ts_params.first = first;
    if (ts_params.after_last == date::invalid_date())
        ts_params.after_last = after_last;

    auto series = acmacs::time_series::make(ts_params);

    if (!ts_stat.counter().empty())
        AD_INFO("time series full range {} .. {}", ts_stat.counter().begin()->first, ts_stat.counter().rbegin()->first);
    AD_INFO("time series suggested  {} .. {}", first, after_last);
    AD_INFO("time series used       {} .. {} ({} slots)", ts_params.first, ts_params.after_last, series.size());
    if (rjson::v3::read_bool(getenv("report"sv), false))
        AD_INFO("time series report:\n{}", ts_stat.report("    {value}  {counter:6d}\n"));

    const auto filename_pattern = rjson::v3::read_string(getenv("output"sv, toplevel_only::no, throw_if_partial_substitution::no));
    if (!filename_pattern.has_value())
        AD_WARNING("Cannot make time series: no \"output\" in {}", getenv_toplevel());

    std::vector<std::string_view> title;
    getenv("title"sv, toplevel_only::yes, throw_if_partial_substitution::no).visit([&title]<typename Val>(const Val& lines) {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
            for (const auto& line : lines)
                title.push_back(line.template to<std::string_view>());
        }
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::string>)
            title.push_back(lines.template to<std::string_view>());
        else if constexpr (!std::is_same_v<Val, rjson::v3::detail::null>)
            throw error{fmt::format("\"time-series\" \"title\": unrecognized: {} (expected string or array of strings)", lines)};
    });

    auto shown_on_all{select_antigens(getenv("shown-on-all"sv), if_null::empty)};

    return {std::string{*filename_pattern}, std::move(title), std::move(series), std::move(shown_on_all)};

} // acmacs::mapi::v1::Settings::time_series_settings

// ----------------------------------------------------------------------




// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
