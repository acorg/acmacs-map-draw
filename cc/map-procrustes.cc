#include <iostream>
#include <fstream>
#include <string>
using namespace std::string_literals;

#include "acmacs-base/argc-argv.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/quicklook.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/settings.hh"
#include "acmacs-map-draw/mod-applicator.hh"
#include "acmacs-map-draw/setup-dbs.hh"

// ----------------------------------------------------------------------

static int draw(const argc_argv& args);

int main(int argc, char* const argv[])
{
    int exit_code = 1;
    try {
        argc_argv args(argc, argv, {
                {"--p1", 0L, "projection number of the first chart"},
                {"--p2", 0L, "projection number of the second chart"},
                {"--threshold", 0.1, "arrow threshold"},
                {"--report", false, "report common antigens/sera"},
                {"--open", false},
                {"--db-dir", ""},
                {"--time", false, "report time of loading chart"},
                {"-h", false},
                {"--help", false},
                {"--verbose", false},
                {"-v", false},
        });
        if (args["-h"] || args["--help"] || args.number_of_arguments() < 2 || args.number_of_arguments() > 3)
            std::cerr << "Usage: " << args.program() << " [options] <chart1.ace> <chart2.ace> [<pc.pdf>]\n" << args.usage_options() << '\n';
        else
            exit_code = draw(args);
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        exit_code = 2;
    }
    return exit_code;
}

// ----------------------------------------------------------------------

int draw(const argc_argv& args)
{
    // const bool verbose = args["-v"] || args["--verbose"];
    const auto report = do_report_time(args["--time"]);
    // setup_dbs(args["--db-dir"], verbose);
    const size_t p1 = args["--p1"], p2 = args["--p2"];
    const double threshold = args["--threshold"];
    const bool report_common = args["--report"];

    ChartDraw chart_draw(std::make_shared<acmacs::chart::ChartModify>(acmacs::chart::import_from_file(args[0], acmacs::chart::Verify::None, report)), p1);

    auto settings = settings_default();
    settings.set_field("apply", rjson::array{
                                    "title",
                                    rjson::object{{{"N", rjson::string{"procrustes_arrows"}},
                                                   {"chart", rjson::string{args[1]}},
                                                   {"projection", rjson::integer{p2}},
                                                   {"threshold", rjson::number{threshold}},
                                                   {"report", rjson::boolean{report_common}}}},
                                });

    try {
        apply_mods(chart_draw, settings["apply"], settings);
    }
    catch (rjson::field_not_found& err) {
        throw std::runtime_error{std::string{"No \""} + err.what() + "\" in the settings:\n\n" + settings.to_json_pp(2, rjson::json_pp_emacs_indent::no) + "\n\n"};
    }

    chart_draw.calculate_viewport();

    acmacs::file::temp temp_file(".pdf");
    const std::string output = args.number_of_arguments() > 2 ? std::string{args[2]} : static_cast<std::string>(temp_file);
    chart_draw.draw(output, 800, report_time::Yes);

    if (args["--open"] || args.number_of_arguments() < 3)
        acmacs::quicklook(output, 2);
    return 0;

} // draw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
