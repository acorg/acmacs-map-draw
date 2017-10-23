#include <string>
#include <fstream>

#include "acmacs-base/argc-argv.hh"
#include "acmacs-chart/ace.hh"

#include "setup-dbs.hh"
#include "select.hh"

using namespace std::string_literals;

// ----------------------------------------------------------------------

constexpr const char* sUsage = " [options] <chart.ace>\n";

int main(int argc, char* const argv[])
{
    try {
        argc_argv args(argc, argv, {
                {"--clades", false},
                {"--db-dir", ""},
                {"-v", false},
                {"--verbose", false},
                {"-h", false},
                {"--help", false},
            });
        if (args["-h"] || args["--help"] || args.number_of_arguments() != 1)
            throw std::runtime_error("\nUsage: "s + args.program() + sUsage + args.usage_options());
        const bool verbose = args["-v"] || args["--verbose"];
        setup_dbs(args["--db-dir"], verbose);
        std::unique_ptr<Chart> chart{import_chart(args[0], verbose ? report_time::Yes : report_time::No)};

        std::cout << chart->make_name() << '\n'
                << "  AG: " << chart->number_of_antigens() << '\n'
                << "  SR: " << chart->number_of_sera() << '\n'
                << "  P : " << chart->number_of_projections() << '\n';
        if (chart->number_of_projections())
            std::cout << "  S : " << chart->projection(0).stress() << '\n';
        if (args["--clades"]) {
            SelectAntigens selector(verbose);
            std::cout << "  Clades:\n" << std::setfill(' ');
            for (auto [clade,number]: selector.clades(*chart)) {
                std::cout << "    " << std::setw(6) << std::left << clade
                          << ": " << std::setw(5) << std::right << number << '\n';
            }
        }
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
