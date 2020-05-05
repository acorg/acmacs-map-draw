#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/report-antigens.hh"

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_antigens()
{
    const auto indexes = select_antigens();

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

acmacs::chart::Indexes acmacs::mapi::v1::Settings::select_antigens() const
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
                    if (select_clause_v.template to<std::string_view>() == "all"sv)
                        ; // do nothing
                    else if (select_clause_v.template to<std::string_view>() == "reference"sv)
                        antigens->filter_reference(indexes);
                    else if (select_clause_v.template to<std::string_view>() == "test"sv)
                        antigens->filter_test(indexes);
                    else
                        throw unrecognized{};
                }
                else if constexpr (std::is_same_v<Value, rjson::v3::detail::object>) {
                    for (const auto& [key, value] : select_clause_v) {
                        if (key == "all"sv) {
                            if (!value.is_bool() || !value)
                                throw unrecognized{fmt::format("unsupported value of \"{}\"", key)};
                        }
                        else if (key == "reference"sv) {
                            if (!value.is_bool() || !value)
                                throw unrecognized{fmt::format("unsupported value of \"{}\"", key)};
                            antigens->filter_reference(indexes);
                        }
                        else if (key == "test"sv) {
                            if (!value.is_bool() || !value)
                                throw unrecognized{fmt::format("unsupported value of \"{}\"", key)};
                            antigens->filter_test(indexes);
                        }
                        else if (key == "report"sv)
                            report = true;
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
            report_antigens(std::begin(indexes), std::end(indexes), chart_draw(), report_threshold);
        }
        return indexes;
    }
    else
        throw unrecognized{"no \"select\" clause"};

} // acmacs::mapi::v1::Settings::select_antigens

// ----------------------------------------------------------------------

acmacs::chart::Indexes acmacs::mapi::v1::Settings::select_sera() const
{
    if (const auto& select_clause = getenv("select"); !select_clause.is_null()) {
        auto sera = chart_draw().chart().sera();
        auto indexes = sera->all_indexes();
        return indexes;
    }
    else
        throw unrecognized{"no \"select\" clause"};

} // acmacs::mapi::v1::Settings::select_sera

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
