#include <iostream>

// #include "acmacs-base/argc-argv.hh"
// #include "acmacs-chart-2/factory-import.hh"
// #include "hidb-5/vaccines.hh"
// #include "acmacs-map-draw/setup-dbs.hh"

// ----------------------------------------------------------------------

// static int report_vaccines(const argc_argv& args);

int main() //int argc, char* const argv[])
{
    std::cerr << "ERROR: use hidb5-vaccines-of-chart\n";
    return 1;

    // int exit_code = 1;
    // try {
    //     argc_argv args(argc, argv, {
    //             {"--db-dir", ""},
    //             {"--time", false, "report time of loading chart"},
    //             {"--verbose", false},
    //             {"-v", false},
    //             {"-h", false},
    //             {"--help", false},
    //     });
    //     if (args["-h"] || args["--help"] || args.number_of_arguments() < 1 || args.number_of_arguments() > 2)
    //         std::cerr << "Usage: " << args.program() << " [options] <chart.ace>\n" << args.usage_options() << '\n';
    //     else
    //         exit_code = report_vaccines(args);
    // }
    // catch (std::exception& err) {
    //     std::cerr << "ERROR: " << err.what() << '\n';
    //     exit_code = 2;
    // }
    // return exit_code;
}

// ----------------------------------------------------------------------

// int report_vaccines(const argc_argv& args)
// {
//     const bool verbose = args["-v"] || args["--verbose"];
//     setup_dbs(args["--db-dir"], verbose);
//     auto chart = acmacs::chart::import_from_file(args[0], acmacs::chart::Verify::None, do_report_time(args["--time"]));

//     hidb::VaccinesOfChart vaccines(hidb::vaccines(*chart, verbose));
//     std::cout << vaccines.report(4) << '\n';

//     return 0;

// } // report_vaccines

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
