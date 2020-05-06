#include "acmacs-base/date.hh"
#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/report-antigens.hh"
#include "acmacs-map-draw/select-filter.hh"

enum class throw_if_unprocessed { no, yes };

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_antigens()
{
    using namespace std::string_view_literals;

    const auto indexes = select_antigens();
    chart_draw().modify(indexes, style_from_toplevel_environment(), drawing_order_from_toplevel_environment());

    return true;

    // const auto verbose = rjson::get_or(args(), "report", false);
    // const auto report_names_threshold = rjson::get_or(args(), "report_names_threshold", 30UL);
    // if (const auto& select = args()["select"]; !select.is_null()) {
    //     const auto indices = SelectAntigens(acmacs::verbose_from(verbose), report_names_threshold).select(aChartDraw, select);
    //     const auto styl = style();

    //     // AD_DEBUG("{}", styl);

    //     aChartDraw.modify(indices, styl, drawing_order());
    //     if (const auto& label = args()["label"]; !label.is_null())
    //         add_labels(aChartDraw, indices, 0, label);
    //     if (const auto& legend = args()["legend"]; !legend.is_null())
    //         add_legend(aChartDraw, indices, styl, legend);
    // }
    // else {
    //     throw unrecognized_mod{"no \"select\" in \"antigens\" mod: " + rjson::format(args())};
    // }

} // acmacs::mapi::v1::Settings::apply_antigens

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_sera()
{
    const auto indexes = select_sera();
    chart_draw().modify_sera(indexes, style_from_toplevel_environment(), drawing_order_from_toplevel_environment());

    return true;

    // const auto verbose = rjson::get_or(args(), "report", false);
    // if (const auto& select = args()["select"]; !select.is_null()) {
    //     auto& projection = aChartDraw.projection();
    //     if (auto relative = args().get("relative"); !relative.is_null()) {
    //         auto layout = aChartDraw.layout();
    //         for (auto index : SelectSera(acmacs::verbose_from(verbose)).select(aChartDraw, select)) {
    //             const auto coord = layout->at(index + aChartDraw.number_of_antigens());
    //             projection.move_point(index + aChartDraw.number_of_antigens(), acmacs::PointCoordinates(coord.x() + relative[0].to<double>(), coord.y() + relative[1].to<double>()));
    //         }
    //     }
    //     else if (const auto& flip_line = args()["flip_over_line"]; !flip_line.is_null()) {
    //         acmacs::PointCoordinates from{flip_line["from"][0].to<double>(), flip_line["from"][1].to<double>()}, to{flip_line["to"][0].to<double>(), flip_line["to"][1].to<double>()};
    //         if (!rjson::get_or(flip_line, "transform", true)) {
    //             const auto transformation = aChartDraw.transformation().inverse();
    //             from = transformation.transform(from);
    //             to = transformation.transform(to);
    //         }
    //         const acmacs::LineDefinedByEquation line(from, to);
    //         auto layout = aChartDraw.layout();
    //         for (auto index : SelectSera(acmacs::verbose_from(verbose)).select(aChartDraw, select)) {
    //             const auto flipped = line.flip_over(layout->at(index + aChartDraw.number_of_antigens()), 1.0);
    //             projection.move_point(index + aChartDraw.number_of_antigens(), flipped);
    //         }
    //     }
    //     else {
    //         const auto move_to = get_move_to(aChartDraw, verbose);
    //         for (auto index : SelectSera(acmacs::verbose_from(verbose)).select(aChartDraw, select)) {
    //             projection.move_point(index + aChartDraw.number_of_antigens(), move_to);
    //         }
    //     }
    // }
    // else {
    //     throw unrecognized_mod{"no \"select\" in \"move_sera\" mod: " + rjson::format(args())};
    // }

} // acmacs::mapi::v1::Settings::apply_sera

// ----------------------------------------------------------------------

