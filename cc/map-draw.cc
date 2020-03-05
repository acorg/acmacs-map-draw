#include "acmacs-base/acmacsd.hh"
#include "acmacs-base/argv.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/quicklook.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "acmacs-map-draw/draw.hh"
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
    option<str>       save{*this, "save", desc{"save resulting chart with modified projection and plot spec"}};
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
        exit_code = draw(opt);
    }
    catch (std::exception& err) {
        fmt::print(stderr, "ERROR: {}\n", err);
        exit_code = 2;
    }
    return exit_code;
}

// ----------------------------------------------------------------------

int draw(const Options& opt)
{
    using namespace std::string_view_literals;

    setup_dbs(opt.db_dir, opt.verbose);

    ChartDraw chart_draw(std::make_shared<acmacs::chart::ChartModify>(acmacs::chart::import_from_file(opt.chart)), opt.projection);
    if (!opt.previous->empty())
        chart_draw.previous_chart(acmacs::chart::import_from_file(opt.previous));

    rjson::value settings{rjson::object{{"apply", rjson::array{"title"}}}};
    for (const auto& settings_file_name : {"acmacs-map-draw.json"sv}) {
        if (const auto filename = fmt::format("{}/share/conf/{}", acmacs::acmacsd_root(), settings_file_name); fs::exists(filename))
            settings.update(rjson::parse_file(filename, rjson::remove_comments::no));
        else
            fmt::print(stderr, "WARNING: cannot load \"{}\": file not found\n", filename);
    }

    bool settings_loaded = false;
    for (auto fn : *opt.settings_files) {
        if (opt.verbose)
            fmt::print(stderr, "DEBUG: reading settings from {}\n", fn);
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
        if (opt.verbose)
            fmt::print(stderr, "DEBUG: new apply {}\n", new_apply);
        if (settings_loaded) {
            // if (const auto& old_apply_const = settings["apply"]; !old_apply_const.is_null()) {
            //     auto& old_apply = const_cast<rjson::value&>(old_apply_const);
            //     if (opt.verbose)
            //         std::cerr << "DEBUG: old apply " << old_apply << '\n';
            //     rjson::for_each(new_apply, [&old_apply](const rjson::value& element) { old_apply.append(element); });
            // }
            // else
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
        const auto msg = rjson::to_string(settings["apply"]).substr(0, 200);
        const auto add_double_quotes = (std::count(std::begin(msg), std::end(msg), '"') % 2) == 1 ? "\"" : "";
        throw std::runtime_error{fmt::format(": {}   cannot apply: {} ... {}", err.what(), msg, add_double_quotes)};
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
        acmacs::open_or_quicklook(opt.open, opt.ql, opt.output_pdf, 2);
    }

    if (!opt.save->empty())
        chart_draw.save(opt.save, opt.program_name());

    return 0;

} // draw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
