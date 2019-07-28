#include <cstdio>

#include "acmacs-base/argv.hh"
#include "acmacs-base/filesystem.hh"
#include "acmacs-base/timeit.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/quicklook.hh"
#include "hidb-5/hidb.hh"
#include "seqdb/seqdb.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/settings.hh"
#include "acmacs-map-draw/mod-applicator.hh"

static bool draw(std::shared_ptr<acmacs::chart::ChartModify> chart, const std::vector<std::string_view>& settings_files, std::string_view output_pdf);

// ----------------------------------------------------------------------

using namespace acmacs::argv;
struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    option<str> db_dir{*this, "db-dir"};
    // option<str> seqdb{*this, "seqdb"};

    option<str_array> settings_files{*this, 's'};
    option<size_t>    projection{*this, 'p', "projection", dflt{0UL}};

    argument<str> chart{*this, arg_name{"chart.ace"}, mandatory};
    argument<str> output_pdf{*this, arg_name{"output.pdf"}, mandatory};
};

int main(int argc, char* const argv[])
{
    int exit_code = 1;
    try {
        Options opt(argc, argv);

        if (std::count_if(opt.settings_files->begin(), opt.settings_files->end(), [](std::string_view fn) {
                if (!fs::exists(fn)) {
                    fmt::print(stderr, "ERROR: \"{}\" does not exist\n", fn);
                    return true;
                }
                else
                    return false;
            }) > 0)
            throw std::runtime_error("not all settings files exist");

        const auto cmd = fmt::format("fswatch --latency=0.1 '{}'", string::join("' '", *opt.settings_files));
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);

        Timeit ti_chart(fmt::format("DEBUG: chart loading from {}: ", *opt.chart), report_time::yes);
        auto chart = std::make_shared<acmacs::chart::ChartModify>(acmacs::chart::import_from_file(opt.chart));
        ti_chart.report();
        hidb::get(chart->info()->virus_type(), report_time::yes);
        seqdb::get(seqdb::ignore_errors::no, report_time::yes);

        std::array<char, 1024> buffer;
        for (;;) {
            if (draw(chart, *opt.settings_files, opt.output_pdf))
                acmacs::open_or_quicklook(true, false, opt.output_pdf, 0);
            fgets(buffer.data(), buffer.size(), pipe.get());
        }
    }
    catch (std::exception& err) {
        fmt::print(stderr, "ERROR: {}\n", err);
        exit_code = 2;
    }
    return exit_code;
}

// ----------------------------------------------------------------------

bool draw(std::shared_ptr<acmacs::chart::ChartModify> chart, const std::vector<std::string_view>& settings_files, std::string_view output_pdf)
{
    Timeit ti_chart("DEBUG: drawing: ", report_time::yes);

    try {
        ChartDraw chart_draw(chart, 0);

        auto settings = settings_default();
        settings.update(settings_builtin_mods());
        for (auto sf : settings_files)
            settings.update(rjson::parse_file(sf, rjson::remove_comments::no));

        apply_mods(chart_draw, settings["apply"], settings);
        chart_draw.calculate_viewport();
        chart_draw.draw(output_pdf, 800, report_time::yes);
        return true;
    }
    catch (std::exception& err) {
        fmt::print(stderr, "ERROR: {}\n", err);
    }
    return false;

} // draw

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
