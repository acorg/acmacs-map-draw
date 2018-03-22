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
                {"--apply", "", "json array to use as \"apply\", e.g. [\"all_grey\",\"egg\",\"clades\",\"labels\"]"},
                {"--clade", false},
                {"--point-scale", 1.0},
                {"--rotate-degrees", 0.0, "counter clockwise"},
                {"-r", 0.0, "rotate in degrees, counter clockwise"},
                {"--flip-ew", false},
                {"--flip-ns", false},
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
                {"--time", false, "report time of loading chart"},
                {"--verbose", false},
                {"-v", false},
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

    ChartDraw chart_draw(std::make_shared<acmacs::chart::ChartModify>(acmacs::chart::import_from_file(args[0], acmacs::chart::Verify::None, args["--time"] ? report_time::Yes : report_time::No)), args["--projection"]);
    if (args["--previous"])
        chart_draw.previous_chart(acmacs::chart::import_from_file(args["--previous"], acmacs::chart::Verify::None, do_report_time(args["--time"])));

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

    bool settings_loaded = false;
    auto load_settings = [&](argc_argv::strings aFilenames) {
        for (auto fn: aFilenames) {
            if (verbose)
                std::cerr << "DEBUG: reading settings from " << fn << '\n';
            settings.update(rjson::parse_file(fn, rjson::remove_comments::No));
            settings_loaded = true;
        }
    };
    load_settings(args["-s"]);
    load_settings(args["--settings"]);
      // std::cerr << "DEBUG: loaded settings\n" << settings.to_json_pp() << '\n';

    if (args["--apply"]) {
        const auto new_apply_value = rjson::parse_string(args["--apply"].str_view());
        try {
            const rjson::array& new_apply = new_apply_value;
            if (settings_loaded) {
                if (auto [present, old_apply_const] = settings.get_array_if("apply"); present) {
                    auto& old_apply = const_cast<rjson::array&>(old_apply_const);
                    for (const auto& element : new_apply)
                        old_apply.insert(element);
                }
                else
                    settings.set_field("apply", new_apply);
            }
            else {
                settings.set_field("apply", new_apply);
            }
        }
        catch (std::bad_variant_access&) {
            throw std::runtime_error{"invalid --apply argument (json array expected): " + args["--apply"].str()};
        }
    }
    else if (args["--clade"]) {
        settings.set_field("apply", rjson::array{"size_reset", "all_grey", "egg", "clades", "vaccines", "title"});
    }

    if (args["--point-scale"].present()) {
        static_cast<rjson::array&>(settings["apply"]).insert(rjson::object{{{"N", rjson::string{"point_scale"}}, {"scale", rjson::number{static_cast<double>(args["--point-scale"])}}, {"outline_scale", rjson::number{1.0}}}});
    }

    if (args["--flip-ew"].present()) {
        static_cast<rjson::array&>(settings["apply"]).insert(rjson::object{{{"N", rjson::string{"flip"}}, {"direction", rjson::string{"ew"}}}});
    }
    if (args["--flip-ns"].present()) {
        static_cast<rjson::array&>(settings["apply"]).insert(rjson::object{{{"N", rjson::string{"flip"}}, {"direction", rjson::string{"ns"}}}});
    }
    if (auto r_opt = args["-r"]; r_opt.present()) {
        static_cast<rjson::array&>(settings["apply"]).insert(rjson::object{{{"N", rjson::string{"rotate"}}, {"degrees", rjson::number{static_cast<double>(r_opt)}}}});
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

    if (args["--open"] || args.number_of_arguments() < 2)
        acmacs::quicklook(output, 2);
    return 0;

} // draw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
