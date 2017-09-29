#include <string>
#include <fstream>

#include "acmacs-base/argc-argv.hh"
// #include "acmacs-base/string.hh"
#include "acmacs-chart/ace.hh"

#include "select.hh"

using namespace std::string_literals;

// ----------------------------------------------------------------------

constexpr const char* sUsage = R"( [options] <chart.ace> <command-in-json-format>
  -s|--sera
  --seqdb <seqdb.json.xz>
  --locdb <locationdb.json.xz>
  --hidb-dir <~/AD/data>
  -v|--verbose
)";

int main(int argc, char* const argv[])
{
    try {
        argc_argv args(argc, argv, {"--seqdb", "--hidb-dir", "--locdb"});
        if (args["-h"] || args["--help"] || args.number_of_arguments() != 2)
            throw std::runtime_error("Usage: "s + args.program() + sUsage);
        const bool verbose = args["-v"] || args["--verbose"];
        const auto selector = rjson::parse_string(args[1]);
        std::unique_ptr<Chart> chart{import_chart(args[0], verbose ? report_time::Yes : report_time::No)};

        if (!args["-s"] && !args["--sera"]) {
            const auto num_digits = static_cast<int>(std::log10(chart->number_of_antigens())) + 1;
            const auto indices = SelectAntigens(args.get("--locdb", ""s), args.get("--hidb-dir", ""s), args.get("--seqdb", ""s), verbose).select(*chart, selector);
            for (auto index: indices)
                std::cout << "AG " << std::setfill(' ') << std::setw(num_digits) << index << ' ' << chart->antigen(index).full_name() << '\n';
        }
        else {
            const auto num_digits = static_cast<int>(std::log10(chart->number_of_sera())) + 1;
            const auto indices = SelectSera(args.get("--locdb", ""s), args.get("--hidb-dir", ""s), args.get("--seqdb", ""s), verbose).select(*chart, selector);
            for (auto index: indices)
                std::cout << "SR " << std::setfill(' ') << std::setw(num_digits) << index << ' ' << chart->serum(index).full_name() << '\n';
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