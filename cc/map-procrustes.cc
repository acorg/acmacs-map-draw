#include <iostream>
#include <fstream>
#include <string>

#include "acmacs-base/argv.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/string-split.hh"
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

    option<size_t>    p1{*this, "p1", dflt{0UL}, desc{"projection number of the first chart"}};
    option<size_t>    p2{*this, "p2", dflt{0UL}, desc{"projection number of the second chart"}};
    option<str_array> settings{*this, 's'};
    option<double>    threshold{*this, "threshold", dflt{0.1}, desc{"arrow threshold"}};
    option<str>       subset{*this, "subset", dflt{"all"}, desc{"all, antigens, sera"}};
    option<bool>      report{*this, "report", desc{"report common antigens/sera"}};
    option<bool>      clade{*this, "clade", desc{"color by clade"}};
    option<str>       viewport{*this, "viewport", dflt{""}, desc{"\"rel\" array of viewport, e.g. 2,2,-4"}};
    option<bool>      open{*this, "open"};

    argument<str>     chart1{*this, arg_name{"chart"}, mandatory};
    argument<str>     chart2{*this, arg_name{"secondary-chart"}};
    argument<str>     pdf{*this, arg_name{"output-pdf"}};
};

int main(int argc, char* const argv[])
{
    int exit_code = 0;
    try {
        Options opt(argc, argv);
        ChartDraw chart_draw(std::make_shared<acmacs::chart::ChartModify>(acmacs::chart::import_from_file(opt.chart1, acmacs::chart::Verify::None, report_time::no)), opt.p1);
        const std::string secondary_chart((!opt.chart2->empty() && !string::ends_with(*opt.chart2, ".pdf")) ? opt.chart2 : opt.chart1);

        auto settings = settings_default();
        settings.update(settings_builtin_mods());
        for (size_t sett_no = 0; sett_no < opt.settings->size(); ++sett_no) {
            settings.update(rjson::parse_file(opt.settings->at(sett_no), rjson::remove_comments::no));
        }

        if (!opt.viewport->empty()) {
            const auto values = acmacs::string::split_into_double(*opt.viewport);
            settings["apply"].append(rjson::object{{"N", "viewport"}, {"rel", rjson::array(values.begin(), values.end())}});
        }
        if (opt.clade) {
            settings["apply"].append("size_reset");
            settings["apply"].append("all_grey");
            settings["apply"].append("egg");
            settings["apply"].append("clades");
            settings["apply"].append("vaccines");
        }

        settings["apply"].append(
            rjson::object{{"N", "procrustes_arrows"}, {"chart", secondary_chart}, {"projection", *opt.p2}, {"subset", *opt.subset}, {"threshold", *opt.threshold}, {"report", *opt.report}});

        try {
            apply_mods(chart_draw, settings["apply"], settings);
        }
        catch (std::exception& err) {
            throw std::runtime_error{"Cannot apply " + rjson::to_string(settings["apply"]) + ": " + err.what() + "\n settings:\n" + rjson::pretty(settings, rjson::emacs_indent::no) + '\n'};
        }

        chart_draw.calculate_viewport();

        const std::string output_file{string::ends_with(*opt.chart2, ".pdf") ? *opt.chart2 : *opt.pdf};
        acmacs::file::temp temp_file(".pdf");
        const std::string output = output_file.empty() ? static_cast<std::string>(temp_file) : output_file;
        chart_draw.draw(output, 800, report_time::yes);

        if (opt.open || output_file.empty())
            acmacs::quicklook(output, 2);
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
