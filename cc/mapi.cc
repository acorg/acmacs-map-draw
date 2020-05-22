#include "acmacs-base/argv.hh"
#include "acmacs-base/temp-file.hh"
#include "acmacs-base/quicklook.hh"
#include "acmacs-base/filesystem.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "acmacs-map-draw/setup-dbs.hh"
#include "acmacs-map-draw/log.hh"
#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

constexpr const std::string_view sHelpPost = R"(
-D <arg> --define <arg>

)";

using namespace acmacs::argv;
struct MapiOptions : public acmacs::argv::v2::argv
{
    MapiOptions(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) { parse(a_argc, a_argv, on_err); }
    // std::string_view help_pre() const override { return sHelpPost; }
    std::string_view help_post() const override { return sHelpPost; }

    option<str> db_dir{*this, "db-dir"};
    // option<str> seqdb{*this, "seqdb"};

    option<str_array> settings_files{*this, 's'};
    option<str_array> defines{*this, 'D', "define", desc{"see {ACMACSD_ROOT}/share/doc/mapi.org"}};
    option<str_array> apply{*this, 'a', "apply", dflt{str_array{"main"}}, desc{"name or json array to use as \"apply\", e.g. [\"/all-grey\",\"/egg\",\"/clades\",\"/labels\"]"}};
    // option<str>       apply_from{*this, "apply-from", desc{"read json array to use as \"apply\" from file (or stdin if \"-\""}};
    // option<bool>      clade{*this, "clade"};
    // option<double>    point_scale{*this, "point-scale", dflt{1.0}};
    // option<double>    rotate_degrees{*this, 'r', "rotate-degrees", dflt{0.0}, desc{"counter clockwise"}};
    // option<bool>      flip_ew{*this, "flip-ew"};
    // option<bool>      flip_ns{*this, "flip-ns"};
    // option<str>       save{*this, "save", desc{"save resulting chart with modified projection and plot spec"}};
    // option<str>       previous{*this, "previous"};
    option<size_t>    projection{*this, 'p', "projection", dflt{0ul}};

    option<bool>      open{*this, "open"};
    option<bool>      ql{*this, "ql"};
    option<str_array> verbose{*this, 'v', "verbose", desc{"comma separated list (or multiple switches) of enablers"}};

    argument<str> chart{*this, arg_name{"chart.ace"}, mandatory};
    argument<str> output{*this, arg_name{"mapi-output.pdf"}};
};

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    using namespace std::string_view_literals;
    int exit_code = 0;
    try {
        acmacs::log::register_enabler_acmacs_base();
        MapiOptions opt(argc, argv);
        acmacs::log::enable(opt.verbose);
        setup_dbs(opt.db_dir, !opt.verbose.empty());
        ChartDraw chart_draw(std::make_shared<acmacs::chart::ChartModify>(acmacs::chart::import_from_file(opt.chart)), opt.projection);

        acmacs::mapi::Settings settings{chart_draw};
        settings.load(opt.settings_files, opt.defines);
        for (const auto& to_apply : opt.apply) {
            // AD_DEBUG("to_apply \"{}\"", to_apply);
            settings.apply(to_apply);
        }

        chart_draw.calculate_viewport();
        AD_INFO("{:.2f}", chart_draw.viewport("mapi main"));
        AD_INFO("transformation: {}", chart_draw.transformation());

        if (opt.output->empty()) {
            acmacs::file::temp output{fmt::format("{}--p{}.pdf", fs::path(*opt.chart).stem(), opt.projection)};
            chart_draw.draw(output, 800, report_time::yes);
            acmacs::quicklook(output, 2);
        }
        else if (*opt.output == "/dev/null"sv || opt.output == "/"sv) { // do not generate pdf
        }
        else {
            chart_draw.draw(opt.output, 800, report_time::yes);
            acmacs::open_or_quicklook(opt.open, opt.ql, opt.output);
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
