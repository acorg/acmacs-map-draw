#include <iostream>
#include <fstream>
#include <string>
using namespace std::string_literals;

#include "acmacs-base/argc-argv.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/quicklook.hh"
// #include "acmacs-base/enumerate.hh"
#include "acmacs-chart-1/ace.hh"
#include "acmacs-map-draw/draw.hh"

#include "settings.hh"
#include "mod-applicator.hh"
#include "setup-dbs.hh"

// ----------------------------------------------------------------------

static int draw(const argc_argv& args);

int main(int argc, char* const argv[])
{
    int exit_code = 1;
    try {
        argc_argv args(argc, argv, {
                {"--clade", false},
                {"--settings", argc_argv::strings{}},
                {"-s", argc_argv::strings{}},
                {"--init-settings", ""},
                {"--save", ""},
                {"-h", false},
                {"--help", false},
                {"--help-mods", false},
                {"--help-select", false},
                {"--previous", ""},
                {"--open", false},
                {"--projection", 0L},
                {"--db-dir", ""},
                {"-v", false},
                {"--verbose", false},
        });
        if (args["--help-mods"])
            std::cerr << settings_help_mods();
        else if (args["--help-select"])
            std::cerr << settings_help_select();
        else if (args["-h"] || args["--help"] || args.number_of_arguments() < 1 || args.number_of_arguments() > 2)
            std::cerr << "Usage: " << args.program() << " [options] <chart.ace> [<map.pdf>]\n" << args.usage_options() << '\n';
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
    const bool verbose = args["-v"] || args["--verbose"];

    setup_dbs(args["--db-dir"], verbose);

    auto settings = settings_default();
    std::unique_ptr<Chart> chart{import_chart(args[0], verbose ? report_time::Yes : report_time::No)};
    std::unique_ptr<Chart> previous_chart;
    if (args["--previous"])
        previous_chart = std::unique_ptr<Chart>(import_chart(args["--previous"], verbose ? report_time::Yes : report_time::No));

    ChartDraw chart_draw(*chart, args["--projection"]);
    if (previous_chart)
        chart_draw.previous_chart(*previous_chart);

    chart_draw.prepare();

    if (args["--init-settings"]) {
        auto write_settings = [&settings](std::ostream& out) { out << settings.to_json_pp() << '\n'; };
        if (args["--init-settings"] == "-") {
            write_settings(std::cout);
        }
        else {
            std::ofstream out(args["--init-settings"]);
            write_settings(out);
        }
    }

    settings.update(settings_builtin_mods());

    auto load_settings = [&](argc_argv::strings aFilenames) {
        for (auto fn: aFilenames) {
            if (verbose)
                std::cerr << "DEBUG: reading settings from " << fn << '\n';
            settings.update(rjson::parse_file(fn, rjson::remove_comments::No));
        }
    };
    if (args["-s"])
        load_settings(args["-s"]);
    if (args["--settings"])
        load_settings(args["--settings"]);
      // std::cerr << "DEBUG: loaded settings\n" << settings.to_json_pp() << '\n';

    if (args["--clade"]) {
        settings.set_field("apply", rjson::array{"all_grey", "egg", "clades", "vaccines"});
    }

    try {
        Timeit ti("applying mods: ");
        apply_mods(chart_draw, settings["apply"], settings);
    }
    catch (rjson::field_not_found& err) {
        throw std::runtime_error{std::string{"No \""} + err.what() + "\" in the settings:\n\n" + settings.to_json_pp(2, rjson::json_pp_emacs_indent::no) + "\n\n"};
    }

      // auto mods = rjson::parse_string(R"(["all_grey", {"N": "clades", "seqdb_file": "/Users/eu/AD/data/seqdb.json.xz", "report": false}])");
      // auto mods = rjson::parse_string(R"([{"N": "clades", "seqdb_file": "/Users/eu/AD/data/seqdb.json.xz", "report": false}])");
    // auto mods = rjson::parse_string(R"(["all_red"])");
    // apply_mods(chart_draw, mods, settings);

    chart_draw.calculate_viewport();

    acmacs_base::TempFile temp_file(".pdf");
    const std::string output = args.number_of_arguments() > 1 ? std::string{args[1]} : static_cast<std::string>(temp_file);
    chart_draw.draw(output, 800, report_time::Yes);

    if (const std::string save_settings = args["--init-settings"]; !save_settings.empty())
        acmacs_base::write_file(save_settings, settings.to_json_pp());

    if (const std::string save = args["--save"]; !save.empty()) {
        if (save.size() > 4 && save.substr(save.size() - 4) == ".ace")
            chart_draw.export_ace(save);
        else if ((save.size() > 5 && save.substr(save.size() - 5) == ".save") || (save.size() > 8 && save.substr(save.size() - 8) == ".save.xz"))
            chart_draw.export_lispmds(save);
        else
            throw std::runtime_error("cannot detect export file format for " + save);
    }

    if (args["--open"])
        acmacs::quicklook(output, 2);
    return 0;

} // draw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
