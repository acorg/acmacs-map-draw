#include <chrono>

#include "acmacs-base/rjson-v2.hh"
#include "acmacs-base/string.hh"
#include "acmacs-map-draw/vaccines.hh"
#include "acmacs-map-draw/select.hh"
#include "acmacs-map-draw/report-antigens.hh"

// ----------------------------------------------------------------------

SelectAntigensSera::~SelectAntigensSera()
{
} // SelectAntigensSera::~SelectAntigensSera

// ----------------------------------------------------------------------

acmacs::chart::Indexes SelectAntigensSera::select(ChartSelectInterface& aChartSelectInterface, const rjson::value& aSelector)
{
    try {
        return std::visit(
            [this, &aChartSelectInterface](const auto& val) -> acmacs::chart::Indexes {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, std::string>)
                    return this->command(aChartSelectInterface, rjson::object{{val, true}});
                else if constexpr (std::is_same_v<T, rjson::object>)
                    return this->command(aChartSelectInterface, val);
                else
                    throw std::runtime_error{"(type is neither string not object)"};
            },
            aSelector.val_());
    }
    catch (std::exception& err) {
        throw std::runtime_error{fmt::format("Unsupported selector value: {}: {}", aSelector, err)};
    }

} // SelectAntigensSera::select

// ----------------------------------------------------------------------

template <typename AgSr>
void filter(const AgSr& ag_sr, acmacs::chart::Indexes& indexes, SelectAntigensSera& /*aSelectAntigensSera*/, const ChartSelectInterface& /*aChartSelectInterface*/, const rjson::value& aSelector,
            std::string_view key, const rjson::value& val)
{
    if (key == "egg") {
        ag_sr.filter_egg(indexes);
    }
    else if (key == "cell") {
        ag_sr.filter_cell(indexes);
    }
    else if (key == "reassortant") {
        ag_sr.filter_reassortant(indexes);
    }
    else if (key == "passage") {
        const std::string_view passage{val.to<std::string_view>()};
        if (passage == "egg")
            ag_sr.filter_egg(indexes);
        else if (passage == "cell")
            ag_sr.filter_cell(indexes);
        else if (passage == "reassortant")
            ag_sr.filter_reassortant(indexes);
        else
            throw std::exception{};
    }
    else if (key == "country") {
        ag_sr.filter_country(indexes, string::upper(val.to<std::string_view>()));
    }
    else if (key == "continent") {
        ag_sr.filter_continent(indexes, string::upper(val.to<std::string_view>()));
    }
    else if (key == "exclude_distinct") {
        // processed together with the main selector, e.g. "name"
    }
    else {
        fmt::print(stderr, "WARNING: unrecognized key \"{}\" in selector: {}\n", key, aSelector);
    }

} // filter

// ----------------------------------------------------------------------

