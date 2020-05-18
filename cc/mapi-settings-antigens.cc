#include "acmacs-base/date.hh"
#include "acmacs-base/string-compare.hh"
#include "acmacs-whocc-data/labs.hh"
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

template <typename Conv, typename Callback> void check_disjunction(acmacs::chart::PointIndexList& indexes, const rjson::v3::detail::array& arr, Callback&& callback)
{
    const auto orig{indexes};
    bool first{true};
    for (const auto& name : arr) {
        if (first) {
            callback(indexes, name.template to<Conv>());
            first = false;
        }
        else {
            auto ind{orig};
            callback(ind, name.template to<Conv>());
            indexes.extend(ind);
        }
    }

} // check_disjunction

// ----------------------------------------------------------------------

template <typename Conv, typename Callback> void check_conjunction(acmacs::chart::PointIndexList& indexes, const rjson::v3::detail::array& arr, Callback&& callback)
{
    for (const auto& name : arr)
        callback(indexes, name.template to<Conv>());

} // check_disjunction

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

    const auto report_error = [&value]() { throw acmacs::mapi::unrecognized{fmt::format("unrecognized \"index\" clause: {}", value)}; };

    const auto keep = [&ag_sr](acmacs::chart::PointIndexList& ind, size_t index_to_keep) {
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
        if constexpr (std::is_same_v<Val, rjson::v3::detail::number>)
                             keep(indexes, val.template to<size_t>());
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::array>)
            check_disjunction<size_t>(indexes, val, keep);
        else
            report_error();
    });

} // check_index

// ----------------------------------------------------------------------

template <typename AgSr> static void check_name(const AgSr& ag_sr, acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& value)
{
    using namespace std::string_view_literals;

    const auto report_error = [&value]() { throw acmacs::mapi::unrecognized{fmt::format("unrecognized \"name\" clause: {}", value)}; };

    const auto name_one = [&ag_sr](acmacs::chart::PointIndexList& ind, std::string_view name) {
        acmacs::map_draw::select::filter::name_in(ag_sr, ind, name);
    };

    if (key != "name"sv)
        AD_WARNING("Selecting antigen/serum with \"{}\" deprecated, use \"name\"", key);

    value.visit([&indexes, name_one, report_error]<typename Val>(const Val& val) {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::string>) {
            name_one(indexes, val.template to<std::string_view>());
        }
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
            check_disjunction<std::string_view>(indexes, val, name_one);
        }
        else
            report_error();
    });

} // check_name

// ----------------------------------------------------------------------

static inline void check_lab(const acmacs::chart::Chart& chart, acmacs::chart::PointIndexList& indexes, std::string_view /*key*/, const rjson::v3::value& value)
{
    using namespace std::string_view_literals;

    const auto report_error = [&value]() { throw acmacs::mapi::unrecognized{fmt::format("unrecognized \"lab\" clause: {}", value)}; };

    const auto lab_one = [&chart](acmacs::chart::PointIndexList& ind, std::string_view lab) {
        if (acmacs::whocc::lab_name_normalize(chart.info()->lab(acmacs::chart::Info::Compute::Yes)) != acmacs::whocc::lab_name_normalize(lab))
            ind.clear();
    };

    value.visit([&indexes, lab_one, report_error]<typename Val>(const Val& val) {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::string>) {
            lab_one(indexes, val.template to<std::string_view>());
        }
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
            check_disjunction<std::string_view>(indexes, val, lab_one);
        }
        else
            report_error();
    });

} // check_name

// ----------------------------------------------------------------------

static inline void check_lineage(const acmacs::chart::Chart& chart, acmacs::chart::PointIndexList& indexes, std::string_view /*key*/, const rjson::v3::value& value)
{
    using namespace std::string_view_literals;

    const auto lineage_is = [&value](const acmacs::virus::lineage_t& chart_lineage) -> bool {
        return value.visit([&chart_lineage, &value]<typename Val>(const Val& val) -> bool {
            if constexpr (std::is_same_v<Val, rjson::v3::detail::string>)
                return acmacs::string::startswith_ignore_case(*chart_lineage, val.template to<std::string_view>());
            else
                throw acmacs::mapi::unrecognized{fmt::format("unrecognized \"lineage\" clause: {}", value)};
        });
    };

    if (chart.info()->virus_type(acmacs::chart::Info::Compute::Yes).h_or_b() != "B"sv || !lineage_is(chart.lineage()))
        indexes.clear();

} // check_name

// ----------------------------------------------------------------------

