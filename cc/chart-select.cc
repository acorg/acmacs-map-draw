#include <string>
#include <fstream>

#include "acmacs-base/argc-argv.hh"
#include "acmacs-chart-2/factory-import.hh"

#include "setup-dbs.hh"
#include "settings.hh"
#include "select.hh"

using namespace std::string_literals;

// ----------------------------------------------------------------------

static int do_select(const argc_argv& args);

int main(int argc, char* const argv[])
{
    int exit_code = 1;
    try {
        argc_argv args(argc, argv, {
                {"-s", false, "select sera (alias for --sera)"},
                {"--sera", false, "select sera (alias for -s)"},
                {"--just-indexes", false, "report just indexes, comma separated"},
                {"--projection", 0},
                {"-h", false},
                {"--help", false},
                {"--help-select", false},
                {"--db-dir", ""},
                {"--time", false, "report time of loading chart"},
                {"--verbose", false},
                {"-v", false},
            });
        if (args["--help-select"])
            std::cerr << settings_help_select();
        else if (args["-h"] || args["--help"] || args.number_of_arguments() != 2)
            std::cerr << "Usage: " << args.program() << " [options] <chart.ace> <command-in-json-format>\n" << args.usage_options() << '\n';
        else
            exit_code = do_select(args);
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        exit_code = 2;
    }
    return exit_code;
}

// ----------------------------------------------------------------------

int do_select(const argc_argv& args)
{
    const bool verbose = args["-v"] || args["--verbose"];
    setup_dbs(args["--db-dir"].str(), verbose);
    const auto selector = rjson::parse_string(args[1]);
    auto chart = acmacs::chart::import_from_file(args[0], acmacs::chart::Verify::None, do_report_time(args["--time"]));
    ChartSelectInterface chart_select(std::make_shared<acmacs::chart::ChartModify>(chart), args["--projection"]);
    if (!args["-s"] && !args["--sera"]) {
        const auto num_digits = static_cast<int>(std::log10(chart->number_of_antigens())) + 1;
        const auto indices = SelectAntigens(verbose).select(chart_select, selector);
        if (args["--just-indexes"]) {
            std::cout << string::join(",", indices) << '\n';
        }
        else {
            for (auto index : indices)
                std::cout << "AG " << std::setfill(' ') << std::setw(num_digits) << index << ' ' << chart->antigen(index)->full_name() << '\n';
        }
    }
    else {
        const auto num_digits = static_cast<int>(std::log10(chart->number_of_sera())) + 1;
        const auto indices = SelectSera(verbose).select(chart_select, selector);
        if (args["--just-indexes"]) {
            std::cout << string::join(",", indices) << '\n';
        }
        else {
            for (auto index : indices)
                std::cout << "SR " << std::setfill(' ') << std::setw(num_digits) << index << ' ' << chart->serum(index)->full_name() << '\n';
        }
    }
    return 0;

} // do_select

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
