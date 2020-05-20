#include <iostream>
#include <fstream>
#include <string>

#include "acmacs-base/argv.hh"
#include "acmacs-base/file-stream.hh"
#include "acmacs-base/temp-file.hh"
#include "acmacs-base/quicklook.hh"

#include "geographic-settings.hh"
#include "geographic-map.hh"
#include "setup-dbs.hh"

// ----------------------------------------------------------------------

using namespace acmacs::argv;
struct Options : public argv
{
    Options(int a_argc, const char* const a_argv[], on_error on_err = on_error::exit) : argv() { parse(a_argc, a_argv, on_err); }

    option<str> db_dir{*this, "db-dir"};

    option<str>       time_series{*this, "time-series", desc{"monthly, yearly, weekly (output prefix required)"}};
    option<str_array> settings_files{*this, 's', "settings"};
    option<str>       init_settings{*this, 'i', "init-settings"};
    option<bool>      open{*this, "open"};
    option<bool>      ql{*this, "ql"};
    option<bool>      verbose{*this, 'v', "verbose"};

    argument<str> subtype{*this, arg_name{"H1|H3|B"}, mandatory};
    argument<str> output_pdf{*this, arg_name{"map.pdf"}};
};

static int draw(const Options& opt);
static GeographicMapColoring* make_coloring(const rjson::value& aSettings);
static void set_title(map_elements::v1::Title& aTitle, const rjson::value& aSettings, bool use_title_text);

