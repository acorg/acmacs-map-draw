#include "acmacs-base/string-join.hh"
#include "seqdb-3/seqdb.hh"
#include "acmacs-map-draw/report-antigens.hh"
#include "acmacs-map-draw/chart-select-interface.hh"

// ----------------------------------------------------------------------

void report_antigens(const acmacs::chart::PointIndexList& indexes, const ChartSelectInterface& aChartSelectInterface, size_t threshold)
{
    using namespace std::string_literals;

    if (threshold >= indexes.size()) {
        const auto& chart = aChartSelectInterface.chart();
        const auto& seqdb = acmacs::seqdb::get();
        const auto& matched_seqdb = aChartSelectInterface.match_seqdb();
        auto layout = aChartSelectInterface.layout();
        auto antigens = chart.antigens();
        auto titers = chart.titers();
        auto info = chart.info();
        fmt::print(stderr, "  AG ({}) {}\n", indexes.size(), acmacs::string::join(acmacs::string::join_comma, std::begin(indexes), std::end(indexes)));

        size_t full_name_max{0}, seq_max{0}, clades_max{0};
        for (auto index : indexes) {
            full_name_max = std::max(full_name_max, antigens->at(index)->full_name().size());
            if (const auto& ref = matched_seqdb[index]; ref) {
                seq_max = std::max(seq_max, ref.seq_id().size());
                clades_max = std::max(clades_max, fmt::format("{}", ref.seq_with_sequence(seqdb).clades).size());
            }
        }

        for (auto index : indexes) {
            const auto antigen = antigens->at(index);
            const auto coord = layout->at(index);
            fmt::print(stderr, "  AG {:5d} {: <{}} {:10s} {: <6s}", index, fmt::format("\"{}\"", antigen->full_name()), full_name_max + 2, antigen->date(), antigen->passage().passage_type());
            if (const auto& ref = matched_seqdb[index]; ref)
                fmt::print(stderr, " {:<{}s} {:<{}s}", ref.seq_id(), seq_max, fmt::format("{}", ref.seq_with_sequence(seqdb).clades), clades_max);
            else
                fmt::print(stderr, " {:{}c}", ' ', seq_max + clades_max + 1);
            if (coord.exists())
                fmt::print(stderr, " {:8.4f}", coord);
            else
                fmt::print(stderr, "    <disconnected>   ");
            if (titers->number_of_layers() > 1) {
                std::vector<std::string> layers;
                for (size_t layer_no : titers->layers_with_antigen(index))
                    layers.emplace_back(info->source(layer_no)->date());
                fmt::print(stderr, " {}", layers);
            }
            fmt::print(stderr, "\n");
        }
    }
}

// ----------------------------------------------------------------------

void report_sera(const acmacs::chart::PointIndexList& indexes, const ChartSelectInterface& aChartSelectInterface, size_t threshold)
{
    if (threshold >= indexes.size()) {
        const auto& chart = aChartSelectInterface.chart();
        auto sera = chart.sera();
        fmt::print(stderr, "  SR ({}) {}\n", indexes.size(), acmacs::string::join(acmacs::string::join_comma, std::begin(indexes), std::end(indexes)));
        for (auto index : indexes) {
            const auto serum = sera->at(index);
            fmt::print(stderr, "  SR {:5d} \"{}\"\n", index, serum->full_name());
        }
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