acmacs::chart::Indexes SelectAntigens::command(ChartSelectInterface& aChartSelectInterface, const rjson::value& aSelector)
{
    auto antigens = aChartSelectInterface.chart().antigens();
    auto indexes = antigens->all_indexes();
    rjson::for_each(aSelector, [this, &aChartSelectInterface, &antigens, &indexes, &aSelector](std::string_view key, const rjson::value& val) {
        if (!key.empty() && (key.front() == '?' || key.back() == '?')) {
            // comment
        }
        else if (key == "all") {
            // do nothing
        }
        else if (key == "reference") {
            antigens->filter_reference(indexes);
        }
        else if (key == "test") {
            antigens->filter_test(indexes);
        }
        else if (key == "date_range") {
            antigens->filter_date_range(indexes, val[0].to<std::string_view>(), val[1].to<std::string_view>());
        }
        else if (key == "older_than_days") {
            using namespace std::chrono_literals;
            const int days{val.to<int>()};
            const auto then = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() - 24h * days);
            char buffer[20];
            std::strftime(buffer, sizeof buffer, "%Y-%m-%d", std::localtime(&then));
            antigens->filter_date_range(indexes, "", buffer);
        }
        else if (key == "younger_than_days") {
            using namespace std::chrono_literals;
            const int days{val.to<int>()};
            const auto then = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() - 24h * days);
            char buffer[20];
            std::strftime(buffer, sizeof buffer, "%Y-%m-%d", std::localtime(&then));
            antigens->filter_date_range(indexes, buffer, "");
        }
        else if (key == "index") {
            indexes.erase_except(val.to<size_t>());
        }
        else if (key == "indexes") {
            acmacs::chart::Indexes to_keep(val.size());
            rjson::transform(val, to_keep.begin(), [](const rjson::value& v) -> size_t { return v.to<size_t>(); });
            indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), [&to_keep](auto index) -> bool { return std::find(to_keep.begin(), to_keep.end(), index) == to_keep.end(); }),
                                indexes.end());
        }
        else if (key == "sequenced") {
            filter_sequenced(aChartSelectInterface, indexes);
        }
        else if (key == "not_sequenced") {
            filter_not_sequenced(aChartSelectInterface, indexes);
        }
        else if (key == "clade") {
            filter_clade(aChartSelectInterface, indexes, string::upper(val.to<std::string_view>()));
        }
        else if (key == "amino_acid") {
            if (val.is_object())
                filter_amino_acid_at_pos(aChartSelectInterface, indexes, val["aa"].to<std::string_view>()[0], acmacs::seqdb::pos1_t{val["pos"].to<size_t>()}, true);
            else if (val.is_array()) {
                std::vector<std::string> vals;
                rjson::transform(val, std::back_inserter(vals), [](const rjson::value& v) -> std::string { return v.to<std::string>(); });
                filter_amino_acid_at_pos(aChartSelectInterface, indexes, acmacs::seqdb::extract_aa_at_pos1_eq_list(acmacs::string::join(acmacs::string::join_comma, vals)));
            }
            else
                throw std::runtime_error{"invalid \"amino_acid\" value, object or array expected"};
        }
        else if (key == "outlier") {
            filter_outlier(aChartSelectInterface, indexes, val.to<double>());
        }
        else if (key == "name") {
            if (val.is_string()) {
                filter_name(aChartSelectInterface, indexes, val.to<std::string_view>());
            }
            else if (val.is_array()) {
                decltype(indexes) to_include;
                rjson::for_each(val, [&to_include, &antigens](const rjson::value& entry) {
                    const auto by_name = antigens->find_by_name(entry.to<std::string_view>());
                    to_include.extend(by_name);
                });
                acmacs::chart::Indexes result(indexes->size());
                const auto end = std::set_intersection(indexes.begin(), indexes.end(), to_include.begin(), to_include.end(), result.begin());
                indexes.get().erase(std::copy(result.begin(), end, indexes.begin()), indexes.end());
            }
            else
                throw std::exception{};
            if (rjson::get_or(aSelector, "exclude_distinct", false))
                filter_out_distinct(aChartSelectInterface, indexes);
        }
        else if (key == "full_name") {
            if (val.is_string()) {
                filter_full_name(aChartSelectInterface, indexes, string::upper(val.to<std::string_view>()));
                if (const auto& no = aSelector.get("no"); !no.is_null())
                    indexes.erase_except((*indexes)[no.to<size_t>()]);
            }
            else if (val.is_array()) {
                decltype(indexes) to_include;
                rjson::for_each(val, [&to_include, &antigens](const rjson::value& entry) {
                    if (const auto by_full_name = antigens->find_by_full_name(entry.to<std::string_view>()); by_full_name.has_value())
                        to_include.insert(*by_full_name);
                });
                acmacs::chart::Indexes result(indexes->size());
                const auto end = std::set_intersection(indexes.begin(), indexes.end(), to_include.begin(), to_include.end(), result.begin());
                indexes.get().erase(std::copy(result.begin(), end, indexes.begin()), indexes.end());
            }
            else
                throw std::exception{};
        }
        else if (key == "location") {
            filter_location(aChartSelectInterface, indexes, string::upper(val.to<std::string_view>()));
        }
        else if (key == "vaccine" || key == "vaccines") {
            try {
                filter_vaccine(
                    aChartSelectInterface, indexes,
                    VaccineMatchData{}.type(rjson::get_or(val, "type", "")).passage(rjson::get_or(val, "passage", "")).no(rjson::get_or(val, "no", 0UL)).name(rjson::get_or(val, "name", "")));
            }
            catch (std::bad_variant_access&) {
                // std::cerr << "WARNING: filter_vaccine: bad_variant_access" << '\n';
                filter_vaccine(aChartSelectInterface, indexes, {});
            }
        }
        else if (key == "in_rectangle") {
            const auto& c1 = val["c1"];
            const auto& c2 = val["c2"];
            filter_rectangle(aChartSelectInterface, indexes, {c1[0].to<double>(), c1[1].to<double>(), c2[0].to<double>(), c2[1].to<double>()},
                             rjson::get_or(val, "transform", true) ? transformed_t::yes : transformed_t::no, RotationRadiansOrDegrees(rjson::get_or(val, "rotate", 0.0)));
        }
        else if (key == "in_circle") {
            const auto& center = val["center"];
            const double radius{val["radius"].to<double>()};
            filter_circle(aChartSelectInterface, indexes, {{center[0].to<double>(), center[1].to<double>()}, radius});
        }
        else if (key == "relative_to_serum_line") {
            filter_relative_to_serum_line(aChartSelectInterface, indexes, rjson::get_or(val, "distance_min", 0.0), rjson::get_or(val, "distance_max", std::numeric_limits<double>::max()),
                                          rjson::get_or(val, "direction", 0));
        }
        else if (key == "lab") {
            if (aChartSelectInterface.chart().info()->lab(acmacs::chart::Info::Compute::Yes) != acmacs::Lab{val.to<std::string_view>()})
                indexes.get().clear();
        }
        else if (key == "subtype") {
            const std::string virus_type{aChartSelectInterface.chart().info()->virus_type(acmacs::chart::Info::Compute::Yes)};
            const std::string val_u = string::upper(val.to<std::string_view>());
            if (val_u != virus_type) {
                bool clear_indexes = true;
                if (virus_type == "B") {
                    const auto lineage = aChartSelectInterface.chart().lineage();
                    clear_indexes = !(((val_u == "BVIC" || val_u == "BV") && lineage == acmacs::virus::lineage::VICTORIA) ||
                                      ((val_u == "BYAM" || val_u == "BY") && lineage == acmacs::virus::lineage::YAMAGATA));
                }
                else {
                    clear_indexes = !((val_u == "H1" && virus_type == "A(H1N1)") || (val_u == "H3" && virus_type == "A(H3N2)"));
                }
                if (clear_indexes)
                    indexes.get().clear();
            }
        }
        else if (key == "found_in_previous") {
            if (!aChartSelectInterface.previous_chart())
                throw std::runtime_error{"\"found_in_previous\" selector used but no previous chart provided"};
            auto previous_antigens = aChartSelectInterface.previous_chart()->antigens();
            antigens->filter_found_in(indexes, *previous_antigens);
        }
        else if (key == "not_found_in_previous") {
            if (!aChartSelectInterface.previous_chart())
                throw std::runtime_error{"\"not_found_in_previous\" selector used but no previous chart provided"};
            auto previous_antigens = aChartSelectInterface.previous_chart()->antigens();
            antigens->filter_not_found_in(indexes, *previous_antigens);
        }
        else if (key == "table") {
            filter_table(aChartSelectInterface, indexes, val.to<std::string_view>());
        }
        else if (key == "layer") {
            filter_layer(aChartSelectInterface, indexes, val.to<int>());
        }
        else if (key == "titrated_against_sera") {
            const auto serum_indexes = SelectSera(acmacs::verbose::yes).command(aChartSelectInterface, val);
            filter_titrated_against(aChartSelectInterface, indexes, serum_indexes);
            // std::cerr << "DEBUG: titrated_against_sera " << serum_indexes << ' ' << indexes << '\n';
        }
        else if (key == "no") {
            // processed together with the main selector, e.g. "full_name"
        }
        else if (key == "outline") {
            filter_outline(aChartSelectInterface, indexes, Color(val.to<std::string_view>()));
        }
        else if (key == "outline_width") {
            filter_outline_width(aChartSelectInterface, indexes, Pixels{val.to<double>()});
        }
        else {
            filter(*antigens, indexes, *this, aChartSelectInterface, aSelector, key, val);
        }
    });
    if (verbose()) {
        fmt::print("INFO: antigens selected: {:4d} {}\n", indexes->size(), aSelector);
        if (!indexes->empty())
            fmt::print(stderr, "{}", report_antigens(indexes, aChartSelectInterface, report_names_threshold()));
    }

    return indexes;

} // SelectAntigens::command

