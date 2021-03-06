#include <cstdio>

#include "acmacs-base/acmacsd.hh"
#include "acmacs-base/argv.hh"
#include "acmacs-base/filesystem.hh"
#include "acmacs-base/timeit.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/quicklook.hh"
#include "hidb-5/hidb.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/mod-applicator.hh"
#include "acmacs-map-draw/setup-dbs.hh"

static std::string draw(ChartDraw& chart_draw, const std::vector<std::string_view>& settings_files, std::string_view output_pdf, bool name_after_mod);
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
    option<str>       preview_pos{*this, "pos"};
    option<str>       previous{*this, "previous"};
    option<str>       previous_pos{*this, "previous-pos"};

    argument<str> chart{*this, arg_name{"chart.ace"}, mandatory};
    argument<str> output_pdf{*this, arg_name{"mdi-output.pdf"}};
};

int main(int argc, char* const argv[])
{
    int exit_code = 1;
    try {
        Options opt(argc, argv);
        setup_dbs(opt.db_dir, false);

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

        const auto cmd = fmt::format("fswatch --latency=0.1 '{}'", acmacs::string::join(acmacs::string::join_sep_t{"' '"}, *opt.settings_files));
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);

        // auto chart = std::make_shared<acmacs::chart::ChartModify>(acmacs::chart::import_from_file(opt.chart));
        ChartDraw chart_draw{opt.chart, 0};
        [[maybe_unused]] const auto& hidb = hidb::get(chart_draw.chart().info()->virus_type(), report_time::yes);
        acmacs::seqdb::get();

        if (opt.previous) {
            if (opt.previous_pos)
                acmacs::preview(opt.previous, *opt.previous_pos);
            else
                acmacs::open(opt.previous);
        }

        std::array<char, 1024> buffer;
        for (;;) {
            if (const auto output_name = draw(chart_draw, *opt.settings_files, output_pdf, opt.name_after_mod); !output_name.empty()) {
                acmacs::run_and_detach("tink");
                if (opt.preview_pos)
                    acmacs::preview(output_name, *opt.preview_pos);
                else
                    acmacs::open(output_name.data());
                // for (auto sf = opt.settings_files->rbegin(); sf != opt.settings_files->rend(); ++sf) {
                //     std::string name{*sf};
                //     name.push_back('\0');
                //     acmacs::run_and_detach({"emacsclient", "-n", name.data()}, 0);
                // }
            }
            else
                acmacs::run_and_detach("submarine");

            fmt::print("\nSETTINGS:\n");
            for (const auto& sf : *opt.settings_files)
                fmt::print("    {}\n", sf);

            if (!fgets(buffer.data(), buffer.size(), pipe.get()))
                throw std::runtime_error{"fgets error"};
            acmacs::run_and_detach("tink");
            chart_draw.reset();
        }
    }
    catch (std::exception& err) {
        fmt::print(stderr, "ERROR: {}\n", err);
        exit_code = 2;
    }
    return exit_code;
}

// ----------------------------------------------------------------------

std::string draw(ChartDraw& chart_draw, const std::vector<std::string_view>& settings_files, std::string_view output_pdf, bool name_after_mod)
{
    using namespace std::string_view_literals;
    Timeit ti_chart("DEBUG: drawing: ", report_time::yes);

    try {
        // const size_t projection_no = 0;
        // const acmacs::Layout orig_layout(*chart->projection_modify(projection_no)->layout());
        // const auto orig_transformation = chart->projection_modify(projection_no)->transformation();

        rjson::value settings{rjson::object{{"apply", rjson::array{"title"}}}};
        for (const auto& settings_file_name : {"acmacs-map-draw.json"sv}) {
            if (const auto filename = fmt::format("{}/share/conf/{}", acmacs::acmacsd_root(), settings_file_name); fs::exists(filename))
                settings.update(rjson::parse_file(filename, rjson::remove_comments::no));
            else
                fmt::print(stderr, "WARNING: cannot load \"{}\": file not found\n", filename);
        }
        for (auto fn : settings_files)
            settings.update(rjson::parse_file(fn, rjson::remove_comments::no));

        std::string output{output_pdf};
        if (name_after_mod)
            output += mod_name(settings["apply"]) + ".pdf";
        settings["output_pdf"] = output;

        apply_mods(chart_draw, settings["apply"], settings);
        chart_draw.calculate_viewport();
        AD_INFO("{}\n{}", chart_draw.viewport("map-draw-interactive main"), chart_draw.chart(0).modified_transformation());
        chart_draw.draw(output, 800, report_time::yes);
        // chart->projection_modify(projection_no)->transformation(orig_transformation);
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
        if (const std::string_view mod_desc_s{mod_desc.to<std::string_view>()}; name.empty() && mod_desc_s[0] != '?' && mod_desc_s[0] != '_')
            name = mod_desc_s;
    });
    return name;

} // mod_name

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
