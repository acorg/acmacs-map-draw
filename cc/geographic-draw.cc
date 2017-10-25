#include <iostream>
#include <fstream>
#include <string>
using namespace std::string_literals;

#include "acmacs-base/argc-argv.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/quicklook.hh"

#include "geographic-settings.hh"
#include "geographic-map.hh"
#include "setup-dbs.hh"

// ----------------------------------------------------------------------

static int draw(const argc_argv& args);

int main(int argc, char* const argv[])
{
    int exit_code = 1;
    try {
        argc_argv args(argc, argv, {
                {"-h", false},
                {"--help", false},
                // {"--help-mods", false},
                // {"--help-select", false},
                {"-s", argc_argv::strings{}},
                {"--settings", argc_argv::strings{}},
                {"--init-settings", ""},
                {"--open", false},
                {"--db-dir", ""},
                {"-v", false},
                {"--verbose", false},
        });
        // if (args["--help-mods"])
        //     std::cerr << settings_help_mods();
        // else if (args["--help-select"])
        //     std::cerr << settings_help_select();
        if (args["-h"] || args["--help"] || args.number_of_arguments() < 1 || args.number_of_arguments() > 2)
            std::cerr << "Usage: " << args.program() << " [options] <H1|H3|B> [<map.pdf>]\n" << args.usage_options() << '\n';
        else
            exit_code = draw(args);
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        exit_code = 2;
    }
    return exit_code;
}

// ----------------------------------------------------------------------

static inline std::map<std::string, std::string> make_map(const rjson::object& aSource)
{
    std::map<std::string, std::string> result;
    std::transform(std::begin(aSource), std::end(aSource), std::inserter(result, result.end()), [](const auto& entry) -> decltype(result)::value_type { return {entry.first, entry.second}; });
    return result;
}

// ----------------------------------------------------------------------

int draw(const argc_argv& args)
{
    const bool verbose = args["-v"] || args["--verbose"];

    if (args["--db-dir"])
        setup_dbs(args["--db-dir"], verbose);

    auto settings = geographic_settings_default();

    if (args["--init-settings"]) {
        auto write_settings = [&settings](std::ostream& out) { out << settings.to_json_pp() << '\n'; };
        if (args["--init-settings"] == "-") {
            write_settings(std::cout);
        }
        else {
            std::ofstream out(args["--init-settings"]);
            write_settings(out);
        }
    }

    settings.update(geographic_settings_builtin_mods());

    auto load_settings = [&](argc_argv::strings aFilenames) {
        for (auto fn: aFilenames) {
            if (verbose)
                std::cerr << "DEBUG: reading settings from " << fn << '\n';
            settings.update(rjson::parse_file(fn, rjson::remove_comments::No));
        }
    };
    if (args["-s"])
        load_settings(args["-s"]);
    if (args["--settings"])
        load_settings(args["--settings"]);
      // std::cerr << "DEBUG: loaded settings\n" << settings.to_json_pp() << '\n';

    GeographicMapWithPointsFromHidb geographic_map(args[0], settings["point_size_in_pixels"], settings["point_density"], settings["outline_color"], settings["outline_width"]);
    const std::string coloring = settings["coloring"], start_date = settings["start_date"], end_date = settings["end_date"];
    if (coloring == "" || coloring == "continent")
        geographic_map.add_points_from_hidb_colored_by_continent(make_map(settings["continent_color"]), /* color_override */{}, start_date, end_date);
    else if (coloring == "clade")
        geographic_map.add_points_from_hidb_colored_by_clade(make_map(settings["clade_color"]), /* color_override */{}, start_date, end_date);
    else if (coloring == "lineage")
        geographic_map.add_points_from_hidb_colored_by_lineage(make_map(settings["lineage_color"]), /* color_override */{}, start_date, end_date);
    else
        throw std::runtime_error("Unsupported coloring: " + coloring);

      //_update_title(geographic_map.title(), settings).add_line(title)

    acmacs_base::TempFile temp_file(".pdf");
    const std::string output = args.number_of_arguments() > 1 ? std::string{args[1]} : static_cast<std::string>(temp_file);
    geographic_map.draw(output, settings["output_image_width"]);

    if (args["--open"])
        acmacs::quicklook(output, 2);
    return 0;

} // draw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
