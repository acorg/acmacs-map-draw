#include "acmacs-base/date.hh"
#include "acmacs-base/string-compare.hh"
#include "acmacs-base/rjson-v3-helper.hh"
#include "acmacs-base/color-amino-acid.hh"
#include "acmacs-whocc-data/labs.hh"
#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/report-antigens.hh"
#include "acmacs-map-draw/select-filter.hh"

enum class throw_if_unprocessed { no, yes };

// ----------------------------------------------------------------------

template <typename AgSr> bool acmacs::mapi::v1::Settings::color_according_to_passage(const AgSr& ag_sr, const acmacs::chart::PointIndexList& indexes, const point_style_t& style)
{
    using namespace std::string_view_literals;

    if ((style.passage_fill.has_value() && style.passage_fill->egg.has_value()) || (style.passage_outline.has_value() && style.passage_outline->egg.has_value())) {
        auto egg_indexes = indexes;
        ag_sr.filter_egg(egg_indexes, acmacs::chart::reassortant_as_egg::no);
        auto reassortant_indexes = indexes;
        ag_sr.filter_reassortant(reassortant_indexes);
        auto cell_indexes = indexes;
        ag_sr.filter_cell(cell_indexes);

        PointStyleModified ps_egg, ps_reassortant, ps_cell;
        if (style.passage_fill.has_value()) {
            ps_egg.fill(*style.passage_fill->egg);
            ps_reassortant.fill(*style.passage_fill->reassortant);
            ps_cell.fill(*style.passage_fill->cell);
        }
        if (style.passage_outline.has_value()) {
            ps_egg.outline(*style.passage_outline->egg);
            ps_reassortant.outline(*style.passage_outline->reassortant);
            ps_cell.outline(*style.passage_outline->cell);
        }

        // order b y number of occurrences, lest frequent passage (last in by_passage) is on top
        using by_passage_t = std::tuple<const acmacs::chart::PointIndexList*, std::string_view, const PointStyleModified*>;
        std::array by_passage{
            by_passage_t{&egg_indexes, "egg", &ps_egg},
            by_passage_t{&reassortant_indexes, "reassortant", &ps_reassortant},
            by_passage_t{&cell_indexes, "cell", &ps_cell},
        };
        std::sort(std::begin(by_passage), std::end(by_passage), [](const auto& e1, const auto& e2) { return std::get<const acmacs::chart::PointIndexList*>(e1)->size() > std::get<const acmacs::chart::PointIndexList*>(e2)->size(); });

        if constexpr (std::is_same_v<AgSr, acmacs::chart::Antigens>) {
            chart_draw().modify(egg_indexes, ps_egg);
            chart_draw().modify(reassortant_indexes, ps_reassortant);
            chart_draw().modify(cell_indexes, ps_cell);
            chart_draw().modify_drawing_order(*std::get<const acmacs::chart::PointIndexList*>(by_passage[1]), PointDrawingOrder::Raise);
            chart_draw().modify_drawing_order(*std::get<const acmacs::chart::PointIndexList*>(by_passage[2]), PointDrawingOrder::Raise);
        }
        else {
            chart_draw().modify_sera(egg_indexes, ps_egg);
            chart_draw().modify_sera(reassortant_indexes, ps_reassortant);
            chart_draw().modify_sera(cell_indexes, ps_cell);
            chart_draw().modify_sera_drawing_order(*std::get<const acmacs::chart::PointIndexList*>(by_passage[1]), PointDrawingOrder::Raise);
            chart_draw().modify_sera_drawing_order(*std::get<const acmacs::chart::PointIndexList*>(by_passage[2]), PointDrawingOrder::Raise);
        }

        // order legend by number of occurrences
        if (const auto& legend = getenv("legend"sv); !legend.is_null()) {
            for (const auto& en : by_passage) {
                if (const auto& indexes_of_passage = *std::get<const acmacs::chart::PointIndexList*>(en); indexes_of_passage.size() > 0) {
                    const auto label{fmt::format(rjson::v3::get_or(legend["label"sv], "{passage} ({count})"sv),
                                                 fmt::arg("passage", std::get<std::string_view>(en)),
                                                 fmt::arg("count", indexes_of_passage.size()))};
                    add_legend(indexes_of_passage, *std::get<const PointStyleModified*>(en), label, legend);
                }
            }
        }

        return true;
    }
    else
        return false;

} // acmacs::mapi::v1::Settings::color_according_to_passage

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_antigens()
{
    using namespace std::string_view_literals;

    const auto indexes = select_antigens(getenv("select"sv));
    const auto style = style_from_toplevel_environment();
    chart_draw().modify(indexes, style.style, drawing_order_from_toplevel_environment());
    if (!color_according_to_passage(*chart_draw().chart().antigens(), indexes, style) && !color_according_to_aa_at_pos(indexes, style)) {
        if (const auto& legend = getenv("legend"sv); !legend.is_null())
            add_legend(indexes, style.style, legend);
    }
    if (const auto& label = getenv("label"sv); !label.is_null())
        add_labels(indexes, 0, label);

    return true;

} // acmacs::mapi::v1::Settings::apply_antigens

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_sera()
{
    using namespace std::string_view_literals;

    const auto indexes = select_sera(getenv("select"sv));
    const auto style = style_from_toplevel_environment();
    chart_draw().modify_sera(indexes, style.style, drawing_order_from_toplevel_environment());
    if (!color_according_to_passage(*chart_draw().chart().sera(), indexes, style)) {
        if (const auto& legend = getenv("legend"sv); !legend.is_null())
            add_legend(indexes, style.style, legend);
    }
    const auto number_of_antigens = chart_draw().chart().number_of_antigens();
    if (const auto& label = getenv("label"sv); !label.is_null())
        add_labels(indexes, number_of_antigens, label);

    return true;

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

template <typename AgSr> void acmacs::mapi::v1::Settings::check_titrated_against(acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& value) const
{
    using namespace std::string_view_literals;

    if constexpr (std::is_same_v<AgSr, acmacs::chart::Antigens>) {
        const auto serum_indexes = select(*chart_draw().chart().sera(), value, if_null::raise);
        if (key[0] == 't')
            acmacs::map_draw::select::filter::antigens_titrated_against(chart_draw(), indexes, serum_indexes);
        else
            acmacs::map_draw::select::filter::antigens_not_titrated_against(chart_draw(), indexes, serum_indexes);
    }
    else {
        const auto antigen_indexes = select(*chart_draw().chart().antigens(), value, if_null::raise);
        if (key[0] == 't')
            acmacs::map_draw::select::filter::sera_titrated_against(chart_draw(), antigen_indexes, indexes);
        else
            acmacs::map_draw::select::filter::sera_not_titrated_against(chart_draw(), antigen_indexes, indexes);
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

template <typename AgSr> static inline void check_inside(const acmacs::mapi::v1::Settings& settings, acmacs::chart::PointIndexList& indexes, std::string_view key, const rjson::v3::value& value)
{
    using namespace std::string_view_literals;

    size_t index_base{0};
    if constexpr (std::is_same_v<AgSr, acmacs::chart::Sera>)
        index_base = settings.chart_draw().chart().number_of_antigens();

    if (const auto& points = value["points"sv]; !points.is_null())
        settings.filter_inside_path(indexes, points, index_base);
    else
        throw acmacs::mapi::unrecognized{fmt::format("unrecognized \"{}\" clause: {}", key, value)};

} // check_location

// ----------------------------------------------------------------------

template <typename AgSr> acmacs::chart::PointIndexList acmacs::mapi::v1::Settings::select(const AgSr& ag_sr, const rjson::v3::value& select_clause, if_null ifnull) const
{
    using namespace std::string_view_literals;

    auto indexes = ag_sr.all_indexes();
    bool report{false};
    size_t report_threshold{20};

    try {
        select_clause.visit([&indexes, &ag_sr, &report, &report_threshold, ifnull, this]<typename Value>(const Value& select_clause_v) {
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
                        check_date(chart_draw().chart(), ag_sr, indexes, key, value);
                    else if (key == "index"sv || key == "indexes"sv || key == "indices"sv)
                        check_index(ag_sr, indexes, key, value);
                    else if (key == "name"sv || key == "names"sv)
                        check_name(ag_sr, indexes, key, value);
                    else if (key == "passage"sv)
                        check_passage(ag_sr, indexes, key, value, throw_if_unprocessed::yes);
                    else if (key == "sequenced"sv || key == "clade"sv || key == "clades"sv || key == "amino_acid"sv || key == "amino_acids"sv || key == "amino-acid"sv || key == "amino-acids"sv)
                        check_sequence(chart_draw(), ag_sr, indexes, key, value);
                    else if (key == "lab"sv || key == "labs"sv)
                        check_lab(chart_draw().chart(), indexes, key, value);
                    else if (key == "lineage"sv)
                        check_lineage(chart_draw().chart(), indexes, key, value);
                    else if (key == "fill"sv || key == "outline"sv || key == "outline-width"sv || key == "outline_width"sv)
                        check_color(chart_draw(), indexes, key, value);
                    else if (key == "exclude-distinct"sv || key == "exclude_distinct"sv)
                        acmacs::map_draw::select::filter::out_distinct_in(ag_sr, indexes);
                    else if (key == "serum-id"sv || key == "serum_id"sv) {
                        if constexpr (std::is_same_v<AgSr, acmacs::chart::Sera>)
                            acmacs::map_draw::select::filter::serum_id_in(ag_sr, indexes, value.template to<std::string_view>());
                        else
                            AD_WARNING("\"select\" key: \"{}\" not applicable for antigens", key);
                    }
                    else if (key == "layer"sv || key == "layers"sv || key == "table"sv || key == "tables"sv)
                        check_layer<AgSr>(chart_draw().chart(), indexes, key, value);
                    else if (key == "titrated-against-sera"sv || key == "titrated-against-antigens"sv || key == "titrated-against"sv || key == "not-titrated-against-sera"sv ||
                             key == "not-titrated-against-antigens"sv || key == "not-titrated-against"sv)
                        check_titrated_against<AgSr>(indexes, key, value);
                    else if (key == "report"sv)
                        report = value.template to<bool>();
                    else if (key == "report-threshold"sv || key == "report_threshold"sv)
                        report_threshold = value.template to<size_t>();
                    else if (key == "country"sv || key == "countries"sv || key == "continent"sv || key == "continents"sv || key == "location"sv || key == "locations"sv)
                        check_location(ag_sr, indexes, key, value);
                    else if (key == "inside"sv)
                        check_inside<AgSr>(*this, indexes, key, value);
                    else if (!key.empty() && key[0] != '?')
                        AD_WARNING("unrecognized \"select\" key: \"{}\"", key);
                }
            }
            else if constexpr (std::is_same_v<Value, rjson::v3::detail::null>) {
                switch (ifnull) {
                    case if_null::empty:
                        indexes.clear();
                        break;
                    case if_null::warn_empty:
                        AD_WARNING("no antigens sera selected because no select clause present");
                        indexes.clear();
                        break;
                    case if_null::raise:
                        throw acmacs::mapi::unrecognized{};
                    case if_null::all:
                        break;
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
            fmt::print(stderr, "{}", report_antigens(indexes, chart_draw(), report_threshold));
        }
        else {
            AD_INFO("{} sera selected with {}", indexes.size(), select_clause);
            fmt::print(stderr, "{}", report_sera(indexes, chart_draw(), report_threshold));
        }
    }
    return indexes;

} // acmacs::mapi::v1::Settings::select

// ----------------------------------------------------------------------

acmacs::chart::PointIndexList acmacs::mapi::v1::Settings::select_antigens(const rjson::v3::value& select_clause, if_null ifnull) const
{
    return select(*chart_draw().chart().antigens(), select_clause, ifnull);

} // acmacs::mapi::v1::Settings::select_antigens

// ----------------------------------------------------------------------

acmacs::chart::PointIndexList acmacs::mapi::v1::Settings::select_sera(const rjson::v3::value& select_clause, if_null ifnull) const
{
    return select(*chart_draw().chart().sera(), select_clause, ifnull);

} // acmacs::mapi::v1::Settings::select_sera

// ----------------------------------------------------------------------

acmacs::mapi::v1::Settings::point_style_t acmacs::mapi::v1::Settings::style_from_toplevel_environment() const
{
    using namespace std::string_view_literals;
    point_style_t result;
    for (const auto& [key, val] : getenv_toplevel()) {
        // AD_DEBUG("apply_antigens {}: {}", key, val);
        if (key == "fill"sv) {
            std::visit(
                [&result]<typename Modifier>(const Modifier& modifier) {
                    if constexpr (std::is_same_v<Modifier, acmacs::color::Modifier>)
                        result.style.fill(modifier);
                    else
                        result.passage_fill = modifier;
                },
                color(val));
        }
        else if (key == "outline"sv) {
            std::visit(
                [&result]<typename Modifier>(const Modifier& modifier) {
                    if constexpr (std::is_same_v<Modifier, acmacs::color::Modifier>)
                        result.style.outline(modifier);
                    else
                        result.passage_outline = modifier;
                },
                color(val));
        }
        else if (key == "show"sv)
            result.style.shown(substitute_to_bool(val));
        else if (key == "hide"sv)
            result.style.shown(!substitute_to_bool(val));
        else if (key == "shape"sv)
            result.style.shape(PointShape{substitute_to_string(val)});
        else if (key == "size"sv)
            result.style.size(Pixels{substitute_to_double(val)});
        else if (key == "outline_width"sv)
            result.style.outline_width(Pixels{substitute_to_double(val)});
        else if (key == "aspect"sv)
            result.style.aspect(Aspect{substitute_to_double(val)});
        else if (key == "rotation"sv)
            result.style.rotation(Rotation{substitute_to_double(val)});
        else if (key == "fill_saturation"sv || key == "fill_brightness"sv || key == "outline_saturation"sv || key == "outline_brightness"sv) {
            AD_WARNING("\"{}\" is not supported, use color modificators, e.g. \":s-0.5\"", key);
        }
    }
    return result;

} // acmacs::mapi::v1::Settings::style_from_toplevel_environment

// ----------------------------------------------------------------------

void acmacs::mapi::v1::Settings::passage_color_t::init_passage_colors()
{
    egg = acmacs::color::Modifier{Color{0xFF4040}};
    reassortant = acmacs::color::Modifier{Color{0xFFB040}};
    cell = acmacs::color::Modifier{Color{0x4040FF}};

} // acmacs::mapi::v1::Settings::passage_color_t::init_passage_colors

// ----------------------------------------------------------------------

acmacs::mapi::v1::Settings::modifier_or_passage_t acmacs::mapi::v1::Settings::color(const rjson::v3::value& value) const
{
    using namespace std::string_view_literals;
    const auto make_color = [](std::string_view source) -> modifier_or_passage_t {
        if (source == "passage"sv) {
            passage_color_t colors;
            colors.init_passage_colors();
            return colors;
        }
        else
            return acmacs::color::Modifier{source};
    };

    const auto make_color_passage_helper = [](const auto& substituted) -> std::optional<color::Modifier> {
        return std::visit(
            []<typename Res>(const Res& res) -> std::optional<color::Modifier> {
                if constexpr (std::is_same_v<Res, const rjson::v3::value*>) {
                    if (!res->is_null())
                        return color::Modifier{res->template to<std::string_view>()};
                }
                else
                    return color::Modifier{res};
                return std::nullopt;
            },
            substituted);
    };

    const auto make_color_passage = [this, make_color_passage_helper](passage_color_t& result, const rjson::v3::value& egg, const rjson::v3::value& reassortant, const rjson::v3::value& cell) {
        if (!egg.is_null() || !reassortant.is_null() || !cell.is_null())
            result.init_passage_colors();
        if (const auto egg_val = make_color_passage_helper(substitute(egg)); egg_val.has_value())
            result.egg = result.reassortant = *egg_val;
        if (const auto reassortant_val = make_color_passage_helper(substitute(reassortant)); reassortant_val.has_value())
            result.reassortant = *reassortant_val;
        if (const auto cell_val = make_color_passage_helper(substitute(cell)); cell_val.has_value())
            result.cell = *cell_val;
    };

    const auto make_color_aa_at = [](passage_color_t& result, const rjson::v3::value& aa_at, const rjson::v3::value& colors) {
        result.pos = rjson::v3::read_number<acmacs::seqdb::pos1_t>(aa_at);
        colors.visit([&result]<typename Val>(const Val& color_values) {
            if constexpr (std::is_same_v<Val, rjson::v3::detail::array>) {
                for (const auto& col : color_values)
                    result.color_order.emplace_back(col.template to<std::string_view>());
            }
            else if constexpr (!std::is_same_v<Val, rjson::v3::detail::null>)
                AD_WARNING("invalid \"colors\": {} (array of colors expected)", color_values);
        });
    };

    try {
        return std::visit(
            [make_color, make_color_passage, make_color_aa_at]<typename Value>(const Value& substituted_val) -> modifier_or_passage_t {
                if constexpr (std::is_same_v<Value, const rjson::v3::value*>) {
                    return substituted_val->visit([make_color, make_color_passage, make_color_aa_at]<typename Val>(const Val& val) -> modifier_or_passage_t {
                        if constexpr (std::is_same_v<Val, rjson::v3::detail::string>)
                            return make_color(val.template to<std::string_view>());
                        else if constexpr (std::is_same_v<Val, rjson::v3::detail::object>) {
                            passage_color_t passage_color;
                            make_color_passage(passage_color, val["egg"sv], val["reassortant"sv], val["cell"sv]);
                            make_color_aa_at(passage_color, val["aa-at"sv], val["colors"sv]);
                            return passage_color;
                        }
                        else
                            throw std::exception{};
                    });
                }
                else
                    return make_color(substituted_val);
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

bool acmacs::mapi::v1::Settings::color_according_to_aa_at_pos(const acmacs::chart::PointIndexList& indexes, const point_style_t& style)
{
    using namespace std::string_view_literals;

    const auto make = [&indexes, this](const std::optional<passage_color_t>& data, bool fill) -> bool {
        if (data.has_value() && data->pos.has_value()) {
            const auto& entries = chart_draw().match_seqdb();
            const auto& seqdb = acmacs::seqdb::get();
            acmacs::small_map_with_unique_keys_t<char, std::pair<acmacs::chart::PointIndexList, PointStyleModified>> per_aa;
            for (const auto index : indexes) {
                if (const auto& entry{entries[index]}; entry)
                    per_aa.emplace_not_replace(entry.aa_at_pos(seqdb, *data->pos)).second.first.insert(index);
            }
            per_aa.sort([](const auto& e1, const auto& e2) { return e1.second.first.size() > e2.second.first.size(); }); // most occurred aa first

            // set colors
            if (!data->color_order.empty()) {
                size_t colors_used{0};
                for (auto& [aa, en] : per_aa) {
                    acmacs::color::Modifier color;
                    if (aa == 'X')
                        color.add(acmacs::color::Modifier{GREY});
                    else if (colors_used < data->color_order.size())
                        color.add(data->color_order[colors_used++]);
                    else
                        throw acmacs::mapi::unrecognized{fmt::format("color_according_to_aa_at_pos: too few colors in the color order for pos {}", *data->pos)};
                    if (fill)
                        en.second.fill(color);
                    else
                        en.second.outline(color);
                }
            }
            else { // standard aa colors
                for (auto& [aa, en] : per_aa) {
                    const acmacs::color::Modifier color{acmacs::amino_acid_color(aa)};
                    if (fill)
                        en.second.fill(color);
                    else
                        en.second.outline(color);
                }
            }

            // mark antigens
            for (const auto& [aa, en] : per_aa)
                chart_draw().modify(en.first, en.second, PointDrawingOrder::Raise);

            // legend lines
            if (const auto& legend = getenv("legend"sv); !legend.is_null()) {
                for (const auto& [aa, en] : per_aa) {
                    const auto label{fmt::format(rjson::v3::get_or(legend["label"sv], "{pos}{aa} ({count})"sv),
                                                 fmt::arg("aa", aa), fmt::arg("pos", *data->pos), fmt::arg("count", en.first.size()))};
                        add_legend(en.first, en.second, label, legend);
                }
            }

            return true;
        }
        else
            return false;
    };

    return make(style.passage_fill, true) || make(style.passage_outline, false);

} // acmacs::mapi::v1::Settings::color_according_to_aa_at_pos

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
