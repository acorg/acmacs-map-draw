#include "geographic-settings.hh"

// ----------------------------------------------------------------------

static const char* const SETTINGS_DEFAULT = R"(
{
  "coloring": "clade",
  "coloring?": ["continent", "clade", "lineage", "lineage-deletion-mutants"],
  "color_override?": {"B": {"?": "B/Vic deletion mutants", "?B/DOMINICAN REPUBLIC/9932/2016": "#00FFFF"}},
  "start_date": "2017-01-01",
  "end_date": "",
  "point_size_in_pixels": 4.0,
  "point_density": 0.8,
  "continent_outline_color": "grey63",
  "continent_outline_width": 0.5,
  "output_image_width": 800,
  "title": {"offset": [0, 0], "text_size": 20, "background": "transparent", "border_width": 0}
}
)";

rjson::value geographic_settings_default()
{
    try {
        return rjson::parse_string(SETTINGS_DEFAULT, rjson::remove_comments::No);
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: parsing SETTINGS_DEFAULT: " << err.what() << '\n';
        throw;
    }
}

// ----------------------------------------------------------------------

static const char* const SETTINGS_BUILTIN_MODS = R"(
{
  "continent_color": {
    "EUROPE":            "green",
    "CENTRAL-AMERICA":   "#AAF9FF",
    "MIDDLE-EAST":       "#8000FF",
    "NORTH-AMERICA":     "blue4",
    "AFRICA":            "darkorange1",
    "ASIA":              "red",
    "RUSSIA":            "maroon",
    "AUSTRALIA-OCEANIA": "hotpink",
    "SOUTH-AMERICA":     "turquoise",
    "ANTARCTICA":        "grey50",
    "UNKNOWN":           "grey50",
    "":                  "grey50"
  },

  "clade_color": {
    "?": "========== sequenced but not in any clade",
    "": "grey50",

    "?": "========== H3",
    "3C3": "cornflowerblue",
    "3C2a": "red",
    "3C2a1": "darkred",
    "3C3a": "green",
    "3C3b": "blue",

    "?": "========== H1pdm",
    "6B1": "blue",
    "6B2": "red",

    "?": "========== B/Yam",
    "Y2": "cornflowerblue",
    "Y3": "red",

    "?": "========== B/Vic",
    "1": "blue",
    "1A": "cornflowerblue",
    "1B": "red",
    "DEL2017": "#000080",
    "TRIPLEDEL2017": "#46f0f0"
  },

  "lineage_color": {
    "YAMAGATA": "#d10000",
    "VICTORIA": "#2700b7",
    "VICTORIA_DEL": "#23a8d1"
  }
}
)";

rjson::value geographic_settings_builtin_mods()
{
    try {
        return rjson::parse_string(SETTINGS_BUILTIN_MODS, rjson::remove_comments::Yes);
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: parsing SETTINGS_BUILTIN_MODS: " << err.what() << '\n';
        throw;
    }
}

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
