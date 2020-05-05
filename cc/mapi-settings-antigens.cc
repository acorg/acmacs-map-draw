#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/report-antigens.hh"

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
    else if (key == "reference"sv) {
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
    return true;

} // check_reference

// ----------------------------------------------------------------------

template <typename AgSr> static bool check_passage(const AgSr& ag_sr, acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& value)
{
    using namespace std::string_view_literals;

    const auto passage_group = [&ag_sr, &indexes](std::string_view passage_key) -> bool {
        if (passage_key == "egg"sv)
            ag_sr.filter_egg(indexes, acmacs::chart::reassortant_as_egg::no);
        else if (passage_key == "cell"sv)
            ag_sr.filter_cell(indexes);
        else if (passage_key == "reassortant"sv)
            ag_sr.filter_reassortant(indexes);
        else
            return false;
        return true;
    };

    if (passage_group(key))
        ; // processed
    else if (key == "passage"sv) {
        value.visit([passage_group]<typename Val>(const Val& val) {
            if constexpr (std::is_same_v<Val, rjson::v3::detail::string>) {
                if (passage_group(val.template to<std::string_view>()))
                    ; // processed
                else
                    throw std::exception{};
            }
            else if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
                throw std::exception{};
            }
            else
                throw std::exception{};
        });
    }
    else
        return false;
    return true;

} // check_passage

// ----------------------------------------------------------------------

acmacs::chart::PointIndexList acmacs::mapi::v1::Settings::select_antigens() const
{
    using namespace std::string_view_literals;

    if (const auto& select_clause = getenv("select"sv); !select_clause.is_null()) {
        auto antigens = chart_draw().chart().antigens();
        auto indexes = antigens->all_indexes();
        bool report{false};
        size_t report_threshold{20};

        try {
            select_clause.visit([&indexes, &antigens, &report, &report_threshold]<typename Value>(const Value& select_clause_v) {
                if constexpr (std::is_same_v<Value, rjson::v3::detail::string>) {
                    if (check_reference(*antigens, indexes, select_clause_v.template to<std::string_view>(), rjson::v3::const_true))
                        ; // processed
                    else if (check_passage(*antigens, indexes, select_clause_v.template to<std::string_view>(), rjson::v3::const_true))
                        ; // processed
                    else
                        throw unrecognized{};
                }
                else if constexpr (std::is_same_v<Value, rjson::v3::detail::object>) {
                    for (const auto& [key, value] : select_clause_v) {
                        if (check_reference(*antigens, indexes, key, value))
                            ; // processed
                        else if (key == "passage"sv) {
                            if (!check_passage(*antigens, indexes, key, value))
                                throw unrecognized{fmt::format("unrecognized passage clause: {}", value)};
                        }
                        else if (key == "report"sv)
                            report = value.template to<bool>();
                        else if (key == "report_threshold"sv)
                            report_threshold = value.template to<size_t>();
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
            AD_INFO("{} antigens selected with {}", indexes.size(), select_clause);
            report_antigens(indexes, chart_draw(), report_threshold);
        }
        return indexes;
    }
    else
        throw unrecognized{"no \"select\" clause"};

} // acmacs::mapi::v1::Settings::select_antigens

// ----------------------------------------------------------------------

acmacs::chart::PointIndexList acmacs::mapi::v1::Settings::select_sera() const
{
    using namespace std::string_view_literals;

    if (const auto& select_clause = getenv("select"); !select_clause.is_null()) {
        auto sera = chart_draw().chart().sera();
        auto indexes = sera->all_indexes();
        bool report{false};
        size_t report_threshold{20};

        try {
            select_clause.visit([&indexes, &sera, &report, &report_threshold]<typename Value>(const Value& select_clause_v) {
                if constexpr (std::is_same_v<Value, rjson::v3::detail::string>) {
                    if (select_clause_v.template to<std::string_view>() == "all"sv)
                        ; // processed
                    else if (check_passage(*sera, indexes, select_clause_v.template to<std::string_view>(), rjson::v3::const_true))
                        ; // processed
                    else
                        throw unrecognized{};
                }
                else if constexpr (std::is_same_v<Value, rjson::v3::detail::object>) {
                    for (const auto& [key, value] : select_clause_v) {
                        if (key == "all"sv)
                            ;   // processed
                        else if (key == "passage"sv) {
                            if (!check_passage(*sera, indexes, key, value))
                                throw unrecognized{fmt::format("unrecognized passage clause: {}", value)};
                        }
                        else if (key == "report"sv)
                            report = value.template to<bool>();
                        else if (key == "report_threshold"sv)
                            report_threshold = value.template to<size_t>();
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
            AD_INFO("{} sera selected with {}", indexes.size(), select_clause);
            report_sera(indexes, chart_draw(), report_threshold);
        }

        return indexes;
    }
    else
        throw unrecognized{"no \"select\" clause"};

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