static inline void check_date(const acmacs::chart::Chart& /*chart*/, const acmacs::chart::Antigens& antigens, acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& value)
{
    using namespace std::string_view_literals;

    const auto report_error = [&value]() { throw acmacs::mapi::unrecognized{fmt::format("unrecognized \"date\" clause: {}", value)}; };

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
                if (key == "younger-than-days"sv || key == "younger_than_days"sv)
                    update_first(date::display(date::days_ago(date::today(), value.template to<int>())));
                else if (key == "older-than-days"sv || key == "older_than_days"sv)
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

    auto homologous_filtered_out = [&sera, &antigen_indexes](auto serum_index) -> bool {
        for (auto antigen_index : sera.at(serum_index)->homologous_antigens()) {
            if (antigen_indexes.contains(antigen_index))
                return false;   // homologous antigen selected by ..., do not remove this serum from indexes
        }
        return true;
    };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), homologous_filtered_out), indexes.end());

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
        if constexpr (std::is_same_v<Val, rjson::v3::detail::string>)
            location_one(indexes, val.template to<std::string_view>());
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::array>)
            check_disjunction<std::string_view>(indexes, val, location_one);
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
    const auto passage_group = [&ag_sr, report_error](acmacs::chart::PointIndexList& ind, std::string_view passage_key, basic bas = basic::no) -> bool {
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

    if (passage_group(indexes, key, basic::yes))
        ; // processed
    else if (key == "passage"sv) {
        value.visit([passage_group, &indexes, report_error]<typename Val>(const Val& val) {
            if constexpr (std::is_same_v<Val, rjson::v3::detail::string>) {
                if (passage_group(indexes, val.template to<std::string_view>(), basic::no))
                    ; // processed
                else
                    report_error(throw_if_unprocessed::yes);
            }
            else if constexpr (std::is_same_v<Val, rjson::v3::detail::array>)
                check_disjunction<std::string_view>(indexes, val, passage_group);
            else
                report_error(throw_if_unprocessed::yes);
        });
    }
    else
        return report_error();
    return true;

} // check_passage

// ----------------------------------------------------------------------

static inline void check_sequence(const ChartSelectInterface& aChartSelectInterface, const acmacs::chart::Antigens& /*antigens*/, acmacs::chart::PointIndexList& indexes, std::string_view key,
                                  const rjson::v3::value& value)
{
    using namespace std::string_view_literals;

    const auto report_error = [key, &value]() { throw acmacs::mapi::unrecognized{fmt::format("unrecognized \"{}\" clause: {}", key, value)}; };

    const auto sequence_one = [&aChartSelectInterface, report_error, key](acmacs::chart::PointIndexList& ind, std::string_view name) {
        if (key == "clade"sv || key == "clades"sv)
            acmacs::map_draw::select::filter::clade(aChartSelectInterface, ind, name);
        else if (key == "amino-acid"sv || key == "amino-acids"sv || key == "amino_acid"sv || key == "amino_acids"sv)
            acmacs::map_draw::select::filter::amino_acid_at_pos(aChartSelectInterface, ind, acmacs::seqdb::extract_aa_at_pos1_eq(name));
        else
            report_error();
    };

    value.visit([&aChartSelectInterface, &indexes, report_error, sequence_one, key]<typename Val>(const Val& val) {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::string>) {
            sequence_one(indexes, val.template to<std::string_view>());
        }
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::boolean>) {
            if (key == "sequenced"sv) {
                if (val.template to<bool>())
                    acmacs::map_draw::select::filter::sequenced(aChartSelectInterface, indexes);
                else
                    acmacs::map_draw::select::filter::not_sequenced(aChartSelectInterface, indexes);
            }
            else
                report_error();
        }
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
            if (key == "clade"sv || key == "clades"sv)
                check_disjunction<std::string_view>(indexes, val, sequence_one);
            else if (key == "amino-acid"sv || key == "amino-acids"sv || key == "amino_acid"sv || key == "amino_acids"sv)
                check_conjunction<std::string_view>(indexes, val, sequence_one);
            else
                report_error();
        }
        // else if constexpr (std::is_same_v<Val, rjson::v3::detail::object>) {
        //     report_error();
        // }
        else
            report_error();
    });

} // check_sequence(antigens)

// ----------------------------------------------------------------------

static inline void check_sequence(const ChartSelectInterface& aChartSelectInterface, const acmacs::chart::Sera& sera, acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& value)
{
    aChartSelectInterface.chart().set_homologous(acmacs::chart::find_homologous::relaxed);

    auto antigens = aChartSelectInterface.chart().antigens();
    auto antigen_indexes = antigens->all_indexes();
    check_sequence(aChartSelectInterface, *antigens, antigen_indexes, key, value);

    auto homologous_filtered_out = [&sera, &antigen_indexes](auto serum_index) -> bool {
        for (auto antigen_index : sera.at(serum_index)->homologous_antigens()) {
            if (antigen_indexes.contains(antigen_index))
                return false;   // homologous antigen selected by ..., do not remove this serum from indexes
        }
        return true;
    };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), homologous_filtered_out), indexes.end());

} // check_sequence(sera)

