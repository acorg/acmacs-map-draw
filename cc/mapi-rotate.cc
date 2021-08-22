#include "acmacs-base/argv.hh"
#include "acmacs-base/temp-file.hh"
#include "acmacs-base/quicklook.hh"
#include "acmacs-base/filesystem.hh"
#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/draw.hh"

// ======================================================================

using namespace acmacs::argv;
struct MapiOptions : public acmacs::argv::v2::argv
{
    MapiOptions(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) { parse(a_argc, a_argv, on_err); }

    option<bool>      flip{*this, 'f', "flip"};

    option<str_array> verbose{*this, 'v', "verbose", desc{"comma separated list (or multiple switches) of enablers"}};

    argument<str> input{*this, arg_name{"input: chart.ace, chart.save, chart.acd1"}, mandatory};
    argument<double> angle{*this, arg_name{"angle: positive - counter-clockwise"}, mandatory};
    argument<str> output{*this, arg_name{"input: chart.ace, chart.save, chart.acd1"}}; };

int main(int argc, char* const argv[])
{
    int exit_code = 0;
    try {
        MapiOptions opt(argc, argv);
        acmacs::log::enable(opt.verbose);

        ChartDraw chart_draw{opt.input, 0};
        auto& settings = chart_draw.settings();
        settings.load_from_conf({"mapi.json", "clades.mapi", "vaccines.json"});
        if (opt.flip) {
            chart_draw.flip(0, 1); // ew
            // chart_draw.flip(1, 0); // ns
        }
        if (std::abs(*opt.angle) < 4.0)
            chart_draw.rotate(*opt.angle); // radians
        else
            chart_draw.rotate(*opt.angle * std::acos(-1) / 180.0); // degrees
        settings.apply("//clades");
        // settings.apply("mapi");

        chart_draw.calculate_viewport();
        acmacs::file::temp output(fmt::format("{}.pdf", fs::path(*opt.input).stem()), false);
        chart_draw.draw(output, 800, report_time::yes);
        acmacs::open(output, 0, 0, 5);
        if (opt.output)
            chart_draw.chart(0).export_chart(opt.output);
    }
    catch (std::exception& err) {
        AD_ERROR("{}", err);
        exit_code = 2;
    }
    return exit_code;
}

// ======================================================================
