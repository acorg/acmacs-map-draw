#include <iostream>
#include <fstream>
#include <string>
using namespace std::string_literals;

#include "acmacs-base/argc-argv.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/quicklook.hh"
#include "acmacs-chart-2/factory-import.hh"
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
                {"--apply", "", "json array to use as \"apply\", e.g. [\"all_grey\",\"egg\",\"clades\"]"},
                {"--clade", false},
                {"--point-scale", 1.0},
                {"--rotate-degrees", 0.0, "counter clockwise"},
                {"--settings", argc_argv::strings{}, "load settings from file"},
                {"-s", argc_argv::strings{}, "load settings from file"},
                {"--init-settings", "", "initialize (overwrite) settings file"},
                {"--save", "", "save resulting chart with modified projection and plot spec"},
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

    ChartDraw chart_draw(std::make_shared<acmacs::chart::ChartModify>(acmacs::chart::import_factory(args[0], acmacs::chart::Verify::None)), args["--projection"]);
    if (args["--previous"])
        chart_draw.previous_chart(acmacs::chart::import_factory(args["--previous"], acmacs::chart::Verify::None));

    auto settings = settings_default();
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

    if (args["--apply"]) {
        settings.set_field("apply", rjson::parse_string(args["--apply"].str_view()));
    }
    else if (args["--clade"]) {
        settings.set_field("apply", rjson::array{"all_grey", "egg", "clades", "vaccines"});
    }

    if (args["--point-scale"].present()) {
        static_cast<rjson::array&>(settings["apply"]).insert(rjson::object{{{"N", rjson::string{"point_scale"}}, {"scale", rjson::number{static_cast<double>(args["--point-scale"])}}, {"outline_scale", rjson::number{1.0}}}});
    }

    if (args["--rotate-degrees"].present()) {
        static_cast<rjson::array&>(settings["apply"]).insert(rjson::object{{{"N", rjson::string{"rotate"}}, {"degrees", rjson::number{static_cast<double>(args["--rotate-degrees"])}}}});
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

    acmacs::file::temp temp_file(".pdf");
    const std::string output = args.number_of_arguments() > 1 ? std::string{args[1]} : static_cast<std::string>(temp_file);
    chart_draw.draw(output, 800, report_time::Yes);

    if (const std::string save_settings = args["--init-settings"]; !save_settings.empty())
        acmacs::file::write(save_settings, settings.to_json_pp());

    if (const std::string save = args["--save"]; !save.empty()) {
        chart_draw.save(save, args.program());
    }

    if (args["--open"])
        acmacs::quicklook(output, 2);
    return 0;

} // draw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
