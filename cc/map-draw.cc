#include <iostream>
#include <fstream>
#include <string>

#include "acmacs-base/argv.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/quicklook.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/settings.hh"
#include "acmacs-map-draw/mod-applicator.hh"
#include "acmacs-map-draw/setup-dbs.hh"

// ----------------------------------------------------------------------

using namespace acmacs::argv;
struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    option<str> db_dir{*this, "db-dir"};
    // option<str> seqdb{*this, "seqdb"};

    option<str>       apply{*this, "apply", desc{"json array to use as \"apply\", e.g. [\"all_grey\",\"egg\",\"clades\",\"labels\"]"}};
    option<str>       apply_from{*this, "apply-from", desc{"read json array to use as \"apply\" from file (or stdin if \"-\""}};
    option<bool>      clade{*this, "clade"};
    option<double>    point_scale{*this, "point-scale", dflt{1.0}};
    option<double>    rotate_degrees{*this, 'r', "rotate-degrees", dflt{0.0}, desc{"counter clockwise"}};
    option<bool>      flip_ew{*this, "flip-ew"};
    option<bool>      flip_ns{*this, "flip-ns"};
    option<str_array> settings_files{*this, 's'};
    option<str>       init_settings{*this, 'i', "init-settings"};
    option<str>       save{*this, "save", desc{"save resulting chart with modified projection and plot spec"}};
    option<bool>      help_mods{*this, "help-mods"};
    option<bool>      help_select{*this, "help-select"};
    option<str>       previous{*this, "previous"};
    option<size_t>    projection{*this, 'p', "projection", dflt{0UL}};
    option<bool>      open{*this, "open"};
    option<bool>      ql{*this, "ql"};
    option<bool>      verbose{*this, 'v', "verbose"};

    argument<str> chart{*this, arg_name{"chart.ace"}, mandatory};
    argument<str> output_pdf{*this, arg_name{"output.pdf"}};
};

static int draw(const Options& opt);

int main(int argc, char* const argv[])
{
    int exit_code = 1;
    try {
        Options opt(argc, argv);
        // argc_argv args(argc, argv, {
        //         {"--apply", "", "json array to use as \"apply\", e.g. [\"all_grey\",\"egg\",\"clades\",\"labels\"]"},
        //         {"--apply-from", "", "read json array to use as \"apply\" from file (or stdin if \"-\""},
        //         {"--clade", false},
        //         {"--point-scale", 1.0},
        //         {"--rotate-degrees", 0.0, "counter clockwise"},
        //         {"-r", 0.0, "rotate in degrees, counter clockwise"},
        //         {"--flip-ew", false},
        //         {"--flip-ns", false},
        //         {"--settings", argc_argv::strings{}, "load settings from file"},
        //         {"-s", argc_argv::strings{}, "load settings from file"},
        //         {"--init-settings", "", "initialize (overwrite) settings file"},
        //         {"--save", "", "save resulting chart with modified projection and plot spec"},
        //         {"-h", false},
        //         {"--help", false},
        //         {"--help-mods", false},
        //         {"--help-select", false},
        //         {"--previous", ""},
        //         {"--open", false},
        //         {"--ql", false},
        //         {"--projection", 0},
        //         {"-p", 0, "projection"},
        //         {"--db-dir", ""},
        //         {"--time", false, "report time of loading chart"},
        //         {"--verbose", false},
        //         {"-v", false},
        // });
        if (opt.help_mods)
            std::cerr << settings_help_mods();
        else if (opt.help_select)
            std::cerr << settings_help_select();
        else
            exit_code = draw(opt);
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        exit_code = 2;
    }
    return exit_code;
}

// ----------------------------------------------------------------------

int draw(const Options& opt)
{
    setup_dbs(opt.db_dir, opt.verbose);

    ChartDraw chart_draw(std::make_shared<acmacs::chart::ChartModify>(acmacs::chart::import_from_file(opt.chart)), opt.projection);
    if (!opt.previous->empty())
        chart_draw.previous_chart(acmacs::chart::import_from_file(opt.previous));

    auto settings = settings_default();
    if (!opt.init_settings->empty()) {
        auto write_settings = [&settings](std::ostream& out) { out << rjson::pretty(settings) << '\n'; };
        if (*opt.init_settings == "-") {
            write_settings(std::cout);
        }
        else {
            std::ofstream out(*opt.init_settings);
            write_settings(out);
        }
    }

    settings.update(settings_builtin_mods());

    bool settings_loaded = false;
    for (auto fn : *opt.settings_files) {
        if (opt.verbose)
            std::cerr << "DEBUG: reading settings from " << fn << '\n';
        try {
            settings.update(rjson::parse_file(fn, rjson::remove_comments::no));
        }
        catch (std::exception& err) {
            throw std::runtime_error(string::concat(fn, ':', err.what()));
        }
        settings_loaded = true;
    }
    // std::cerr << "DEBUG: loaded settings\n" << settings.to_json_pp() << '\n';

    if (!opt.apply->empty() || !opt.apply_from->empty()) {
        const auto new_apply = !opt.apply->empty() ? rjson::parse_string(opt.apply) : rjson::parse_string(acmacs::file::ifstream(opt.apply_from).read());
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
    else if (opt.clade) {
        settings["apply"] = rjson::array{"size_reset", "all_grey", "egg", "clades", "vaccines", "title"};
    }

    if (opt.point_scale.has_value()) {
        static_cast<rjson::value&>(settings["apply"]).append(rjson::object{{"N", "point_scale"}, {"scale", static_cast<double>(opt.point_scale)}, {"outline_scale", 1.0}});
    }

    if (opt.flip_ew) {
        static_cast<rjson::value&>(settings["apply"]).append(rjson::object{{"N", "flip"}, {"direction", "ew"}});
    }
    if (opt.flip_ns) {
        static_cast<rjson::value&>(settings["apply"]).append(rjson::object{{"N", "flip"}, {"direction", "ns"}});
    }
    if (opt.rotate_degrees.has_value()) {
        static_cast<rjson::value&>(settings["apply"]).append(rjson::object{{"N", "rotate"}, {"degrees", static_cast<double>(opt.rotate_degrees)}});
    }

    try {
        apply_mods(chart_draw, settings["apply"], settings);
    }
    catch (std::exception& err) {
        // throw std::runtime_error{"Cannot apply " + rjson::to_string(settings["apply"]) + ": " + err.what() + "\n settings:\n" + rjson::pretty(settings, rjson::emacs_indent::no) + '\n'};
        throw std::runtime_error{"Cannot apply " + rjson::to_string(settings["apply"]).substr(0, 200) + ": " + err.what()};
    }

    chart_draw.calculate_viewport();

    if (opt.output_pdf->empty()) {
        acmacs::file::temp output(static_cast<std::string>(fs::path(*opt.chart).stem()) + "--p" + std::to_string(opt.projection), ".pdf");
        chart_draw.draw(output, 800, report_time::yes);
        acmacs::quicklook(output, 2);
    }
    else if (*opt.output_pdf == "/dev/null" || opt.output_pdf == "/") { // do not generate pdf
    }
    else {
        chart_draw.draw(opt.output_pdf, 800, report_time::yes);
        if (opt.open)
            acmacs::open(opt.output_pdf, 2);
        else if (opt.ql)
            acmacs::quicklook(opt.output_pdf, 2);
    }

    if (!opt.init_settings->empty())
        acmacs::file::write(opt.init_settings, rjson::pretty(settings));

    if (!opt.save->empty())
        chart_draw.save(opt.save, opt.program_name());

    return 0;

} // draw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