// ----------------------------------------------------------------------

// const acmacs::seqdb::subset& SelectAntigensSera::seqdb_entries(const ChartSelectInterface& aChartSelectInterface)
// {
//     // thread unsafe!
//     static std::map<const ChartSelectInterface*, acmacs::seqdb::subset> cache_seqdb_entries_for_chart;

//     auto found = cache_seqdb_entries_for_chart.find(&aChartSelectInterface);
//     if (found == cache_seqdb_entries_for_chart.end())
//         found = cache_seqdb_entries_for_chart.emplace(&aChartSelectInterface, aChartSelectInterface.match_seqdb()).first;
//     return found->second;

// } // SelectAntigensSera::seqdb_entries

// ----------------------------------------------------------------------

acmacs::chart::Indexes SelectSera::command(ChartSelectInterface& aChartSelectInterface, const rjson::value& aSelector)
{
    const auto& sera = aChartSelectInterface.chart().sera();
    auto indexes = sera->all_indexes();
    rjson::for_each(aSelector, [this, &aChartSelectInterface, &sera, &indexes, &aSelector](std::string_view key, const rjson::value& val) {
        if (!key.empty() && (key.front() == '?' || key.back() == '?')) {
            // comment
        }
        else if (key == "all") {
            // do nothing
        }
        else if (key == "serum_id") {
            sera->filter_serum_id(indexes, string::upper(val.to<std::string_view>()));
        }
        else if (key == "index") {
            indexes.erase_except(val.to<size_t>());
        }
        else if (key == "indexes") {
            acmacs::chart::Indexes to_keep(val.size());
            rjson::transform(val, to_keep.begin(), [](const rjson::value& v) -> size_t { return v.to<size_t>(); });
            indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), [&to_keep](auto index) -> bool { return std::find(to_keep.begin(), to_keep.end(), index) == to_keep.end(); }),
                                indexes.end());
        }
        else if (key == "clade") {
            filter_clade(aChartSelectInterface, indexes, string::upper(val.to<std::string_view>()));
        }
        else if (key == "amino_acid") {
            if (val.is_array()) {
                std::vector<std::string> vals;
                rjson::transform(val, std::back_inserter(vals), [](const rjson::value& v) -> std::string { return v.to<std::string>(); });
                filter_amino_acid_at_pos(aChartSelectInterface, indexes, acmacs::seqdb::extract_aa_at_pos1_eq_list(acmacs::string::join(acmacs::string::join_comma, vals)));
            }
            else
                throw std::runtime_error{"invalid \"amino_acid\" value, array expected"};
        }
        // else if (key == "country") {
        //     sera->filter_country(indexes, string::upper(val.to<std::string_view>()));
        // }
        // else if (key == "continent") {
        //     sera->filter_continent(indexes, string::upper(val.to<std::string_view>()));
        // }
        else if (key == "name") {
            filter_name(aChartSelectInterface, indexes, val.to<std::string_view>());
        }
        else if (key == "full_name") {
            filter_full_name(aChartSelectInterface, indexes, string::upper(val.to<std::string_view>()));
        }
        else if (key == "location") {
            filter_location(aChartSelectInterface, indexes, string::upper(val.to<std::string_view>()));
        }
        else if (key == "table") {
            filter_table(aChartSelectInterface, indexes, val.to<std::string_view>());
        }
        else if (key == "date_range") {
            filter_date_range(aChartSelectInterface, indexes, val[0].to<std::string_view>(), val[1].to<std::string_view>());
        }
        else if (key == "in_rectangle") {
            const auto& c1 = val["c1"];
            const auto& c2 = val["c2"];
            filter_rectangle(aChartSelectInterface, indexes, {c1[0].to<double>(), c1[1].to<double>(), c2[0].to<double>(), c2[1].to<double>()},
                             rjson::get_or(val, "transform", true) ? transformed_t::yes : transformed_t::no, RotationRadiansOrDegrees(rjson::get_or(val, "rotate", 0.0)));
        }
        else if (key == "in_circle") {
            const auto& center = val["center"];
            const double radius{val["radius"].to<double>()};
            filter_circle(aChartSelectInterface, indexes, {{center[0].to<double>(), center[1].to<double>()}, radius});
        }
        else if (key == "titrated_against_antigens") {
            const auto antigen_indexes = SelectAntigens().command(aChartSelectInterface, val);
            filter_titrated_against(aChartSelectInterface, indexes, antigen_indexes);
            // std::cerr << "DEBUG: titrated_against_antigens " << antigen_indexes << ' ' << indexes << '\n';
        }
        else if (key == "outline") {
            filter_outline(aChartSelectInterface, indexes, Color(val.to<std::string_view>()));
        }
        else if (key == "outline_width") {
            filter_outline_width(aChartSelectInterface, indexes, Pixels{val.to<double>()});
        }
        else {
            filter(*sera, indexes, *this, aChartSelectInterface, aSelector, key, val);
        }
    });
    if (verbose()) {
        fmt::print("INFO: sera selected: {:4d} {}\n", indexes->size(), aSelector);
        if (!indexes->empty())
            fmt::print(stderr, "{}", report_sera(indexes, aChartSelectInterface, report_names_threshold()));
    }
    return indexes;

} // SelectSera::command