// ----------------------------------------------------------------------

template <typename AgSr> static void check_layer(const acmacs::chart::Chart& chart, acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& value)
{
    using namespace std::string_view_literals;

    const auto report_error = [key, &value]() { throw acmacs::mapi::unrecognized{fmt::format("unrecognized \"{}\" clause: {}", key, value)}; };

    const auto layer_one = [&chart](acmacs::chart::PointIndexList& ind, int layer) {
        if constexpr (std::is_same_v<AgSr, acmacs::chart::Antigens>)
            acmacs::map_draw::select::filter::layer(chart, ind, layer, acmacs::map_draw::select::filter::antigens);
        else
            acmacs::map_draw::select::filter::layer(chart, ind, layer, acmacs::map_draw::select::filter::sera);
    };

    const auto table_one = [&chart](acmacs::chart::PointIndexList& ind, std::string_view table) {
        const auto& hidb = hidb::get(chart.info()->virus_type());
        if constexpr (std::is_same_v<AgSr, acmacs::chart::Antigens>)
            acmacs::map_draw::select::filter::table_ag_sr(hidb.antigens()->find(*chart.antigens()), ind, table, hidb.tables());
        else
            acmacs::map_draw::select::filter::table_ag_sr(hidb.sera()->find(*chart.sera()), ind, table, hidb.tables());
    };

    value.visit([&indexes, key, layer_one, table_one, report_error]<typename Val>(const Val& val) {
        if constexpr (std::is_same_v<Val, rjson::v3::detail::number>) {
            if (key == "layer"sv || key == "layers"sv)
                layer_one(indexes, val.template to<int>());
            else
                report_error();
        }
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::string>) {
            if (key == "table"sv || key == "tables"sv)
                table_one(indexes, val.template to<std::string_view>());
            else
                report_error();
        }
        else if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
            if (key == "layer"sv || key == "layers"sv)
                check_disjunction<int>(indexes, val, layer_one);
            else if (key == "table"sv || key == "tables"sv)
                check_disjunction<std::string_view>(indexes, val, table_one);
            else
                report_error();
        }
        else
            report_error();
    });

} // check_layer<>

// ----------------------------------------------------------------------

template <typename AgSr> static acmacs::chart::PointIndexList select(const ChartSelectInterface& aChartSelectInterface, const AgSr& ag_sr, const rjson::v3::value& select_clause);

template <typename AgSr> static void check_titrated_against(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& value)
{
    using namespace std::string_view_literals;

    if constexpr (std::is_same_v<AgSr, acmacs::chart::Antigens>) {
        const auto serum_indexes = select(aChartSelectInterface, *aChartSelectInterface.chart().sera(), value);
        if (key[0] == 't')
            acmacs::map_draw::select::filter::antigens_titrated_against(aChartSelectInterface, indexes, serum_indexes);
        else
            acmacs::map_draw::select::filter::antigens_not_titrated_against(aChartSelectInterface, indexes, serum_indexes);
    }
    else {
        const auto antigen_indexes = select(aChartSelectInterface, *aChartSelectInterface.chart().antigens(), value);
        if (key[0] == 't')
            acmacs::map_draw::select::filter::sera_titrated_against(aChartSelectInterface, antigen_indexes, indexes);
        else
            acmacs::map_draw::select::filter::sera_not_titrated_against(aChartSelectInterface, antigen_indexes, indexes);
    }
}

// ----------------------------------------------------------------------

static inline void check_color(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& value)
{
    using namespace std::string_view_literals;

    if (key == "fill"sv)
        acmacs::map_draw::select::filter::fill_in(aChartSelectInterface, indexes, 0, Color(value.to<std::string_view>()));
    else if (key == "outline"sv)
        acmacs::map_draw::select::filter::outline_in(aChartSelectInterface, indexes, 0, Color(value.to<std::string_view>()));
    else if (key == "outline_width"sv || key == "outline-width"sv)
        acmacs::map_draw::select::filter::outline_width_in(aChartSelectInterface, indexes, 0, value.to<Pixels>());
    else
        throw acmacs::mapi::unrecognized{fmt::format("unrecognized \"{}\" clause: {}", key, value)};

} // check_name

// ----------------------------------------------------------------------

