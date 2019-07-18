#include <iostream>
#include <fstream>
#include <string>

#include "acmacs-base/fmt.hh"
#include "acmacs-base/argv.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/string-split.hh"
#include "acmacs-base/quicklook.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/settings.hh"
#include "acmacs-map-draw/mod-applicator.hh"
#include "acmacs-map-draw/hemisphering-data.hh"

// ----------------------------------------------------------------------

using namespace acmacs::argv;
struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    option<str_array> settings{*this, 's', desc{"settings.json (multiple -s possible)"}};
    option<bool>      open{*this, "open"};
    option<bool>      ql{*this, "ql"};

    argument<str>     chart{*this, arg_name{"chart"}, mandatory};
    argument<str>     hemi_data{*this, arg_name{"hemi-data.json"}, mandatory};
    argument<str>     pdf{*this, arg_name{"output-pdf"}};
};

int main(int argc, char* const argv[])
{
    int exit_code = 0;
    try {
        Options opt(argc, argv);

        ChartDraw chart_draw(std::make_shared<acmacs::chart::ChartModify>(acmacs::chart::import_from_file(opt.chart)), 0);

        auto settings = settings_default();
        settings.update(settings_builtin_mods());
        for (size_t sett_no = 0; sett_no < opt.settings->size(); ++sett_no) {
            settings.update(rjson::parse_file(opt.settings->at(sett_no), rjson::remove_comments::no));
        }

        const auto hemi_data = acmacs::hemi::parse(acmacs::file::read(opt.hemi_data));
        for (const auto& point_data : hemi_data.hemi_points) {
            settings["apply"].append(rjson::object{
                    {"N", "arrow"},
                    {"from_antigen", rjson::object{{"index", static_cast<size_t>(point_data.point_no)}}},
                    {"to", rjson::array{static_cast<double>(point_data.pos[0]), static_cast<double>(point_data.pos[1])}},
                    {"width", 1.0},
                    {"color", "blue"}
                  });
        }

        try {
            apply_mods(chart_draw, settings["apply"], settings);
        }
        catch (std::exception& err) {
            // throw std::runtime_error{"Cannot apply " + rjson::to_string(settings["apply"]) + ": " + err.what() + "\n settings:\n" + rjson::pretty(settings, rjson::emacs_indent::no) + '\n'};
            throw std::runtime_error{fmt::format("{}", err)};
        }

        chart_draw.calculate_viewport();

        if (opt.pdf) {
            chart_draw.draw(opt.pdf, 800, report_time::yes);
            acmacs::open_or_quicklook(opt.open, opt.ql, opt.pdf, 2);
        }
        else {
            acmacs::file::temp temp_file(".pdf");
            chart_draw.draw(temp_file, 800, report_time::yes);
            acmacs::open_or_quicklook(opt.open, true, temp_file, 2);
        }
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        exit_code = 2;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