template <typename AgSr> static bool check_reference(const AgSr& ag_sr, acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& expected_bool)
{
    using namespace std::string_view_literals;

    if (key == "all"sv) {
        if (!expected_bool.is_bool() || !expected_bool.template to<bool>())
            throw acmacs::mapi::unrecognized{fmt::format("unsupported \"{}\" value of {}", key, expected_bool)};
    }
    else if constexpr (std::is_same_v<AgSr, acmacs::chart::Antigens>) {
        if (key == "reference"sv) {
            if (!expected_bool.is_bool() || !expected_bool.template to<bool>())
                throw acmacs::mapi::unrecognized{fmt::format("unsupported \"{}\" value of {}", key, expected_bool)};
            ag_sr.filter_reference(indexes);
        }
        else if (key == "test"sv) {
            if (!expected_bool.is_bool() || !expected_bool.template to<bool>())
                throw acmacs::mapi::unrecognized{fmt::format("unsupported \"{}\" value of {}", key, expected_bool)};
            ag_sr.filter_test(indexes);
        }
        else
            return false;
    }
    else
        return false;
    return true;

} // check_reference

// ----------------------------------------------------------------------

template <typename AgSr> static void check_index(const AgSr& ag_sr, acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& value)
{
    using namespace std::string_view_literals;

    const auto report_error = [&value]() { throw acmacs::mapi::unrecognized{fmt::format("unrecognized index clause: {}", value)}; };

    const auto keep = [&ag_sr](size_t index_to_keep, acmacs::chart::PointIndexList& ind) {
        if (index_to_keep < ag_sr.size()) {
            ind.erase_except(index_to_keep);
        }
        else {
            ind.clear();
            AD_WARNING("index is out of range: {} (range: 0 .. {})", index_to_keep, ag_sr.size() - 1);
        }
    };

    if (key != "index"sv)
        AD_WARNING("Selecting antigen/serum with \"{}\" deprecated, use \"index\"", key);

    value.visit([&indexes, keep, report_error]<typename Val>(const Val& val) {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::number>) {
            keep(val.template to<size_t>(), indexes);
        }
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
            const auto orig{indexes};
            bool first{true};
            for (const auto& index_to_keep : val) {
                if (first) {
                    keep(index_to_keep.template to<size_t>(), indexes);
                    first = false;
                }
                else {
                    auto ind{orig};
                    keep(index_to_keep.template to<size_t>(), ind);
                    indexes.extend(ind);
                }
            }
        }
        else
            report_error();
    });

} // check_index

// ----------------------------------------------------------------------

template <typename AgSr> static void check_name(const AgSr& ag_sr, acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& value)
{
    using namespace std::string_view_literals;

    const auto report_error = [&value]() { throw acmacs::mapi::unrecognized{fmt::format("unrecognized name clause: {}", value)}; };

    if (key != "name"sv)
        AD_WARNING("Selecting antigen/serum with \"{}\" deprecated, use \"name\"", key);

    value.visit([&ag_sr, &indexes, report_error]<typename Val>(const Val& val) {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::string>) {
            acmacs::map_draw::select::filter::name_in(ag_sr, indexes, val.template to<std::string_view>());
        }
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
            const auto orig{indexes};
            bool first{true};
            for (const auto& name : val) {
                if (first) {
                    acmacs::map_draw::select::filter::name_in(ag_sr, indexes, name.template to<std::string_view>());
                    first = false;
                 }
                else {
                    auto ind{orig};
                    acmacs::map_draw::select::filter::name_in(ag_sr, ind, name.template to<std::string_view>());
                    indexes.extend(ind);
                }
            }
        }
        else
            report_error();
    });

} // check_name

// ----------------------------------------------------------------------

static inline void check_date(const acmacs::chart::Chart& /*chart*/, const acmacs::chart::Antigens& antigens, acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& value)
{
    using namespace std::string_view_literals;

    const auto report_error = [&value]() { throw acmacs::mapi::unrecognized{fmt::format("unrecognized date clause: {}", value)}; };

    if (key != "date"sv)
        AD_WARNING("Selecting antigen/serum with \"{}\" deprecated, use \"date\"", key);

    value.visit([&antigens, &indexes, report_error, &value]<typename Val>(const Val& val) {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::string>) {
            report_error();
        }
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
            antigens.filter_date_range(indexes, val[0].template to<std::string_view>(), val[1].template to<std::string_view>());
        }
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::object>) {
            std::string first_date, last_date;
            const auto update_first = [&first_date](std::string_view date) {
                if (first_date.empty() || date > first_date)
                    first_date.assign(date);
            };
            const auto update_last = [&last_date](std::string_view date) {
                if (last_date.empty() || date < last_date)
                    last_date.assign(date);
            };
            for (const auto& [key, value] : val) {
                if (key == "younger_than_days"sv)
                    update_first(date::display(date::days_ago(date::today(), value.template to<int>())));
                else if (key == "older_than_days"sv)
                    update_last(date::display(date::days_ago(date::today(), value.template to<int>())));
                else if (key == "before"sv)
                    update_last(value.template to<std::string_view>());
                else if (key == "after"sv)
                    update_first(value.template to<std::string_view>());
                else
                    AD_WARNING("unrecognized \"date\" key \"{}\"", key);
            }
            AD_INFO("date range for selecting antigens/sera by {}: \"{}\" .. \"{}\"", value, first_date, last_date);
            antigens.filter_date_range(indexes, first_date, last_date);
        }
        else
            report_error();
    });

} // check_date(antigens)