template <typename AgSr> static acmacs::chart::PointIndexList select(const ChartSelectInterface& aChartSelectInterface, const AgSr& ag_sr, const rjson::v3::value& select_clause)
{
    using namespace std::string_view_literals;

    if (!select_clause.is_null()) {
        auto indexes = ag_sr.all_indexes();
        bool report{false};
        size_t report_threshold{20};

        try {
            select_clause.visit([&indexes, &ag_sr, &report, &report_threshold, &aChartSelectInterface]<typename Value>(const Value& select_clause_v) {
                if constexpr (std::is_same_v<Value, rjson::v3::detail::string>) {
                    if (check_reference(ag_sr, indexes, select_clause_v.template to<std::string_view>(), rjson::v3::const_true))
                        ; // processed
                    else if (check_passage(ag_sr, indexes, select_clause_v.template to<std::string_view>(), rjson::v3::const_true, throw_if_unprocessed::no))
                        ; // processed
                    else
                        throw acmacs::mapi::unrecognized{};
                }
                else if constexpr (std::is_same_v<Value, rjson::v3::detail::object>) {
                    for (const auto& [key, value] : select_clause_v) {
                        if (check_reference(ag_sr, indexes, key, value))
                            ; // processed
                        else if (key == "date"sv || key == "dates"sv || key == "date_range"sv)
                            check_date(aChartSelectInterface.chart(), ag_sr, indexes, key, value);
                        else if (key == "index"sv || key == "indexes"sv || key == "indices"sv)
                            check_index(ag_sr, indexes, key, value);
                        else if (key == "name"sv || key == "names"sv)
                            check_name(ag_sr, indexes, key, value);
                        else if (key == "passage"sv)
                            check_passage(ag_sr, indexes, key, value, throw_if_unprocessed::yes);
                        else if (key == "sequenced"sv || key == "clade"sv || key == "clades"sv || key == "amino_acid"sv || key == "amino_acids"sv || key == "amino-acid"sv || key == "amino-acids"sv)
                            check_sequence(aChartSelectInterface, ag_sr, indexes, key, value);
                        else if (key == "lab"sv || key == "labs"sv)
                            check_lab(aChartSelectInterface.chart(), indexes, key, value);
                        else if (key == "lineage"sv)
                            check_lineage(aChartSelectInterface.chart(), indexes, key, value);
                        else if (key == "fill"sv || key == "outline"sv || key == "outline-width"sv || key == "outline_width"sv)
                            check_color(aChartSelectInterface, indexes, key, value);
                        else if (key == "layer"sv || key == "layers"sv || key == "table"sv || key == "tables"sv)
                            check_layer<AgSr>(aChartSelectInterface.chart(), indexes, key, value);
                        else if (key == "titrated-against-sera"sv || key == "titrated-against-antigens"sv || key == "titrated-against"sv || key == "not-titrated-against-sera"sv || key == "not-titrated-against-antigens"sv || key == "not-titrated-against"sv)
                            check_titrated_against<AgSr>(aChartSelectInterface, indexes, key, value);
                        else if (key == "report"sv)
                            report = value.template to<bool>();
                        else if (key == "report-threshold"sv || key == "report_threshold"sv)
                            report_threshold = value.template to<size_t>();
                        else if (key == "country"sv || key == "countries"sv || key == "continent"sv || key == "continents"sv || key == "location"sv || key == "locations"sv)
                            check_location(ag_sr, indexes, key, value);
                        else if (!key.empty() && key[0] != '?')
                            AD_WARNING("unrecognized \"select\" key: \"{}\"", key);
                    }
                }
                else
                    throw acmacs::mapi::unrecognized{};
            });
        }
        catch (std::exception& err) {
            throw acmacs::mapi::unrecognized{AD_FORMAT("unrecognized \"select\" clause: {}: {}", select_clause, err)};
        }
        if (report) {
            if constexpr (std::is_same_v<AgSr, acmacs::chart::Antigens>) {
                AD_INFO("{} antigens selected with {}", indexes.size(), select_clause);
                report_antigens(indexes, aChartSelectInterface, report_threshold);
            }
            else {
                AD_INFO("{} sera selected with {}", indexes.size(), select_clause);
                report_sera(indexes, aChartSelectInterface, report_threshold);
            }
        }
        return indexes;
    }
    else
        throw acmacs::mapi::unrecognized{"no \"select\" clause"};

} // acmacs::mapi::v1::Settings::select

// ----------------------------------------------------------------------

acmacs::chart::PointIndexList acmacs::mapi::v1::Settings::select_antigens() const
{
    using namespace std::string_view_literals;
    return ::select(chart_draw(), *chart_draw().chart().antigens(), getenv("select"sv));

} // acmacs::mapi::v1::Settings::select_antigens

// ----------------------------------------------------------------------

acmacs::chart::PointIndexList acmacs::mapi::v1::Settings::select_sera() const
{
    using namespace std::string_view_literals;
    return ::select(chart_draw(), *chart_draw().chart().sera(), getenv("select"sv));

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
