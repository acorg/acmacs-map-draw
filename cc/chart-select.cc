#include <string>
#include <fstream>

#include "acmacs-base/argc-argv.hh"
// #include "acmacs-base/string.hh"
#include "acmacs-chart/ace.hh"

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
                {"-h", false},
                {"--help", false},
                {"--help-select", false},
                {"-s", false},
                {"--sera", false},
                {"--db-dir", ""},
                {"-v", false},
                {"--verbose", false},
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
    setup_dbs(args["--db-dir"]);
    const auto selector = rjson::parse_string(args[1]);
      // const auto selector = rjson::parse_string("{\"in_rectangle\":{\"c1\":[0,0],\"c2\":[1,1]}}");
    std::unique_ptr<Chart> chart{import_chart(args[0], verbose ? report_time::Yes : report_time::No)};

    if (!args["-s"] && !args["--sera"]) {
        const auto num_digits = static_cast<int>(std::log10(chart->number_of_antigens())) + 1;
        const auto indices = SelectAntigens(verbose).select(*chart, selector);
        for (auto index: indices)
            std::cout << "AG " << std::setfill(' ') << std::setw(num_digits) << index << ' ' << chart->antigen(index).full_name() << '\n';
    }
    else {
        const auto num_digits = static_cast<int>(std::log10(chart->number_of_sera())) + 1;
        const auto indices = SelectSera(verbose).select(*chart, selector);
        for (auto index: indices)
            std::cout << "SR " << std::setfill(' ') << std::setw(num_digits) << index << ' ' << chart->serum(index).full_name() << '\n';
    }
    return 0;

} // do_select

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
