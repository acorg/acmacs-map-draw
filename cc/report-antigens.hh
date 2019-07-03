#pragma once

#include "acmacs-base/fmt.hh"
#include "acmacs-base/string.hh"
#include "acmacs-chart-2/chart.hh"

// ----------------------------------------------------------------------

template <typename Iter> inline void report_antigens(Iter first, Iter last, const acmacs::chart::Chart& chart, const acmacs::Layout& layout, size_t threshold = 10)
{
    if (threshold >= static_cast<size_t>(last - first)) {
        auto antigens = chart.antigens();
        auto titers = chart.titers();
        auto info = chart.info();
        fmt::print(stderr, "  AG ({}) {}\n", last - first, string::join(",", first, last));
        const auto full_name_max =
            antigens->at(*std::max_element(first, last, [&antigens](size_t i1, size_t i2) { return antigens->at(i1)->full_name().size() < antigens->at(i2)->full_name().size(); }))->full_name().size();
        for (; first != last; ++first) {
            const auto antigen = antigens->at(*first);
            const auto disconnected = !layout.point_has_coordinates(*first);
            fmt::print(stderr, "  AG {:5d} {: <{}} {:10s} {: <6s}{}", *first, antigen->full_name(), full_name_max, antigen->date(),
                       antigen->passage().passage_type(), disconnected ? " <disconnected>" : ""); // antigen->full_name_with_fields()
            if (titers->number_of_layers() > 1) {
                std::vector<std::string> layers;
                for (size_t layer_no : titers->layers_with_antigen(*first))
                    layers.push_back(info->source(layer_no)->date());
                fmt::print(stderr, " layers:{}", layers);
            }
            fmt::print(stderr, "\n");
        }
    }
}

// ----------------------------------------------------------------------

template <typename Iter> inline void report_sera(Iter first, Iter last, const acmacs::chart::Chart& chart, size_t threshold = 10)
{
    if (threshold >= static_cast<size_t>(last - first)) {
        auto sera = chart.sera();
        fmt::print(stderr, "  SR ({}) {}\n", last - first, string::join(",", first, last));
        for (; first != last; ++first) {
            const auto serum = sera->at(*first);
            fmt::print(stderr, "  SR {:5d} {}\n", *first, serum->full_name());
        }
    }
}


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End: