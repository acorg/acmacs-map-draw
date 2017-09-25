#include <iostream>
#include <string>
using namespace std::string_literals;

#include "acmacs-base/filesystem.hh"
#include "acmacs-base/timeit.hh"
#include "acmacs-chart/ace.hh"
#include "seqdb/seqdb.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    try {
        if (argc < 2)
            throw std::runtime_error("Usage: "s + argv[0] + " <chart>");
        size_t projection_no = 0;
        auto seqdb_file = fs::path{"/Users/eu/AD/data/seqdb.json.xz"};

        Timeit ti_seqdb{"loading seqdb from "s + static_cast<std::string>(seqdb_file) + ": "};
        seqdb::Seqdb seqdb;
        seqdb.load(seqdb_file);
        ti_seqdb.report();

        std::unique_ptr<Chart> chart{import_chart(argv[1])};
        ChartDraw chart_draw(*chart, projection_no);
        chart_draw.prepare();
          // apply mods
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
