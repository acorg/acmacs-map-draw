#include "acmacs-base/acmacsd.hh"
#include "acmacs-base/fmt.hh"
#include "acmacs-base/argv.hh"
#include "acmacs-base/filesystem.hh"
#include "acmacs-base/temp-file.hh"
#include "acmacs-base/string-split.hh"
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

    option<size_t>    p1{*this, "p1", dflt{0UL}, desc{"projection number of the first chart"}};
    option<size_t>    p2{*this, "p2", dflt{0UL}, desc{"projection number of the second chart"}};
    option<str_array> settings{*this, 's', desc{"settings.json (multiple -s possible)"}};
    option<double>    threshold{*this, "threshold", dflt{0.1}, desc{"arrow threshold"}};
    option<str>       subset{*this, "subset", dflt{"all"}, desc{"all, antigens, sera"}};
    option<str>       subset_antigens_clade{*this, "subset-antigens-clade", desc{"for antigens from this clade only"}};
    option<bool>      report{*this, "report", desc{"report common antigens/sera"}};
    option<bool>      clade{*this, "clade", desc{"color by clade"}};
    option<str>       viewport{*this, "viewport", dflt{""}, desc{"\"rel\" array of viewport, e.g. 2,2,-4"}};
    option<bool>      ql{*this, "ql"};
    option<bool>      open{*this, "open"};
    option<bool>      verbose{*this, 'v', "verbose"};

    argument<str>     chart1{*this, arg_name{"chart"}, mandatory};
    argument<str>     chart2{*this, arg_name{"secondary-chart"}};
    argument<str>     pdf{*this, arg_name{"output-pdf"}};
};

int main(int argc, char* const argv[])
{
    using namespace std::string_view_literals;
    int exit_code = 0;
    try {
        Options opt(argc, argv);
        ChartDraw chart_draw(std::make_shared<acmacs::chart::ChartModify>(acmacs::chart::import_from_file(opt.chart1, acmacs::chart::Verify::None, report_time::no)), opt.p1);
        const std::string secondary_chart((!opt.chart2->empty() && !acmacs::string::endswith(*opt.chart2, ".pdf"sv)) ? opt.chart2 : opt.chart1);

        rjson::value settings{rjson::object{{"apply", rjson::array{"title"}}}};
        for (const auto& settings_file_name : {"acmacs-map-draw.json"sv}) {
            if (const auto filename = fmt::format("{}/share/conf/{}", acmacs::acmacsd_root(), settings_file_name); fs::exists(filename))
                settings.update(rjson::parse_file(filename, rjson::remove_comments::no));
            else
                fmt::print(stderr, "WARNING: cannot load \"{}\": file not found\n", filename);
        }
        for (auto fn : *opt.settings)
            settings.update(rjson::parse_file(fn, rjson::remove_comments::no));

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

        if (opt.subset_antigens_clade.has_value())
            settings["apply"].append(rjson::object{{"N", "procrustes_arrows"},
                                                   {"chart", secondary_chart},
                                                   {"projection", *opt.p2},
                                                   {"subset_antigens", rjson::object{{"clade", *opt.subset_antigens_clade}}},
                                                   {"threshold", *opt.threshold},
                                                   {"report", *opt.report}});
        else
            settings["apply"].append(
                rjson::object{{"N", "procrustes_arrows"}, {"chart", secondary_chart}, {"projection", *opt.p2}, {"subset", *opt.subset}, {"threshold", *opt.threshold}, {"report", *opt.report}});

        try {
            apply_mods(chart_draw, settings["apply"], settings, acmacs::verbose_from(opt.verbose));
        }
        catch (std::exception& err) {
            // throw std::runtime_error{fmt::format("Cannot apply {}: {}\n settings:\n{}\n", settings["apply"], err, rjson::pretty(settings, rjson::emacs_indent::no))};
            throw std::runtime_error{fmt::format("{}", err)};
        }

        chart_draw.calculate_viewport();

        const std::string output_file{!opt.chart2->empty() && acmacs::string::endswith(*opt.chart2, ".pdf"sv) ? *opt.chart2 : *opt.pdf};
        acmacs::file::temp temp_file(".pdf");
        const std::string output = output_file.empty() ? static_cast<std::string>(temp_file) : output_file;
        chart_draw.draw(output, 800, report_time::yes);

        if (opt.open || opt.ql || output_file.empty())
            acmacs::open_or_quicklook(opt.open || (output_file.empty() && !opt.ql), opt.ql, output);
    }
    catch (std::exception& err) {
        fmt::print(stderr, "ERROR {}\n", err);
        exit_code = 2;
    }
    return exit_code;
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
