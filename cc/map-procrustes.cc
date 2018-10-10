#include <iostream>
#include <fstream>
#include <string>
using namespace std::string_literals;

#include "acmacs-base/argc-argv.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/string-split.hh"
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
                {"--subset", "all", "all, antigens, sera"},
                {"--report", false, "report common antigens/sera"},
                {"--clade", false},
                {"--viewport", "", "\"rel\" array of viewport, e.g. 2,2,-4"},
                {"--open", false},
                {"--db-dir", ""},
                {"--time", false, "report time of loading chart"},
                {"-h", false},
                {"--help", false},
                {"--verbose", false},
                {"-v", false},
        });
        if (args["-h"] || args["--help"] || args.number_of_arguments() < 1 || args.number_of_arguments() > 3)
            std::cerr << "Usage: " << args.program() << " [options] <chart1.ace> [<chart2.ace>] [<pc.pdf>]\n" << args.usage_options() << '\n';
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

    std::string secondary_chart((args.number_of_arguments() > 1 && !string::ends_with(std::string_view(args[1]), ".pdf")) ? args[1] : args[0]);
    const rjson::value pc{rjson::object{{"N", "procrustes_arrows"},
                                         {"chart", secondary_chart},
                                         {"projection", p2},
                                         {"subset", static_cast<std::string_view>(args["--subset"])},
                                         {"threshold", threshold},
                                         {"report", report_common}}};

    rjson::value viewport{rjson::object{{"N", "viewport"}, {"rel", rjson::array{0, 0, 0}}}};
    if (args["--viewport"]) {
        const auto values = acmacs::string::split_into_double(args["--viewport"].str());
        viewport["rel"] = rjson::array(values.begin(), values.end());
    }

    auto settings = settings_default();
    settings.update(settings_builtin_mods());
    if (args["--clade"])
        settings["apply"] = rjson::array{viewport, "size_reset", "all_grey", "egg", "clades", "vaccines", "title", pc};
    else
        settings["apply"] = rjson::array{viewport, "title", pc};

    try {
        apply_mods(chart_draw, settings["apply"], settings);
    }
    catch (std::exception& err) {
        throw std::runtime_error{"Cannot apply " + rjson::to_string(settings["apply"]) + ": " + err.what() + "\n settings:\n" + rjson::pretty(settings, 2, rjson::json_pp_emacs_indent::no) + '\n'};
    }

    chart_draw.calculate_viewport();

    std::string output_file;
    if (args.number_of_arguments() > 2)
        output_file = args[2];
    else if (args.number_of_arguments() > 1 && string::ends_with(std::string_view(args[1]), ".pdf"))
        output_file = args[1];
    acmacs::file::temp temp_file(".pdf");
    const std::string output = output_file.empty() ? static_cast<std::string>(temp_file) : output_file;
    chart_draw.draw(output, 800, report_time::Yes);

    if (args["--open"] || output_file.empty())
        acmacs::quicklook(output, 2);
    return 0;

} // draw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