// ----------------------------------------------------------------------

static inline void check_date(const acmacs::chart::Chart& chart, const acmacs::chart::Sera& sera, acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& value)
{
    chart.set_homologous(acmacs::chart::find_homologous::relaxed);

    auto antigens = chart.antigens();
    auto antigen_indexes = antigens->all_indexes();
    check_date(chart, *antigens, antigen_indexes, key, value);

    auto homologous_filtered_out_by_date_range = [&sera, &antigen_indexes](auto serum_index) -> bool {
        for (auto antigen_index : sera.at(serum_index)->homologous_antigens()) {
            if (antigen_indexes.contains(antigen_index))
                return false;   // homologous antigen selected by date range, do not remove this serum from indexes
        }
        return true;
    };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), homologous_filtered_out_by_date_range), indexes.end());

} // check_date(sera)

// ----------------------------------------------------------------------

template <typename AgSr> static void check_location(const AgSr& ag_sr, acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& value)
{
    using namespace std::string_view_literals;

    const auto report_error = [key, &value]() { throw acmacs::mapi::unrecognized{fmt::format("unrecognized \"{}\" clause: {}", key, value)}; };

    const auto location_one = [&ag_sr, key](acmacs::chart::PointIndexList& ind, std::string_view name) {
        if (key == "country"sv || key == "countries"sv)
            ag_sr.filter_country(ind, string::upper(name));
        else if (key == "continent"sv || key == "continents"sv)
            ag_sr.filter_continent(ind, string::upper(name));
        else if (key == "location"sv || key == "locations"sv)
            acmacs::map_draw::select::filter::location_in(ag_sr, ind, string::upper(name));
        else
            AD_WARNING("unrecognized location  key \"{}\"", key); // should never come here actually
    };

    value.visit([&indexes, report_error, location_one]<typename Val>(const Val& val) {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::string>) {
            location_one(indexes, val.template to<std::string_view>());
        }
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
            const auto orig{indexes};
            bool first{true};
            for (const auto& name : val) {
                if (first) {
                    location_one(indexes, name.template to<std::string_view>());
                    first = false;
                }
                else {
                    auto ind{orig};
                    location_one(ind, name.template to<std::string_view>());
                    indexes.extend(ind);
                }
            }
        }
        else
            report_error();
    });

} // check_location

// ----------------------------------------------------------------------

template <typename AgSr> static bool check_passage(const AgSr& ag_sr, acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& value, throw_if_unprocessed tiu)
{
    using namespace std::string_view_literals;

    const auto report_error = [tiu, &value](throw_if_unprocessed tiu2 = throw_if_unprocessed::no) -> bool {
        if (tiu == throw_if_unprocessed::yes || tiu2 == throw_if_unprocessed::yes)
            throw acmacs::mapi::unrecognized{fmt::format("unrecognized passage clause: {}", value)};
        return false;
    };

    enum class basic { no, yes };
    const auto passage_group = [&ag_sr, report_error](std::string_view passage_key, acmacs::chart::PointIndexList& ind, basic bas) -> bool {
        // AD_DEBUG("passage_group \"{}\"", passage_key);
        if (passage_key == "egg"sv)
            ag_sr.filter_egg(ind, acmacs::chart::reassortant_as_egg::no);
        else if (passage_key == "cell"sv)
            ag_sr.filter_cell(ind);
        else if (passage_key == "reassortant"sv)
            ag_sr.filter_reassortant(ind);
        else if (bas == basic::no) {
            if (!passage_key.empty() && passage_key[0] == '~') {
                const std::regex re{std::next(std::begin(passage_key), 1), std::end(passage_key), acmacs::regex::icase};
                ag_sr.filter_passage(ind, re);
            }
            else
                ag_sr.filter_passage(ind, passage_key);
        }
        else if (bas == basic::no)
            return report_error();
        else
            return false;
        return true;
    };

    if (passage_group(key, indexes, basic::yes))
        ; // processed
    else if (key == "passage"sv) {
        value.visit([passage_group, &indexes, report_error]<typename Val>(const Val& val) {
            if constexpr (std::is_same_v<Val, rjson::v3::detail::string>) {
                if (passage_group(val.template to<std::string_view>(), indexes, basic::no))
                    ; // processed
                else
                    report_error(throw_if_unprocessed::yes);
            }
            else if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
                const auto orig{indexes};
                bool first{true};
                for (const auto& psg : val) {
                    if (first) {
                        passage_group(psg.template to<std::string_view>(), indexes, basic::no);
                        first = false;
                    }
                    else {
                        auto ind{orig};
                        passage_group(psg.template to<std::string_view>(), ind, basic::no);
                        indexes.extend(ind);
                    }
                }
            }
            else
                report_error(throw_if_unprocessed::yes);
        });
    }
    else
        return report_error();
    return true;

} // check_passage

