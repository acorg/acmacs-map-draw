#include "acmacs-base/acmacsd.hh"
#include "acmacs-base/fmt.hh"
#include "acmacs-base/argv.hh"
#include "acmacs-base/filesystem.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/temp-file.hh"
#include "acmacs-base/string-split.hh"
#include "acmacs-base/quicklook.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "acmacs-map-draw/draw.hh"
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
    using namespace std::string_view_literals;
    int exit_code = 0;
    try {
        Options opt(argc, argv);

        ChartDraw chart_draw{opt.chart, 0};

        rjson::value settings{rjson::object{{"apply", rjson::array{"title"}}}};
        for (const auto& settings_file_name : {"acmacs-map-draw.json"sv}) {
            if (const auto filename = fmt::format("{}/share/conf/{}", acmacs::acmacsd_root(), settings_file_name); fs::exists(filename))
                settings.update(rjson::parse_file(filename, rjson::remove_comments::no));
            else
                fmt::print(stderr, "WARNING: cannot load \"{}\": file not found\n", filename);
        }
        for (auto fn : *opt.settings)
            settings.update(rjson::parse_file(fn, rjson::remove_comments::no));

        const auto hemi_data = acmacs::hemi::parse(acmacs::file::read(opt.hemi_data));
        for (const auto& point_data : hemi_data.hemi_points) {
            settings["apply"].append(rjson::object{
                    {"N", "arrow"},
                    {"from_antigen", rjson::object{{"index", static_cast<size_t>(point_data.point_no)}}},
                    {"to", rjson::array{static_cast<double>(point_data.pos[0]), static_cast<double>(point_data.pos[1])}},
                    {"transform", true},
                    {"width", 1.0},
                    {"color", "blue"}
                  });
        }

        try {
            apply_mods(chart_draw, settings["apply"], settings);
        }
        catch (std::exception& err) {
            // throw std::runtime_error{fmt::format("Cannot apply {}: {}\n settings:\n{}\n", settings["apply"], err, rjson::pretty(settings, rjson::emacs_indent::no))};
            throw std::runtime_error{fmt::format("{}", err)};
        }

        chart_draw.calculate_viewport();
        AD_INFO("{}\n{}", chart_draw.viewport("map-hemisphering main"), chart_draw.chart(0).modified_transformation());

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
        fmt::print(stderr, "ERROR: {}\n", err);
        exit_code = 2;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
