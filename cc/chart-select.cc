#include <string>
#include <fstream>

#include "acmacs-base/argc-argv.hh"
// #include "acmacs-base/string.hh"
#include "acmacs-chart/ace.hh"
//#include "seqdb/seqdb.hh"

#include "select.hh"

using namespace std::string_literals;

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    try {
        argc_argv args(argc, argv, {"--seqdb", "--hidb-dir"});
        if (args["-h"] || args["--help"] || args.number_of_arguments() != 2)
            throw std::runtime_error("Usage: "s + args.program() + " [--seqdb <seqdb.json.xz>] [--hidb-dir <~/AD/data>] [-s (sera)] <chart.ace> <command-in-json-format>");
        const auto selector = rjson::parse_string(args[1]);
        std::unique_ptr<Chart> chart{import_chart(args[0])};

        const auto indices = args["-s"] ? SelectSera{}.select(*chart, selector) : SelectAntigens{}.select(*chart, selector);
        std::cout << indices << '\n';

          // const auto& seqdb = seqdb::get(args.get("--seqdb", "/Users/eu/AD/data/seqdb.json.xz"));
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
