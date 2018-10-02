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
                {"--apply-from", "", "read json array to use as \"apply\" from file (or stdin if \"-\""},
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
                {"-p", 0L, "projection"},
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
    size_t projection_no = 0;
    if (const auto pr1 = args["--projection"]; pr1.present())
        projection_no = pr1;
    else if (const auto pr2 = args["-p"]; pr2.present())
        projection_no = pr2;

    setup_dbs(args["--db-dir"].str(), verbose);

    ChartDraw chart_draw(std::make_shared<acmacs::chart::ChartModify>(acmacs::chart::import_from_file(args[0], acmacs::chart::Verify::None, args["--time"] ? report_time::Yes : report_time::No)),
                         projection_no);
    if (args["--previous"])
        chart_draw.previous_chart(acmacs::chart::import_from_file(args["--previous"], acmacs::chart::Verify::None, do_report_time(args["--time"])));

    auto settings = settings_default();
    if (args["--init-settings"]) {
        auto write_settings = [&settings](std::ostream& out) { out << rjson::pretty(settings) << '\n'; };
        if (args["--init-settings"] == "-") {
            write_settings(std::cout);
        }
        else {
            std::ofstream out(args["--init-settings"].str());
            write_settings(out);
        }
    }

    settings.update(settings_builtin_mods());

    bool settings_loaded = false;
    auto load_settings = [&](argc_argv::strings aFilenames) {
        for (auto fn : aFilenames) {
            if (verbose)
                std::cerr << "DEBUG: reading settings from " << fn << '\n';
            settings.update(rjson::parse_file(fn, rjson::remove_comments::no));
            settings_loaded = true;
        }
    };
    load_settings(args["-s"]);
    load_settings(args["--settings"]);
    // std::cerr << "DEBUG: loaded settings\n" << settings.to_json_pp() << '\n';

    if (args["--apply"] || args["--apply-from"]) {
        const auto new_apply = args["--apply"] ? rjson::parse_string(static_cast<std::string_view>(args["--apply"])) : rjson::parse_string(acmacs::file::ifstream(args["--apply-from"]).read());
        if (settings_loaded) {
            if (const auto& old_apply_const = settings["apply"]; !old_apply_const.is_null()) {
                auto& old_apply = const_cast<rjson::value&>(old_apply_const);
                rjson::for_each(new_apply, [&old_apply](const rjson::value& element) { old_apply.append(element); });
            }
            else
                settings["apply"] = new_apply;
        }
        else {
            settings["apply"] = new_apply;
        }
    }
    else if (args["--clade"]) {
        settings["apply"] = rjson::array{"size_reset", "all_grey", "egg", "clades", "vaccines", "title"};
    }

    if (args["--point-scale"].present()) {
        static_cast<rjson::value&>(settings["apply"])
            .append(rjson::object{{"N", "point_scale"}, {"scale", static_cast<double>(args["--point-scale"])}, {"outline_scale", 1.0}});
    }

    if (args["--flip-ew"].present()) {
        static_cast<rjson::value&>(settings["apply"]).append(rjson::object{{"N", "flip"}, {"direction", "ew"}});
    }
    if (args["--flip-ns"].present()) {
        static_cast<rjson::value&>(settings["apply"]).append(rjson::object{{"N", "flip"}, {"direction", "ns"}});
    }
    if (auto r_opt = args["-r"]; r_opt.present()) {
        static_cast<rjson::value&>(settings["apply"]).append(rjson::object{{"N", "rotate"}, {"degrees", static_cast<double>(r_opt)}});
    }
    if (args["--rotate-degrees"].present()) {
        static_cast<rjson::value&>(settings["apply"])
            .append(rjson::object{{"N", "rotate"}, {"degrees", static_cast<double>(args["--rotate-degrees"])}});
    }

    try {
        apply_mods(chart_draw, settings["apply"], settings);
    }
    catch (std::exception& err) {
        throw std::runtime_error{"Cannot apply " + rjson::to_string(settings["apply"]) + ": " + err.what() + "\n settings:\n" + rjson::pretty(settings, 2, rjson::json_pp_emacs_indent::no) + '\n'};
    }

    chart_draw.calculate_viewport();

    if (args.number_of_arguments() < 2) {
        acmacs::file::temp output(static_cast<std::string>(fs::path(args[0]).stem()) + "--p" + std::to_string(projection_no), ".pdf");
        chart_draw.draw(output, 800, report_time::Yes);
        acmacs::quicklook(output, 2);
    }
    else {
        chart_draw.draw(std::string(args[1]), 800, report_time::Yes);
        if (args["--open"])
            acmacs::quicklook(std::string(args[1]), 2);
    }

    if (const std::string save_settings(args["--init-settings"]); !save_settings.empty())
        acmacs::file::write(save_settings, rjson::pretty(settings));

    if (const std::string save(args["--save"]); !save.empty()) {
        chart_draw.save(save, args.program());
    }

    return 0;

} // draw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
