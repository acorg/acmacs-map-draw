#include <iostream>
#include <string>
using namespace std::string_literals;

#include "acmacs-base/argc-argv.hh"
// #include "acmacs-base/enumerate.hh"
#include "acmacs-chart/ace.hh"
#include "acmacs-map-draw/draw.hh"

#include "settings.hh"
#include "mod-applicator.hh"

// ----------------------------------------------------------------------

constexpr const char* sUsage = " [options] <chart.ace> [<map.pdf>]\n";

int main(int argc, char* const argv[])
{
    try {
        argc_argv args(argc, argv, {
                {"-h", false},
                {"--help", false},
                {"-s", ""},
                {"--settings", ""},
                {"--projection", 0L},
                {"--seqdb", ""},
                {"--hidb-dir", ""},
                {"--locdb", ""},
                {"-v", false},
                {"--verbose", false},
            });
        if (args["-h"] || args["--help"] || args.number_of_arguments() < 1 || args.number_of_arguments() > 2) {
            throw std::runtime_error("Usage: "s + args.program() + sUsage + args.usage_options());
        }
        const bool verbose = args["-v"] || args["--verbose"];

        auto settings = default_settings();
        std::unique_ptr<Chart> chart{import_chart(args[0], verbose ? report_time::Yes : report_time::No)};

        ChartDraw chart_draw(*chart, args["--projection"]);
        chart_draw.prepare();

        auto mods = rjson::parse_string(R"(["all_grey", {"N": "clades", "seqdb_file": "/Users/eu/AD/data/seqdb.json.xz", "report": false}])");
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
