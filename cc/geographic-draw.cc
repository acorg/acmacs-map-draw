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
static GeographicMapColoring* make_coloring(const rjson::value& aSettings);

int main(int argc, char* const argv[])
{
    int exit_code = 1;
    try {
        argc_argv args(argc, argv, {
                {"-h", false},
                {"--help", false},
                // {"--help-mods", false},
                // {"--help-select", false},
                {"--time-series", ""},
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

static inline std::vector<std::string> make_list(const rjson::array& aSource)
{
    std::vector<std::string> result;
    std::transform(std::begin(aSource), std::end(aSource), std::back_inserter(result), [](const auto& entry) -> std::string { return entry; });
    return result;
}

// ----------------------------------------------------------------------

int draw(const argc_argv& args)
{
    const bool verbose = args["-v"] || args["--verbose"];

    setup_dbs(args["--db-dir"], verbose);


    if (args["--init-settings"]) {
        auto write_settings = [](std::ostream& out) { out << geographic_settings_default_raw() << '\n'; };
        if (args["--init-settings"] == "-") {
            write_settings(std::cout);
        }
        else {
            std::ofstream out(args["--init-settings"]);
            write_settings(out);
        }
    }

    auto settings = geographic_settings_default();

    auto load_settings = [&](argc_argv::strings aFilenames) {
        for (auto fn: aFilenames) {
            if (verbose)
                std::cerr << "DEBUG: reading settings from " << fn << '\n';
            settings.update(rjson::parse_file(fn, rjson::remove_comments::Yes));
        }
    };
    if (args["-s"])
        load_settings(args["-s"]);
    if (args["--settings"])
        load_settings(args["--settings"]);
      // std::cerr << "DEBUG: loaded settings\n" << settings.to_json_pp() << '\n';

    const std::string start_date = settings["start_date"], end_date = settings["end_date"];
    std::unique_ptr<GeographicMapColoring> coloring{make_coloring(settings)};

    if (args["--time-series"] == "") {
          // Single map
        std::cerr << "INFO: single map\n";
        GeographicMapWithPointsFromHidb geographic_map(args[0], settings["point_size_in_pixels"], settings["point_density"], settings["continent_outline_color"], settings["continent_outline_width"]);
        geographic_map.add_points_from_hidb_colored_by(*coloring, ColorOverride{}, make_list(settings["priority"]), start_date, end_date);
        if (static_cast<std::string>(settings["title_text"]) != "") {
            const rjson::object& title_data = settings["title"];
            auto& title = geographic_map.title()
                    .show(true)
                    .padding(title_data.get_or_default("padding", 10.0))
                    .background(title_data.get_or_default("background", "transparent"))
                    .border_color(title_data.get_or_default("border_color", "black"))
                    .border_width(title_data.get_or_default("border_width", 0.0))
                    .text_color(title_data.get_or_default("text_color", "black"))
                    .text_size(title_data.get_or_default("text_size", 12.0))
                    ;
            const auto& offset = title_data.get_or_empty_array("offset");
            if (!offset.empty())
                title.offset(offset[0], offset[1]);
            title.add_line(settings["title_text"]);
        }

        acmacs_base::TempFile temp_file(".pdf");
        const std::string output = args.number_of_arguments() > 1 ? std::string{args[1]} : static_cast<std::string>(temp_file);
        geographic_map.draw(output, settings["output_image_width"]);
        if (args["--open"])
            acmacs::quicklook(output, 2);
    }
    else {
        std::cerr << "INFO: time series " << static_cast<std::string>(args["--time-series"]) << '\n';
        if (args.number_of_arguments() < 2)
            throw std::runtime_error("Please provide output filename prefix for time series");
        std::unique_ptr<GeographicTimeSeriesBase> time_series;
        if (args["--time-series"] == "monthly")
            time_series.reset(new GeographicTimeSeriesMonthly(args[0], start_date, end_date, settings["point_size_in_pixels"], settings["point_density"], settings["continent_outline_color"], settings["continent_outline_width"]));
        else if (args["--time-series"] == "yearly")
            time_series.reset(new GeographicTimeSeriesYearly(args[0], start_date, end_date, settings["point_size_in_pixels"], settings["point_density"], settings["continent_outline_color"], settings["continent_outline_width"]));
        else if (args["--time-series"] == "weekly")
            time_series.reset(new GeographicTimeSeriesWeekly(args[0], start_date, end_date, settings["point_size_in_pixels"], settings["point_density"], settings["continent_outline_color"], settings["continent_outline_width"]));
        else
            throw std::runtime_error("Unsupported time series argument: " + static_cast<std::string>(args["--time-series"]) + " (monthly or yearly or weekly expected)");
        time_series->draw(std::string{args[1]}, *coloring, ColorOverride{}, settings["output_image_width"]);
    }

    return 0;

} // draw

// ----------------------------------------------------------------------

static inline GeographicMapColoring::TagToColor make_map(const rjson::object& aSource)
{
    GeographicMapColoring::TagToColor result;
    auto rjson_to_coloring_data = [](const auto& entry) -> GeographicMapColoring::TagToColor::value_type {
        if (!entry.first.empty() && (entry.first.front() == '?' || entry.first.back() == '?'))
            return {entry.first, {"pink"}}; // comment field
        else
            return {entry.first, {entry.second["fill"], entry.second["outline"], entry.second["outline_width"]}};
    };
    std::transform(std::begin(aSource), std::end(aSource), std::inserter(result, result.end()), rjson_to_coloring_data);
    return result;
}

GeographicMapColoring* make_coloring(const rjson::value& aSettings)
{
    GeographicMapColoring* coloring = nullptr;
    const std::string coloring_name = aSettings["coloring"];
    if (coloring_name == "" || coloring_name == "continent")
        coloring = new ColoringByContinent(make_map(aSettings["continent_color"]));
    else if (coloring_name == "clade")
        coloring = new ColoringByClade(make_map(aSettings["clade_color"]));
    else if (coloring_name == "lineage")
        coloring = new ColoringByLineage(make_map(aSettings["lineage_color"]));
    else if (coloring_name == "lineage-deletion-mutants")
        coloring = new ColoringByLineageAndDeletionMutants(make_map(aSettings["lineage_color"]));
    else
        throw std::runtime_error("Unsupported coloring: " + coloring_name);
    std::cerr << "INFO: coloring: " << coloring_name << '\n';
    return coloring;

} // make_coloring

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End: