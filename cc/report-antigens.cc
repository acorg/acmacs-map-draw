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
        const auto& seqdb = acmacs::seqdb::get();
        const auto& matched_seqdb = aChartSelectInterface.match_seqdb();
        auto layout = aChartSelectInterface.layout();
        auto antigens = chart.antigens();
        auto titers = chart.titers();
        auto info = chart.info();
        fmt::format_to(output, "  AG ({}) {}\n", indexes.size(), acmacs::string::join(acmacs::string::join_comma, std::begin(indexes), std::end(indexes)));

        size_t seq_max{0}, clades_max{0};
        for (auto index : indexes) {
            if (const auto& ref = matched_seqdb[index]; ref) {
                seq_max = std::max(seq_max, ref.seq_id().size());
                clades_max = std::max(clades_max, fmt::format("{}", ref.seq_with_sequence(seqdb).clades).size());
            }
        }

        const auto full_name_max{acmacs::chart::max_full_name(*antigens, indexes)};
        for (auto index : indexes) {
            const auto antigen = antigens->at(index);
            const auto coord = layout->at(index);
            fmt::format_to(output, "  AG {:5d} {: <{}} {:10s} {: <6s}", index, fmt::format("\"{}\"", antigen->full_name()), full_name_max + 2, antigen->date(), antigen->passage().passage_type());
            if (const auto& ref = matched_seqdb[index]; ref)
                fmt::format_to(output, " {:<{}s} {:<{}s}", ref.seq_id(), seq_max, fmt::format("{}", ref.seq_with_sequence(seqdb).clades), clades_max);
            else
                fmt::format_to(output, " {:{}c}", ' ', seq_max + clades_max + 1);
            if (coord.exists())
                fmt::format_to(output, " {:8.4f}", coord);
            else
                fmt::format_to(output, "    <disconnected>   ");
            if (titers->number_of_layers() > 1) {
                std::vector<std::string> layers;
                for (size_t layer_no : titers->layers_with_antigen(index))
                    layers.emplace_back(info->source(layer_no)->date());
                fmt::format_to(output, " {}", layers);
            }
            fmt::format_to(output, "\n");
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
        auto sera = chart.sera();
        const auto number_of_antigens = chart.number_of_antigens();
        const auto full_name_max{acmacs::chart::max_full_name(*sera, indexes)};
        auto layout = aChartSelectInterface.layout();
        fmt::format_to(output, "  SR ({}) {}\n", indexes.size(), acmacs::string::join(acmacs::string::join_comma, std::begin(indexes), std::end(indexes)));
        for (auto index : indexes) {
            const auto serum = sera->at(index);
            const auto coord = layout->at(index + number_of_antigens);
            fmt::format_to(output, "  SR {:5d} {: <{}} {: <6s}", index, fmt::format("\"{}\"", serum->full_name()), full_name_max + 2, serum->passage().passage_type());
            if (coord.exists())
                fmt::format_to(output, " {:8.4f}", coord);
            else
                fmt::format_to(output, "    <disconnected>   ");
            fmt::format_to(output, "\n");
        }
    }
    return fmt::to_string(output);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