int main(int argc, char* const argv[])
{
    int exit_code = 1;
    try {
        Options opt(argc, argv);
        exit_code = draw(opt);
    }
    catch (std::exception& err) {
        fmt::print(stderr, "> ERROR {}\n", err);
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

int draw(const Options& opt)
{
    setup_dbs(opt.db_dir, opt.verbose);

    if (opt.init_settings) {
        acmacs::file::ofstream out(opt.init_settings);
        out.stream() << geographic_settings_default_raw() << '\n';
    }

    auto settings = geographic_settings_default();
    for (auto fn : *opt.settings_files) {
        if (opt.verbose)
            std::cerr << "DEBUG: reading settings from " << fn << '\n';
        try {
            settings.update(rjson::parse_file(fn, rjson::remove_comments::yes));
        }
        catch (std::exception& err) {
            throw std::runtime_error(acmacs::string::concat(fn, ": ", err.what()));
        }
        // if (opt.verbose)
        //     std::cerr << "DEBUG: reading settings DONE from " << fn << '\n';
    }

    const std::string_view start_date{settings["start_date"].to<std::string_view>()}, end_date{settings["end_date"].to<std::string_view>()};
    std::unique_ptr<GeographicMapColoring> coloring{make_coloring(settings)};

    const auto subtype = string::upper(*opt.subtype);
    if (!opt.time_series) {
          // Single map
        std::cerr << "INFO: single map\n";
        GeographicMapWithPointsFromHidb geographic_map(subtype, settings["point_size_in_pixels"].to<double>(), settings["point_density"].to<double>(), Color(settings["continent_outline_color"].to<std::string_view>()), settings["continent_outline_width"].to<double>());
        geographic_map.add_points_from_hidb_colored_by(*coloring, ColorOverride{}, make_list(settings["priority"]), start_date, end_date);
        set_title(geographic_map.title(), settings, true);

        acmacs::file::temp temp_file(".pdf");
        const std::string output = opt.output_pdf ? std::string{opt.output_pdf} : static_cast<std::string>(temp_file);
        geographic_map.draw(output, settings["output_image_width"].to<double>());
        acmacs::open_or_quicklook(opt.open, opt.ql, output, 2);
    }
    else {
        std::cerr << "INFO: time series " << static_cast<std::string_view>(opt.time_series) << '\n';
        if (!opt.output_pdf)
            throw std::runtime_error("Please provide output filename prefix for time series");
        std::unique_ptr<GeographicTimeSeries> time_series;
        if (opt.time_series == "monthly")
            time_series.reset(new GeographicTimeSeriesMonthly(subtype, start_date, end_date, make_list(settings["priority"]), settings["point_size_in_pixels"].to<double>(), settings["point_density"].to<double>(), Color(settings["continent_outline_color"].to<std::string_view>()), settings["continent_outline_width"].to<double>()));
        else if (opt.time_series == "yearly")
            time_series.reset(new GeographicTimeSeriesYearly(subtype, start_date, end_date, make_list(settings["priority"]), settings["point_size_in_pixels"].to<double>(), settings["point_density"].to<double>(), Color(settings["continent_outline_color"].to<std::string_view>()), settings["continent_outline_width"].to<double>()));
        else if (opt.time_series == "weekly")
            time_series.reset(new GeographicTimeSeriesWeekly(subtype, start_date, end_date, make_list(settings["priority"]), settings["point_size_in_pixels"].to<double>(), settings["point_density"].to<double>(), Color(settings["continent_outline_color"].to<std::string_view>()), settings["continent_outline_width"].to<double>()));
        else
            throw std::runtime_error(fmt::format("Unsupported time series argument: {} (monthly or yearly or weekly expected)", *opt.time_series));
        set_title(time_series->title(), settings, false);
        time_series->draw(opt.output_pdf, *coloring, ColorOverride{}, settings["output_image_width"].to<double>());
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
            return {entry.first, {Color(entry.second["fill"].to<std::string_view>()), Color(entry.second["outline"].to<std::string_view>()), entry.second["outline_width"].to<double>()}};
    };
    rjson::transform(aSource, std::inserter(result, result.end()), rjson_to_coloring_data);
    return result;
}

GeographicMapColoring* make_coloring(const rjson::value& aSettings)
{
    GeographicMapColoring* coloring = nullptr;
    const std::string_view coloring_name{aSettings["coloring"]["N"].to<std::string_view>()};
    if (coloring_name == "continent") {
        rjson::value continent_color = aSettings["continent_color"];
        if (const auto& continent_color_override = aSettings["coloring"]["continent_color"]; !continent_color_override.is_null())
            continent_color.update(continent_color_override);
        coloring = new ColoringByContinent(make_map(continent_color));
    }
    else if (coloring_name == "clade") {
        rjson::value clade_color = aSettings["clade_color"];
        if (const auto& clade_color_override = aSettings["coloring"]["clade_color"]; !clade_color_override.is_null())
            clade_color.update(clade_color_override);
        coloring = new ColoringByClade(make_map(clade_color));
    }
    else if (coloring_name == "lineage") {
        rjson::value lineage_color = aSettings["lineage_color"];
        if (const auto& lineage_color_override = aSettings["coloring"]["lineage_color"]; !lineage_color_override.is_null())
            lineage_color.update(lineage_color_override);
        coloring = new ColoringByLineage(make_map(lineage_color));
    }
    else if (coloring_name == "lineage-deletion-mutants") {
        rjson::value lineage_color = aSettings["lineage_color"];
        if (const auto& lineage_color_override = aSettings["coloring"]["lineage_color"]; !lineage_color_override.is_null())
            lineage_color.update(lineage_color_override);
        coloring = new ColoringByLineageAndDeletionMutants(make_map(lineage_color));
    }
    else if (coloring_name == "amino-acid") {
        coloring = new ColoringByAminoAcid(aSettings["coloring"]);
    }
    else
        throw std::runtime_error("Unsupported coloring: " + std::string(coloring_name));
    std::cerr << "INFO: coloring: " << coloring_name << '\n';
    return coloring;

} // make_coloring

// ----------------------------------------------------------------------

void set_title(map_elements::v1::Title& aTitle, const rjson::value& aSettings, bool use_title_text)
{
    if (!use_title_text || aSettings["title_text"].to<std::string_view>() != "") {
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
            aTitle.offset({offset[0].to<double>(), offset[1].to<double>()});
        if (use_title_text)
            aTitle.add_line(aSettings["title_text"].to<std::string>());
    }

} // set_title

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
