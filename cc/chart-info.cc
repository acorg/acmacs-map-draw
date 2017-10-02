#include <string>
#include <fstream>

#include "acmacs-base/argc-argv.hh"
#include "acmacs-chart/ace.hh"

#include "select.hh"

using namespace std::string_literals;

// ----------------------------------------------------------------------

constexpr const char* sUsage = R"( [options] <chart.ace>
  --clades
  --seqdb <seqdb.json.xz>
  --locdb <locationdb.json.xz>
  --hidb-dir <~/AD/data>
  -v|--verbose
)";

int main(int argc, char* const argv[])
{
    try {
        argc_argv_simple args(argc, argv, {"--seqdb", "--hidb-dir", "--locdb"});
        if (args["-h"] || args["--help"] || args.number_of_arguments() != 1)
            throw std::runtime_error("\nUsage: "s + args.program() + sUsage);
        const bool verbose = args["-v"] || args["--verbose"];
        std::unique_ptr<Chart> chart{import_chart(args[0], verbose ? report_time::Yes : report_time::No)};

        std::cout << chart->make_name() << '\n'
                << "  AG: " << chart->number_of_antigens() << '\n'
                << "  SR: " << chart->number_of_sera() << '\n'
                << "  P : " << chart->number_of_projections() << '\n';
        if (chart->number_of_projections())
            std::cout << "  S : " << chart->projection(0).stress() << '\n';
        if (args["--clades"]) {
            SelectAntigens selector(args.get("--locdb", ""s), args.get("--hidb-dir", ""s), args.get("--seqdb", ""s), verbose);
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
