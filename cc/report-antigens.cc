#include "acmacs-base/string-join.hh"
#include "seqdb-3/seqdb.hh"
#include "acmacs-map-draw/report-antigens.hh"
#include "acmacs-map-draw/chart-select-interface.hh"

// ----------------------------------------------------------------------

std::string report_antigens(const acmacs::chart::PointIndexList& indexes, const ChartSelectInterface& aChartSelectInterface, size_t threshold)
{
    using namespace std::string_literals;

    fmt::memory_buffer output;
    if (threshold >= indexes.size()) {
        const auto& chart = aChartSelectInterface.chart();
        auto layout = aChartSelectInterface.chart(0).modified_layout();
        auto antigens = chart.antigens();
        auto titers = chart.titers();
        auto info = chart.info();
        fmt::format_to_mb(output, "  AG ({}) {}\n", indexes.size(), acmacs::string::join(acmacs::string::join_comma, std::begin(indexes), std::end(indexes)));

        // const auto& seqdb = acmacs::seqdb::get();
        // const auto& matched_seqdb = aChartSelectInterface.chart(0).match_seqdb();
        // size_t seq_max{0}, clades_max{0};
        // for (auto index : indexes) {
        //     if (const auto& ref = matched_seqdb[index]; ref) {
        //         seq_max = std::max(seq_max, ref.seq_id().size());
        //         clades_max = std::max(clades_max, fmt::format("{}", ref.seq_with_sequence(seqdb).clades).size());
        //     }
        // }

        const auto full_name_max{acmacs::chart::max_full_name(*antigens, indexes)};
        for (auto index : indexes) {
            const auto antigen = antigens->at(index);
            const auto coord = layout->at(index);
            fmt::format_to_mb(output, "  AG {:5d} {: <{}} {:10s} {: <6s}", index, fmt::format("\"{}\"", antigen->name_full()), full_name_max + 2, antigen->date(), antigen->passage().passage_type());
            // if (const auto& ref = matched_seqdb[index]; ref)
            //     fmt::format_to_mb(output, " {:<{}s} {:<{}s}", ref.seq_id(), seq_max, fmt::format("{}", ref.seq_with_sequence(seqdb).clades), clades_max);
            // else
            //     fmt::format_to_mb(output, " {:{}c}", ' ', seq_max + clades_max + 1);
            if (coord.exists())
                fmt::format_to_mb(output, fmt::runtime(" {:8.4f}"), coord);
            else
                fmt::format_to_mb(output, "    <disconnected>   ");
            if (titers->number_of_layers() > 1) {
                std::vector<std::string> layers;
                for (size_t layer_no : titers->layers_with_antigen(index))
                    layers.emplace_back(info->source(layer_no)->date());
                fmt::format_to_mb(output, " layers:{}{}", layers.size(), layers);
            }
            fmt::format_to_mb(output, "\n");
        }
    }
    return fmt::to_string(output);
}

// ----------------------------------------------------------------------

std::string report_sera(const acmacs::chart::PointIndexList& indexes, const ChartSelectInterface& aChartSelectInterface, size_t threshold)
{
    fmt::memory_buffer output;
    if (threshold >= indexes.size()) {
        const auto& chart = aChartSelectInterface.chart();
        chart.set_homologous(acmacs::chart::find_homologous::all);
        auto sera = chart.sera();
        const auto number_of_antigens = chart.number_of_antigens();
        auto layout = aChartSelectInterface.chart(0).modified_layout();
        auto titers = chart.titers();
        auto info = chart.info();
        fmt::format_to_mb(output, "  SR ({}) {}\n", indexes.size(), acmacs::string::join(acmacs::string::join_comma, std::begin(indexes), std::end(indexes)));

        // const auto& seqdb = acmacs::seqdb::get();
        // const auto& matched_seqdb = aChartSelectInterface.chart(0).match_seqdb();
        // using seq_data_t = std::pair<acmacs::seqdb::seq_id_t, std::string>;
        // acmacs::small_map_with_unique_keys_t<size_t, seq_data_t> seq_clades;
        // size_t seq_max{0}, clades_max{0};
        // for (auto index : indexes) {
        //     for (const auto antigen_index : sera->at(index)->homologous_antigens()) {
        //         if (const auto& ref = matched_seqdb[antigen_index]; ref) {
        //             const auto seq_id = ref.seq_id();
        //             seq_max = std::max(seq_max, seq_id.size());
        //             const auto clades = fmt::format("{}", ref.seq_with_sequence(seqdb).clades);
        //             clades_max = std::max(clades_max, clades.size());
        //             seq_clades.emplace_or_replace(index, seq_id, clades);
        //             break;
        //         }
        //     }
        // }

        const auto full_name_max{acmacs::chart::max_full_name(*sera, indexes)};
        for (auto index : indexes) {
            const auto serum = sera->at(index);
            const auto coord = layout->at(index + number_of_antigens);
            fmt::format_to_mb(output, "  SR {:5d} {: <{}} {: <6s}", index, fmt::format("\"{}\"", serum->name_full()), full_name_max + 2, serum->passage_type());
            // const auto& seq_data = seq_clades.get_or(index, seq_data_t{});
            // fmt::format_to_mb(output, " {:<{}s} {:<{}s}", seq_data.first, seq_max, seq_data.second, clades_max);
            if (coord.exists())
                fmt::format_to_mb(output, fmt::runtime(" {:8.4f}"), coord);
            else
                fmt::format_to_mb(output, "    <disconnected>   ");
            if (titers->number_of_layers() > 1) {
                std::vector<std::string> layers;
                for (size_t layer_no : titers->layers_with_serum(index))
                    layers.emplace_back(info->source(layer_no)->date());
                fmt::format_to_mb(output, " {:3d}{}", layers.size(), layers);
            }
            fmt::format_to_mb(output, "\n");
        }
    }
    return fmt::to_string(output);
}

// ----------------------------------------------------------------------
