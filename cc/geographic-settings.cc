#include "geographic-settings.hh"

// ----------------------------------------------------------------------

static const char* const SETTINGS_DEFAULT = R"({ "_":"-*- js-indent-level: 2 -*-",
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

  "title_text": "Geographic map",
  "title": {"offset": [0, 0], "text_size": 20, "background": "transparent", "border_color": "black", "border_width": 0, "text_color": "black", "padding": 10.0},

  "continent_color": {
    "EUROPE":            {"fill": "green",       "outline": "black", "outline_width": 0},
    "CENTRAL-AMERICA":   {"fill": "#AAF9FF",     "outline": "black", "outline_width": 0},
    "MIDDLE-EAST":       {"fill": "#8000FF",     "outline": "black", "outline_width": 0},
    "NORTH-AMERICA":     {"fill": "blue4",       "outline": "black", "outline_width": 0},
    "AFRICA":            {"fill": "darkorange1", "outline": "black", "outline_width": 0},
    "ASIA":              {"fill": "red",         "outline": "black", "outline_width": 0},
    "RUSSIA":            {"fill": "maroon",      "outline": "black", "outline_width": 0},
    "AUSTRALIA-OCEANIA": {"fill": "hotpink",     "outline": "black", "outline_width": 0},
    "SOUTH-AMERICA":     {"fill": "turquoise",   "outline": "black", "outline_width": 0},
    "ANTARCTICA":        {"fill": "grey50",      "outline": "black", "outline_width": 0},
    "UNKNOWN":           {"fill": "grey50",      "outline": "black", "outline_width": 0},
    "":                  {"fill": "grey50",      "outline": "black", "outline_width": 0}
  },

  "clade_color": {
    "?": "========== sequenced but not in any clade",
    "":              {"fill": "grey50",          "outline": "black", "outline_width": 0},

    "?": "========== H3",
    "3C3":           {"fill": "cornflowerblue",  "outline": "black", "outline_width": 0},
    "3C2a":          {"fill": "red",             "outline": "black", "outline_width": 0},
    "3C2a1":         {"fill": "darkred",         "outline": "black", "outline_width": 0},
    "3C3a":          {"fill": "green",           "outline": "black", "outline_width": 0},
    "3C3b":          {"fill": "blue",            "outline": "black", "outline_width": 0},

    "?": "========== H1pdm",
    "6B1":           {"fill": "blue",            "outline": "black", "outline_width": 0},
    "6B2":           {"fill": "red",             "outline": "black", "outline_width": 0},

    "?": "========== B/Yam",
    "Y2":            {"fill": "cornflowerblue",  "outline": "black", "outline_width": 0},
    "Y3":            {"fill": "red",             "outline": "black", "outline_width": 0},

    "?": "========== B/Vic",
    "1":             {"fill": "blue",            "outline": "black", "outline_width": 0},
    "1A":            {"fill": "cornflowerblue",  "outline": "black", "outline_width": 0},
    "1B":            {"fill": "red",             "outline": "black", "outline_width": 0},
    "DEL2017":       {"fill": "#000080",         "outline": "black", "outline_width": 0},
    "TRIPLEDEL2017": {"fill": "#46f0f0",         "outline": "black", "outline_width": 0}
  },

  "lineage_color": {
    "YAMAGATA":      {"fill": "#d10000",         "outline": "black", "outline_width": 0},
    "VICTORIA":      {"fill": "#2700b7",         "outline": "black", "outline_width": 0},
    "VICTORIA_DEL":  {"fill": "#23a8d1",         "outline": "black", "outline_width": 0}
  },

  "?": "draw VICTORIA_DEL on top of VICTORIA",
  "priority": ["YAMAGATA", "VICTORIA", "VICTORIA_DEL"]

}
)";

rjson::value geographic_settings_default()
{
    try {
        return rjson::parse_string(SETTINGS_DEFAULT, rjson::remove_comments::Yes);
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: parsing SETTINGS_DEFAULT: " << err.what() << '\n';
        throw;
    }
}

// ----------------------------------------------------------------------

std::string geographic_settings_default_raw()
{
    return SETTINGS_DEFAULT;

} // geographic_settings_default_raw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