// ----------------------------------------------------------------------

template <typename AgSr> acmacs::chart::PointIndexList acmacs::mapi::v1::Settings::select(const AgSr& ag_sr) const
{
    using namespace std::string_view_literals;

    if (const auto& select_clause = getenv("select"sv); !select_clause.is_null()) {
        auto indexes = ag_sr.all_indexes();
        bool report{false};
        size_t report_threshold{20};

        try {
            select_clause.visit([&indexes, &ag_sr, &report, &report_threshold, this]<typename Value>(const Value& select_clause_v) {
                if constexpr (std::is_same_v<Value, rjson::v3::detail::string>) {
                    if (check_reference(ag_sr, indexes, select_clause_v.template to<std::string_view>(), rjson::v3::const_true))
                        ; // processed
                    else if (check_passage(ag_sr, indexes, select_clause_v.template to<std::string_view>(), rjson::v3::const_true, throw_if_unprocessed::no))
                        ; // processed
                    else
                        throw unrecognized{};
                }
                else if constexpr (std::is_same_v<Value, rjson::v3::detail::object>) {
                    for (const auto& [key, value] : select_clause_v) {
                        if (check_reference(ag_sr, indexes, key, value))
                            ; // processed
                        else if (key == "date"sv || key == "dates"sv || key == "date_range"sv)
                            check_date(chart_draw().chart(), ag_sr, indexes, key, value);
                        else if (key == "index"sv || key == "indexes"sv || key == "indices"sv)
                            check_index(ag_sr, indexes, key, value);
                        else if (key == "name"sv || key == "names"sv)
                            check_name(ag_sr, indexes, key, value);
                        else if (key == "passage"sv)
                            check_passage(ag_sr, indexes, key, value, throw_if_unprocessed::yes);
                        else if (key == "report"sv)
                            report = value.template to<bool>();
                        else if (key == "report_threshold"sv)
                            report_threshold = value.template to<size_t>();
                        else if (key == "country"sv || key == "countries"sv || key == "continent"sv || key == "continents"sv || key == "location"sv || key == "locations"sv)
                            check_location(ag_sr, indexes, key, value);
                        else if (!key.empty() && key[0] != '?')
                            AD_WARNING("unrecognized \"select\" key: \"{}\"", key);
                    }
                }
                else
                    throw unrecognized{};
            });
        }
        catch (std::exception& err) {
            throw unrecognized{AD_FORMAT("unrecognized \"select\" clause: {}: {}", select_clause, err)};
        }
        if (report) {
            if constexpr (std::is_same_v<AgSr, acmacs::chart::Antigens>) {
                AD_INFO("{} antigens selected with {}", indexes.size(), select_clause);
                report_antigens(indexes, chart_draw(), report_threshold);
            }
            else {
                AD_INFO("{} sera selected with {}", indexes.size(), select_clause);
                report_sera(indexes, chart_draw(), report_threshold);
            }
        }
        return indexes;
    }
    else
        throw unrecognized{"no \"select\" clause"};

} // acmacs::mapi::v1::Settings::select

// ----------------------------------------------------------------------

acmacs::chart::PointIndexList acmacs::mapi::v1::Settings::select_antigens() const
{
    return select(*chart_draw().chart().antigens());

} // acmacs::mapi::v1::Settings::select_antigens

