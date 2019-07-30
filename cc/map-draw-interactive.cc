#include <cstdio>

#include "acmacs-base/argv.hh"
#include "acmacs-base/filesystem.hh"
#include "acmacs-base/timeit.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/quicklook.hh"
#include "hidb-5/hidb.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/settings.hh"
#include "acmacs-map-draw/mod-applicator.hh"

static std::string draw(std::shared_ptr<acmacs::chart::ChartModify> chart, const std::vector<std::string_view>& settings_files, std::string_view output_pdf, bool name_after_mod);
static std::string mod_name(const rjson::value& aMods);

// ----------------------------------------------------------------------

using namespace acmacs::argv;
struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    option<str> db_dir{*this, "db-dir"};
    // option<str> seqdb{*this, "seqdb"};

    option<str_array> settings_files{*this, 's'};
    option<size_t>    projection{*this, 'p', "projection", dflt{0UL}};
    option<bool>      name_after_mod{*this, "name-after-mod"};

    argument<str> chart{*this, arg_name{"chart.ace"}, mandatory};
    argument<str> output_pdf{*this, arg_name{"output.pdf"}};
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

        std::string_view output_pdf;
        if (opt.output_pdf)
            output_pdf = opt.output_pdf;
        else if (opt.name_after_mod) {
            output_pdf = opt.chart->substr(0, opt.chart->rfind('.') + 1);
        }
        else
            throw std::runtime_error("not output_pdf provided and no --name-after-mod");

        const auto cmd = fmt::format("fswatch --latency=0.1 '{}'", string::join("' '", *opt.settings_files));
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);

        Timeit ti_chart(fmt::format("DEBUG: chart loading from {}: ", *opt.chart), report_time::yes);
        auto chart = std::make_shared<acmacs::chart::ChartModify>(acmacs::chart::import_from_file(opt.chart));
        ti_chart.report();
        [[maybe_unused]] const auto& hidb = hidb::get(chart->info()->virus_type(), report_time::yes);
        acmacs::seqdb::get();

        std::array<char, 1024> buffer;
        for (;;) {
            if (const auto output_name = draw(chart, *opt.settings_files, output_pdf, opt.name_after_mod); !output_name.empty())
                acmacs::open_or_quicklook(true, false, output_name, 0);
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

std::string draw(std::shared_ptr<acmacs::chart::ChartModify> chart, const std::vector<std::string_view>& settings_files, std::string_view output_pdf, bool name_after_mod)
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
        std::string output{output_pdf};
        if (name_after_mod)
            output += mod_name(settings["apply"]) + ".pdf";
        chart_draw.draw(output, 800, report_time::yes);
        return output;
    }
    catch (std::exception& err) {
        fmt::print(stderr, "ERROR: {}\n", err);
    }
    return std::string{};

} // draw

// ----------------------------------------------------------------------

std::string mod_name(const rjson::value& aMods)
{
    std::string name;
    rjson::for_each(aMods, [&name](const rjson::value& mod_desc) {
        if (name.empty() && static_cast<std::string_view>(mod_desc)[0] != '?' && static_cast<std::string_view>(mod_desc)[0] != '_')
            name = mod_desc;
    });
    return name;

} // mod_name

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