// ----------------------------------------------------------------------

void SelectAntigens::filter_table(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aTable)
{
    const auto& hidb = hidb::get(aChartSelectInterface.chart().info()->virus_type());
    acmacs::map_draw::select::filter::table_ag_sr(hidb.antigens()->find(*aChartSelectInterface.chart().antigens()), indexes, aTable, hidb.tables());

} // SelectAntigens::filter_table

void SelectSera::filter_table(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aTable)
{
    const auto& hidb = hidb::get(aChartSelectInterface.chart().info()->virus_type());
    acmacs::map_draw::select::filter::table_ag_sr(hidb.sera()->find(*aChartSelectInterface.chart().sera()), indexes, aTable, hidb.tables());

} // SelectSera::filter_table

// ----------------------------------------------------------------------

// template <typename F> void filter_layer_ag_sr(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, int aLayer, F first_second)
// {
//     auto titers = aChartSelectInterface.chart().titers();
//     if (aLayer < 0)
//         aLayer = static_cast<int>(titers->number_of_layers()) + aLayer;
//     if (aLayer < 0 || aLayer > static_cast<int>(titers->number_of_layers()))
//         throw std::runtime_error(fmt::format("Invalid layer: {}", aLayer));

//     indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), [antigens_in_layer=first_second(titers->antigens_sera_of_layer(static_cast<size_t>(aLayer)))](size_t index) {
//         return std::find(std::begin(antigens_in_layer), std::end(antigens_in_layer), index) == std::end(antigens_in_layer);
//     }), indexes.end());
// }

