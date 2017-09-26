#include <iostream>
#include <string>
using namespace std::string_literals;

#include "acmacs-base/filesystem.hh"
#include "acmacs-base/timeit.hh"
#include "acmacs-base/enumerate.hh"
#include "acmacs-chart/ace.hh"
#include "seqdb/seqdb.hh"
#include "acmacs-map-draw/draw.hh"

#include "settings.hh"

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    try {
        if (argc < 2)
            throw std::runtime_error("Usage: "s + argv[0] + " <chart>");
        size_t projection_no = 0;
        auto seqdb_file = fs::path{"/Users/eu/AD/data/seqdb.json.xz"};

        const std::map<std::string, const char*> CladeColor = {
            {"", "grey50"},
              // H3
            {"3C3", "cornflowerblue"},
            {"3C2a", "red"},
            {"3C2a1", "darkred"},
            {"3C3a", "green"},
            {"3C3b", "blue"},
              // H1pdm
            {"6B1", "blue"},
            {"6B2", "red"},
              // B/Yam
            {"Y2", "cornflowerblue"},
            {"Y3", "red"},
              // B/Vic
            {"1", "blue"},
            {"1A", "cornflowerblue"},
            {"1B", "red"},
        };

        auto settings = default_settings();
        std::cout << settings.to_json_pp() << '\n';

        const auto& seqdb = seqdb::get(seqdb_file, true);

        Timeit ti_chart{"loading chart from "s + argv[1] + ": "};
        std::unique_ptr<Chart> chart{import_chart(argv[1])};
        ti_chart.report();

        Timeit ti_match{"matching seqdb: "};
        std::vector<seqdb::SeqdbEntrySeq> seqdb_entries;
        seqdb.match(chart->antigens(), seqdb_entries, true);
        ti_match.report();

        ChartDraw chart_draw(*chart, projection_no);
        chart_draw.prepare();

          // all grey
        for (auto [ag_no, antigen]: enumerate(chart->antigens())) {
            chart_draw.modify(ag_no, PointStyleEmpty().fill(antigen.reference() ? TRANSPARENT : GREY).outline(GREY));
        }
        for (auto [sr_no, serum]: enumerate(chart->sera())) {
            chart_draw.modify_serum(sr_no, PointStyleEmpty().fill(TRANSPARENT).outline(GREY));
              // chart_draw.modify_serum(sr_no, PointStyleEmpty().outline("grey"));
        }

        for (auto [ag_no, entry_seq]: enumerate(seqdb_entries)) {
            if (entry_seq) {
                if (const auto& clades = entry_seq.seq().clades(); !clades.empty()) {
                    if (const auto clr = CladeColor.find(clades.front()); clr != CladeColor.end())
                        chart_draw.modify(ag_no, PointStyleEmpty().fill(clr->second).outline(BLACK), ChartDraw::Raise);
                }
            }
        }

        chart_draw.calculate_viewport();
        chart_draw.draw("/r/aaa.pdf", 800);
        return 0;
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        return 1;
    }
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
