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
static void set_title(map_elements::Title& aTitle, const rjson::value& aSettings, bool use_title_text);

int main(int argc, char* const argv[])
{
    int exit_code = 1;
    try {
        argc_argv args(argc, argv, {
                {"-h", false},
                {"--help", false},
                // {"--help-mods", false},
                // {"--help-select", false},
                {"--time-series", "", "monthly, yearly, weekly (output prefix required)"},
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

static inline std::vector<std::string> make_list(const rjson::value& aSource)
{
    std::vector<std::string> result;
    rjson::copy(aSource, result);
    return result;
}

// ----------------------------------------------------------------------

int draw(const argc_argv& args)
{
    using namespace std::string_literals;
    const bool verbose = args["-v"] || args["--verbose"];

    setup_dbs(args["--db-dir"].str(), verbose);

    if (args["--init-settings"]) {
        auto write_settings = [](std::ostream& out) { out << geographic_settings_default_raw() << '\n'; };
        if (args["--init-settings"] == "-") {
            write_settings(std::cout);
        }
        else {
            std::ofstream out(args["--init-settings"].str());
            write_settings(out);
        }
    }

    auto settings = geographic_settings_default();

    auto load_settings = [&](argc_argv::strings aFilenames) {
        for (auto fn: aFilenames) {
            if (verbose)
                std::cerr << "DEBUG: reading settings from " << fn << '\n';
            settings.update(rjson::parse_file(fn, rjson::remove_comments::yes));
        }
    };
    if (args["-s"])
        load_settings(args["-s"]);
    if (args["--settings"])
        load_settings(args["--settings"]);
      // std::cerr << "DEBUG: loaded settings\n" << settings.to_json_pp() << '\n';

    const std::string_view start_date = settings["start_date"], end_date = settings["end_date"];
    std::unique_ptr<GeographicMapColoring> coloring{make_coloring(settings)};

    if (args["--time-series"] == "") {
          // Single map
        std::cerr << "INFO: single map\n";
        GeographicMapWithPointsFromHidb geographic_map(string::upper(args[0]), settings["point_size_in_pixels"], settings["point_density"], Color(static_cast<std::string_view>(settings["continent_outline_color"])), settings["continent_outline_width"]);
        geographic_map.add_points_from_hidb_colored_by(*coloring, ColorOverride{}, make_list(settings["priority"]), start_date, end_date);
        set_title(geographic_map.title(), settings, true);

        acmacs::file::temp temp_file(".pdf");
        const std::string output = args.number_of_arguments() > 1 ? std::string{args[1]} : static_cast<std::string>(temp_file);
        geographic_map.draw(output, settings["output_image_width"]);
        if (args["--open"])
            acmacs::quicklook(output, 2);
    }
    else {
        std::cerr << "INFO: time series " << static_cast<std::string_view>(args["--time-series"]) << '\n';
        if (args.number_of_arguments() < 2)
            throw std::runtime_error("Please provide output filename prefix for time series");
        std::unique_ptr<GeographicTimeSeriesBase> time_series;
        if (args["--time-series"] == "monthly")
            time_series.reset(new GeographicTimeSeriesMonthly(string::upper(args[0]), start_date, end_date, make_list(settings["priority"]), settings["point_size_in_pixels"], settings["point_density"], Color(static_cast<std::string_view>(settings["continent_outline_color"])), settings["continent_outline_width"]));
        else if (args["--time-series"] == "yearly")
            time_series.reset(new GeographicTimeSeriesYearly(string::upper(args[0]), start_date, end_date, make_list(settings["priority"]), settings["point_size_in_pixels"], settings["point_density"], Color(static_cast<std::string_view>(settings["continent_outline_color"])), settings["continent_outline_width"]));
        else if (args["--time-series"] == "weekly")
            time_series.reset(new GeographicTimeSeriesWeekly(string::upper(args[0]), start_date, end_date, make_list(settings["priority"]), settings["point_size_in_pixels"], settings["point_density"], Color(static_cast<std::string_view>(settings["continent_outline_color"])), settings["continent_outline_width"]));
        else
            throw std::runtime_error("Unsupported time series argument: " + std::string{static_cast<std::string_view>(args["--time-series"])} + " (monthly or yearly or weekly expected)");
        set_title(time_series->title(), settings, false);
        time_series->draw(std::string{args[1]}, *coloring, ColorOverride{}, settings["output_image_width"]);
    }

    return 0;

} // draw

// ----------------------------------------------------------------------

static inline GeographicMapColoring::TagToColor make_map(const rjson::value& aSource)
{
    GeographicMapColoring::TagToColor result;
    auto rjson_to_coloring_data = [](const rjson::object::value_type& entry) -> GeographicMapColoring::TagToColor::value_type {
        if (!entry.first.empty() && (entry.first.front() == '?' || entry.first.back() == '?'))
            return {entry.first, {Color("pink")}}; // comment field
        else
            return {entry.first, {Color(static_cast<std::string_view>(entry.second["fill"])), Color(static_cast<std::string_view>(entry.second["outline"])), entry.second["outline_width"]}};
    };
    rjson::transform(aSource, std::inserter(result, result.end()), rjson_to_coloring_data);
    return result;
}

GeographicMapColoring* make_coloring(const rjson::value& aSettings)
{
    GeographicMapColoring* coloring = nullptr;
    const std::string_view coloring_name = aSettings["coloring"];
    if (coloring_name == "" || coloring_name == "continent")
        coloring = new ColoringByContinent(make_map(aSettings["continent_color"]));
    else if (coloring_name == "clade")
        coloring = new ColoringByClade(make_map(aSettings["clade_color"]));
    else if (coloring_name == "lineage")
        coloring = new ColoringByLineage(make_map(aSettings["lineage_color"]));
    else if (coloring_name == "lineage-deletion-mutants")
        coloring = new ColoringByLineageAndDeletionMutants(make_map(aSettings["lineage_color"]));
    else
        throw std::runtime_error("Unsupported coloring: " + std::string(coloring_name));
    std::cerr << "INFO: coloring: " << coloring_name << '\n';
    return coloring;

} // make_coloring

// ----------------------------------------------------------------------

void set_title(map_elements::Title& aTitle, const rjson::value& aSettings, bool use_title_text)
{
    if (!use_title_text || static_cast<std::string_view>(aSettings["title_text"]) != "") {
        const rjson::value& title_data = aSettings["title"];
        aTitle.show(true)
                .padding(rjson::get_or(title_data, "padding", 10.0))
                .background(Color(rjson::get_or(title_data, "background", "transparent")))
                .border_color(Color(rjson::get_or(title_data, "border_color", "black")))
                .border_width(rjson::get_or(title_data, "border_width", 0.0))
                .text_color(Color(rjson::get_or(title_data, "text_color", "black")))
                .text_size(rjson::get_or(title_data, "text_size", 12.0))
                ;
        if (const auto& offset = title_data["offset"]; !offset.empty())
            aTitle.offset({offset[0], offset[1]});
        if (use_title_text)
            aTitle.add_line(std::string(aSettings["title_text"]));
    }

} // set_title

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