void SelectAntigens::filter_layer(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, int aLayer)
{
    acmacs::map_draw::select::filter::layer(aChartSelectInterface.chart(), indexes, aLayer, acmacs::map_draw::select::filter::antigens);

} // SelectAntigens::filter_layer

void SelectSera::filter_layer(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, int aLayer)
{
    acmacs::map_draw::select::filter::layer(aChartSelectInterface.chart(), indexes, aLayer, acmacs::map_draw::select::filter::sera);

} // SelectSera::filter_layer

// ----------------------------------------------------------------------

void SelectSera::filter_date_range(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view from, std::string_view to)
{
    const auto& chart = aChartSelectInterface.chart();
    chart.set_homologous(acmacs::chart::find_homologous::relaxed);
    auto antigens = chart.antigens();
    auto sera = chart.sera();

    auto antigen_indexes = antigens->all_indexes();
    antigens->filter_date_range(antigen_indexes, from, to);

    auto homologous_filtered_out_by_date_range = [&](auto serum_index) -> bool {
        for (auto antigen_index : sera->at(serum_index)->homologous_antigens()) {
            if (antigen_indexes.contains(antigen_index))
                return false;   // homologous antigen selected by date range, do not remove this serum from indexes
        }
        return true;
    };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), homologous_filtered_out_by_date_range), indexes.end());

} // SelectSera::filter_date_range

