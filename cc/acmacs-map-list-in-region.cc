#include <iostream>
#include <string>
using namespace std::string_literals;

//#include "acmacs-base/filesystem.hh"
#include "acmacs-base/timeit.hh"
#include "acmacs-base/enumerate.hh"
#include "acmacs-chart/ace.hh"
// #include "seqdb/seqdb.hh"
#include "acmacs-map-draw/draw.hh"

#include "settings.hh"
#include "mod-applicator.hh"

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    try {
        if (argc < 2)
            throw std::runtime_error("Usage: "s + argv[0] + " <chart>");
        size_t projection_no = 0;

        auto settings = default_settings();

        Timeit ti_chart{"loading chart from "s + argv[1] + ": "};
        std::unique_ptr<Chart> chart{import_chart(argv[1])};
        ti_chart.report();

        ChartDraw chart_draw(*chart, projection_no);
        chart_draw.prepare();

        auto mods = rjson::parse_string(R"(["all_grey", {"N": "clades", "seqdb_file": "/Users/eu/AD/data/seqdb.json.xz", "report": true}])");
        apply_mods(chart_draw, mods, settings);

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