// ----------------------------------------------------------------------

acmacs::chart::PointIndexList acmacs::mapi::v1::Settings::select_sera() const
{
    return select(*chart_draw().chart().sera());

} // acmacs::mapi::v1::Settings::select_sera

// ----------------------------------------------------------------------

acmacs::PointStyleModified acmacs::mapi::v1::Settings::style_from_toplevel_environment() const
{
    using namespace std::string_view_literals;
    acmacs::PointStyleModified style;
    for (const auto& [key, val] : getenv_toplevel()) {
        // AD_DEBUG("apply_antigens {}: {}", key, val);
        if (key == "fill"sv)
            style.fill(color(val));
        else if (key == "outline"sv)
            style.outline(color(val));
        else if (key == "show"sv)
            style.shown(substitute_to_bool(val));
        else if (key == "hide"sv)
            style.shown(!substitute_to_bool(val));
        else if (key == "shape"sv)
            style.shape(PointShape{substitute_to_string(val)});
        else if (key == "size"sv)
            style.size(Pixels{substitute_to_double(val)});
        else if (key == "outline_width"sv)
            style.outline_width(Pixels{substitute_to_double(val)});
        else if (key == "aspect"sv)
            style.aspect(Aspect{substitute_to_double(val)});
        else if (key == "rotation"sv)
            style.rotation(Rotation{substitute_to_double(val)});
        else if (key == "fill_saturation"sv) {
            AD_WARNING("\"fill_saturation\" not implemented");
            // style.fill = acmacs::color::Modifier{acmacs::color::adjust_saturation{val.to<double>()}};
        }
        else if (key == "fill_brightness"sv) {
            AD_WARNING("\"fill_brightness not implemented");
            // style.fill = acmacs::color::Modifier{acmacs::color::adjust_brightness{val.to<double>()}};
        }
        else if (key == "outline_saturation"sv) {
            AD_WARNING("\"outline_saturation\" not implemented");
            // style.outline = acmacs::color::Modifier{acmacs::color::adjust_saturation{val.to<double>()}};
        }
        else if (key == "outline_brightness"sv) {
            AD_WARNING("\"outline_brightness\" not implemented");
            // style.outline = acmacs::color::Modifier{acmacs::color::adjust_brightness{val.to<double>()}};
        }
    }
    return style;

} // acmacs::mapi::v1::Settings::style_from_toplevel_environment

// ----------------------------------------------------------------------

Color acmacs::mapi::v1::Settings::color(const rjson::v3::value& value) const
{
    using namespace std::string_view_literals;
    const auto make_color = [](std::string_view source) -> Color {
        if (source == "passage"sv) {
            AD_WARNING("\"passage\" color not implemented");
            return PINK;
        }
        else
            return source;
    };

    try {
        return std::visit(
            [make_color]<typename Value>(const Value& val) -> Color {
                if constexpr (std::is_same_v<Value, const rjson::v3::value*>) {
                    if (val->is_string()) {
                        return make_color(val->template to<std::string_view>());
                    }
                    else
                        throw std::exception{};
                }
                else
                    return make_color(val);
            },
            substitute(value));
    }
    catch (std::exception&) {
        throw unrecognized{AD_FORMAT("canot get color from {}", value)};
    }

} // acmacs::mapi::v1::Settings::color

// ----------------------------------------------------------------------

PointDrawingOrder acmacs::mapi::v1::Settings::drawing_order_from_toplevel_environment() const
{
    using namespace std::string_view_literals;
    PointDrawingOrder result{PointDrawingOrder::NoChange};
    for (const auto& [key, val] : getenv_toplevel()) {
        if (key == "order"sv) {
            if (const auto order = substitute_to_string(val); order == "raise"sv)
                result = PointDrawingOrder::Raise;
            else if (order == "lower"sv)
                result = PointDrawingOrder::Lower;
            else
                AD_WARNING("unrecognized order value: {}", val);
        }
        else if ((key == "raise"sv || key == "raise_"sv || key == "_raise"sv) && val.to<bool>())
            result = PointDrawingOrder::Raise;
        else if ((key == "lower"sv || key == "lower_"sv || key == "_lower"sv) && val.to<bool>())
            result = PointDrawingOrder::Lower;
        if (result != PointDrawingOrder::NoChange)
            break;
    }
    return result;

} // acmacs::mapi::v1::Settings::drawing_order_from_toplevel_environment

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
