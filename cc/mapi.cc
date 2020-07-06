#include <unistd.h>
#include <signal.h>

#include "acmacs-base/argv.hh"
#include "acmacs-base/temp-file.hh"
#include "acmacs-base/string-compare.hh"
#include "acmacs-base/string-split.hh"
#include "acmacs-base/quicklook.hh"
#include "acmacs-base/filesystem.hh"
#include "acmacs-map-draw/setup-dbs.hh"
#include "acmacs-map-draw/log.hh"
#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

constexpr const std::string_view sHelpPost = R"(
one input chart: make map
two input charts: make procrustes
output /: do not generate any output
no output: generate temp pdf and open it (and then delete it)

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
    option<str_array> apply{*this, 'a', "apply", desc{"comma separated names or json array to use as \"apply\", e.g. [\"/all-grey\",\"/egg\",\"/clades\",\"/labels\"]"}};
    option<bool>      interactive{*this, 'i', "interactive"};
    // option<str>       previous{*this, "previous"};
    option<size_t>    projection{*this, 'p', "projection", dflt{0ul}};
    option<size_t>    secondary_projection{*this, 'r', "secondary-projection", dflt{static_cast<size_t>(-1)}};

    option<bool>      open{*this, "open"};
    option<str>       preview{*this, "preview"};
    // option<bool>      ql{*this, "ql"};
    option<str_array> verbose{*this, 'v', "verbose", desc{"comma separated list (or multiple switches) of enablers"}};

    argument<str_array> files{*this, arg_name{"input: chart.ace, chart.save, chart.acd1; output: map.pdf, /"}, mandatory};
};

    // option<str>       apply_from{*this, "apply-from", desc{"read json array to use as \"apply\" from file (or stdin if \"-\""}};
    // option<bool>      clade{*this, "clade"};
    // option<double>    point_scale{*this, "point-scale", dflt{1.0}};
    // option<double>    rotate_degrees{*this, 'r', "rotate-degrees", dflt{0.0}, desc{"counter clockwise"}};
    // option<bool>      flip_ew{*this, "flip-ew"};
    // option<bool>      flip_ns{*this, "flip-ns"};
    // option<str>       save{*this, "save", desc{"save resulting chart with modified projection and plot spec"}};

// ----------------------------------------------------------------------

static std::pair<std::vector<std::string_view>, std::vector<std::string_view>> parse_files(const argument<str_array>& files);
static void signal_handler(int sig_num);

int main(int argc, char* const argv[])
{
    using namespace std::string_view_literals;
    int exit_code = 0;
    try {
        acmacs::log::register_enabler_acmacs_base();
        MapiOptions opt(argc, argv);
        acmacs::log::enable(opt.verbose);
        setup_dbs(opt.db_dir, !opt.verbose.empty());

        const auto [inputs, outputs] = parse_files(opt.files);

        ChartDraw chart_draw{inputs, opt.projection};

        acmacs::mapi::Settings settings{chart_draw};
        settings.load_from_conf({"mapi.json"sv, "clades.json"sv, "vaccines.json"sv});
        settings.load(opt.settings_files);
        settings.set_defines(opt.defines);
        for (size_t chart_no = 0; chart_no < chart_draw.number_of_charts(); ++chart_no)
            settings.setenv_toplevel(fmt::format("chart[{}]", chart_no), chart_draw.chart(chart_no).filename());

        if (opt.interactive)
            signal(SIGHUP, signal_handler);

        bool opened{false};
        for (;;) {
            exit_code = 0;
            try {
                if (!opt.apply.empty()) {
                    for (const auto& to_apply : opt.apply) {
                        if (!to_apply.empty()) {
                            if (to_apply[0] == '{' || to_apply[0] == '[') {
                                settings.apply(to_apply);
                            }
                            else {
                                for (const auto& to_apply_one : acmacs::string::split(to_apply))
                                    settings.apply(to_apply_one);
                            }
                        }
                    }
                }
                else {
                    settings.apply("mapi"sv);
                }

                chart_draw.calculate_viewport();
                AD_INFO("{:.2f}", chart_draw.viewport("mapi main"));
                AD_INFO("transformation: {}", chart_draw.chart(0).modified_transformation());

                if (outputs.empty()) {
                    acmacs::file::temp output{fmt::format("{}--p{}.pdf", fs::path(inputs[0]).stem(), opt.projection)};
                    chart_draw.draw(output, 800, report_time::yes);
                    acmacs::quicklook(output, 2);
                }
                else if (outputs[0] == "/dev/null"sv || outputs[0] == "/"sv) { // do not generate pdf
                }
                else {
                    chart_draw.draw(outputs[0], 800, report_time::yes);
                    if ((opt.open || opt.interactive) && (!opt.preview || !opened))
                        acmacs::open(outputs[0]);
                    else if (opt.preview) {
                        acmacs::preview(outputs[0], opt.preview);
                        opened = true;
                    }
                }
            }
            catch (std::exception& err) {
                AD_ERROR("{}", err);
                acmacs::run_and_detach({"submarine"}, 0);
                exit_code = 1;
            }
            if (opt.interactive) {
                fmt::print(stderr, "mapi-i >> ");

                fd_set set;
                FD_ZERO(&set);
                FD_SET(0, &set);
                if (select(FD_SETSIZE, &set, nullptr, nullptr, nullptr) > 0) {
                    // fmt::print(stderr, " (reading)\n");
                    std::string input(20, ' ');
                    if (const auto bytes = ::read(0, input.data(), input.size()); bytes > 0) {
                        input.resize(static_cast<size_t>(bytes));
                        // fmt::print(stderr, " (read {} bytes)\n", bytes);
                    }
                }

                acmacs::run_and_detach({"tink"}, 0);
            }
            else
                break;

            try {
                chart_draw.reset();
                settings.reload();
            }
            catch (std::exception& err) {
                AD_ERROR("{}", err);
                acmacs::run_and_detach({"submarine"}, 0);
            }
        }
    }
    catch (std::exception& err) {
        fmt::print(stderr, "ERROR: {}\n", err);
        exit_code = 2;
    }
    return exit_code;
}

// ----------------------------------------------------------------------

std::pair<std::vector<std::string_view>, std::vector<std::string_view>> parse_files(const argument<str_array>& files)
{
    using namespace std::string_view_literals;
    std::vector<std::string_view> inputs, outputs;
    constexpr std::array input_suffixes{".ace"sv, ".acd1"sv, ".acd1.xz"sv, ".acd1.bz2"sv, ".save"sv, ".save.xz"sv, ".save.bz2"sv};
    const auto is_input = [&input_suffixes](std::string_view name) -> bool {
        return std::any_of(std::begin(input_suffixes), std::end(input_suffixes), [name](std::string_view suff) -> bool { return acmacs::string::endswith(name, suff); });
    };
    for (const auto& file : files) {
        if (is_input(file))
            inputs.push_back(file);
        else
            outputs.push_back(file);
    }
    if (inputs.empty())
        throw std::runtime_error{"no input files (charts) found in the command line"};
    if (inputs.size() > 2)
        throw std::runtime_error{fmt::format("too many input files () found in the command line", inputs)};
    return {inputs, outputs};

} // parse_files

// ----------------------------------------------------------------------

void signal_handler(int sig_num)
{
    fmt::print("SIGNAL {}\n", sig_num);

} // signal_handler

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
