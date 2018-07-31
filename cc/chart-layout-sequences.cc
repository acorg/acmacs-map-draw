#include <iostream>

#include "acmacs-base/argc-argv.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "acmacs-map-draw/export.hh"

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    int exit_code = 0;
    try {
        argc_argv args(argc, argv, {
                {"--projection", 0L},
                {"--time", false, "report time of loading chart"},
                {"--verbose", false},
                {"-h", false},
                {"--help", false},
                {"-v", false},
        });
        if (args["-h"] || args["--help"] || args.number_of_arguments() != 2) {
            std::cerr << "Usage: " << args.program() << " [options] <chart-file> <output.csv[.xz]>\n" << args.usage_options() << '\n';
            exit_code = 1;
        }
        else {
            const auto report_time = do_report_time(args["--time"]);
            auto chart = acmacs::chart::import_from_file(args[0], acmacs::chart::Verify::None, report_time);
            export_layout_sequences_into_csv(args[1], *chart, args["--projection"]);
        }
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        exit_code = 2;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