// ----------------------------------------------------------------------

// via seqdb.clades_for_name
void SelectSera::filter_clade(ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aClade)
{
    const auto& chart = aChartSelectInterface.chart();

    auto not_in_clade = [&aClade, &chart](auto serum_index) -> bool {
        const auto clades = acmacs::seqdb::get().clades_for_name(chart.sera()->at(serum_index)->name());
        return std::find(std::begin(clades), std::end(clades), aClade) == std::end(clades);
    };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), not_in_clade), indexes.end());

} // SelectSera::filter_clade

// ----------------------------------------------------------------------

// via homologous antigen
// void SelectSera::filter_clade(ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string aClade)
// {
//     const auto& chart = aChartSelectInterface.chart();
//     chart.set_homologous(acmacs::chart::find_homologous::strict);
//     const auto& entries = aChartSelectInterface.match_seqdb();
//     auto homologous_not_in_clade = [&entries, aClade, &chart](auto serum_index) -> bool {
//         for (auto antigen_index : chart.sera()->at(serum_index)->homologous_antigens()) {
//             if (const auto& entry = entries[antigen_index]; entry && entry.seq().has_clade(aClade))
//                 return false;
//         }
//         return true;
//     };
//     indexes.erase(std::remove_if(indexes.begin(), indexes.end(), homologous_not_in_clade), indexes.end());

// } // SelectSera::filter_clade

// ----------------------------------------------------------------------

void SelectSera::filter_amino_acid_at_pos(ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::seqdb::amino_acid_at_pos1_eq_list_t& pos1_aa)
{
    const auto& chart = aChartSelectInterface.chart();
    chart.set_homologous(acmacs::chart::find_homologous::relaxed);
    auto antigens = chart.antigens();
    auto sera = chart.sera();

    auto antigen_indexes = antigens->all_indexes();
    SelectAntigens().filter_amino_acid_at_pos(aChartSelectInterface, antigen_indexes, pos1_aa);

    auto homologous_filtered_out_by_amino_acid = [&](auto serum_index) -> bool {
        // fmt::print(stderr, "DEBUG: SR {} {}  HOMOL: {}\n", serum_index, sera->at(serum_index)->name_full(), sera->at(serum_index)->homologous_antigens());
        for (auto antigen_index : sera->at(serum_index)->homologous_antigens()) {
            if (antigen_indexes.contains(antigen_index))
                return false;   // homologous antigen selected by pos1_aa, do not remove this serum from indexes
        }
        // fmt::print(stderr, "DEBUG: !!! {} SR {} {}  HOMOL: {}\n", pos1_aa, serum_index, sera->at(serum_index)->name_full(), sera->at(serum_index)->homologous_antigens());
        return true;
    };
    indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), homologous_filtered_out_by_amino_acid), indexes.end());

} // SelectSera::filter_amino_acid_at_pos

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
