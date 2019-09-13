#include "geographic-settings.hh"

// ----------------------------------------------------------------------

static const char* const SETTINGS_DEFAULT = R"({ "_":"-*- js-indent-level: 2 -*-",
  "coloring": {"N": "clade"},
  "coloring?": [
    {"N": "continent", "?continent_color": {"EUROPE": {"fill": "green", "outline": "black", "outline_width": 0}}},
    {"N": "clade", "?clade_color": {"SEQUENCED": {"fill": "yellow", "outline": "black", "outline_width": 0}}},
    {"N": "lineage", "?lineage_color": {"VICTORIA_2DEL": {"fill": "#23a8d1", "outline": "black", "outline_width": 0}, "VICTORIA_3DEL": {"fill": "#80FF00", "outline": "black", "outline_width": 0}}},
    {"N": "lineage-deletion-mutants", "?lineage_color": {"VICTORIA_2DEL": {"fill": "#23a8d1", "outline": "black", "outline_width": 0}, "VICTORIA_3DEL": {"fill": "#80FF00", "outline": "black", "outline_width": 0}}},
    {"N": "amino-acid", "apply": [{"sequenced": true, "color": "red"}, {"aa": ["156N" ,"155G"], "color": "blue"}], "report": false}
  ],
  "???color_override?": {"B": {"?": "B/Vic deletion mutants", "?B/DOMINICAN REPUBLIC/9932/2016": "#00FFFF"}},
  "start_date": "2017-01-01",
  "end_date": "",
  "point_size_in_pixels": 4.0,
  "point_density": 0.8,
  "continent_outline_color": "grey63",
  "continent_outline_width": 0.5,
  "output_image_width": 800,

  "title_text": "",
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
    "SEQUENCED":     {"fill": "yellow",          "outline": "black", "outline_width": 0},

    "?": "========== H3",
    "3C.3":           {"fill": "cornflowerblue",  "outline": "black", "outline_width": 0},
    "2A":             {"fill": "red",             "outline": "black", "outline_width": 0},
    "2A1":            {"fill": "#ffab91",         "outline": "black", "outline_width": 0},
    "2A1A":           {"fill": "#E040FB",         "outline": "black", "outline_width": 0},
    "2A1B":           {"fill": "#CD5C5C",         "outline": "black", "outline_width": 0},
    "2A2":            {"fill": "#EEB422",         "outline": "black", "outline_width": 0},
    "3A":             {"fill": "green",           "outline": "black", "outline_width": 0},
    "3B":             {"fill": "blue",            "outline": "black", "outline_width": 0},

    "?": "========== H1pdm",
    "6B1":           {"fill": "blue",            "outline": "black", "outline_width": 0},
    "6B2":           {"fill": "red",             "outline": "black", "outline_width": 0},

    "?": "========== B/Yam",
    "Y2":            {"fill": "cornflowerblue",  "outline": "black", "outline_width": 0},
    "Y3":            {"fill": "red",             "outline": "black", "outline_width": 0},

    "?": "========== B/Vic",
    "V1":             {"fill": "blue",            "outline": "black", "outline_width": 0},
    "V1A":            {"fill": "cornflowerblue",  "outline": "black", "outline_width": 0},
    "V1B":            {"fill": "red",             "outline": "black", "outline_width": 0},
    "DEL2017":       {"fill": "#000080",         "outline": "black", "outline_width": 0},
    "TRIPLEDEL2017": {"fill": "#46f0f0",         "outline": "black", "outline_width": 0}
  },

  "lineage_color": {
    "YAMAGATA":      {"fill": "#d10000",         "outline": "black", "outline_width": 0},
    "VICTORIA":      {"fill": "#2700b7",         "outline": "black", "outline_width": 0},
    "VICTORIA_2DEL":  {"fill": "#23a8d1",         "outline": "black", "outline_width": 0},
    "VICTORIA_3DEL":  {"fill": "#80FF00",         "outline": "black", "outline_width": 0}
  },

  "?": "draw VICTORIA_2DEL on top of VICTORIA",
  "priority": ["YAMAGATA", "VICTORIA", "VICTORIA_2DEL"]

}
)";

rjson::value geographic_settings_default()
{
    try {
        return rjson::parse_string(SETTINGS_DEFAULT);
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
