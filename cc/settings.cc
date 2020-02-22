#include "settings.hh"

// ----------------------------------------------------------------------

static const char* const SETTINGS_DEFAULT = R"(
{
  "apply": [
    {"N": "title"},
    "?all_grey",
    "?egg",

    "?clades",
    "?vaccines"
  ]
}
)";

rjson::value settings_default()
{
    try {
        return rjson::parse_string(SETTINGS_DEFAULT, rjson::remove_comments::no);
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: parsing SETTINGS_DEFAULT: " << err.what() << '\n';
        throw;
    }
}

// ----------------------------------------------------------------------

static const char* const SETTINGS_BUILTIN_MODS = R"(
{
  "mods": {
    "default_color": [
      {"N": "antigens", "select": "test", "outline": "black", "fill": "green"},
      {"N": "antigens", "select": "reference", "outline": "black", "fill": "transparent"},
      {"N": "sera", "select": "all", "outline": "black", "fill": "transparent"}
    ],
    "all_grey": [
      {"N": "antigens", "select": "test", "outline": "grey80", "fill": "grey80"},
      {"N": "antigens", "select": "reference", "outline": "grey80", "fill": "transparent"},
      {"N": "sera", "select": "all", "outline": "grey80", "fill": "transparent"}
    ],
    "lower_reference": [
      {"N": "antigens", "select": "reference", "order": "lower"},
      {"N": "sera", "select": "all", "order": "lower"}
    ],
    "labels": [
      {"N": "antigens", "select": "all", "label": {"show": true}},
      {"N": "sera", "select": "all", "label": {"show": true}}
    ],
    "size_reset": [
      {"N": "antigens", "select": "test", "size": 5.0, "outline_width": 1.0},
      {"N": "antigens", "select": "reference", "size": 8.0, "outline_width": 1.0},
      {"N": "sera", "select": "all", "size": 6.5, "outline_width": 1.0}
    ],
    "clades": [
      {"N": "antigens", "select": {"sequenced": true},   "outline": "black", "fill": "yellow", "legend": {"label": "sequenced", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "3C.3"}, "fill": "cornflowerblue", "outline": "black", "legend": {"label": "3C.3", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.3A"},   "fill": "green",          "outline": "black", "legend": {"label": "3a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.3B"},   "fill": "blue",           "outline": "black", "legend": {"label": "3b", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.2A"},   "fill": "#FF0000",        "outline": "black", "legend": {"label": "2a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.2A1"},  "fill": "#ffab91",        "outline": "black", "legend": {"label": "2a1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.2A1A"}, "fill": "#E040FB",        "outline": "black", "legend": {"label": "2a1a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.2A1B"}, "fill": "#CD5C5C",        "outline": "black", "legend": {"label": "2a1b", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.2A2"},  "fill": "#EEB422",        "outline": "black", "legend": {"label": "2a2", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "6B"},   "outline": "black", "fill": "cornflowerblue", "legend": {"label": "6B", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "6B1"},   "outline": "black", "fill": "blue", "legend": {"label": "6B1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "6B2"},   "outline": "black", "fill": "red", "legend": {"label": "6B2", "count": true}, "order": "raise", "report": true, "report_names_threshold": 10},

      {"N": "antigens", "select": {"clade": "V1B"}, "outline": "black", "fill": "red", "legend": {"label": "V1.B", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "V1"}, "outline": "black", "fill": "blue", "legend": {"label": "V1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "V1A"}, "outline": "black", "fill": "cornflowerblue", "legend": {"label": "V1.A", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2DEL2017"}, "outline": "black", "fill": "#DE8244", "legend": {"label": "2-Del mutants", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3DEL2017"},                         "fill": "#ff85ff", "legend": {"label": "3-Del mutants", "count": true}, "outline": "black", "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3DEL2017", "amino_acid": ["133G"]}, "fill": "#ee0000", "legend": {"label": "3-Del mutants 133G", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3DEL2017", "amino_acid": ["133R"]}, "fill": "#70ff70", "legend": {"label": "3-Del mutants 133R", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3DEL2017", "amino_acid": ["133E"]}, "fill": "#ff80ff", "legend": {"label": "3-Del mutants 133E", "count": true}, "outline": "black", "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "Y2", "subtype": "BYAM"}, "outline": "black", "fill": "cornflowerblue", "legend": {"label": "Y2", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "Y3", "subtype": "BYAM"}, "outline": "black", "fill": "red", "legend": {"label": "Y3", "count": true}, "order": "raise", "report": true}
    ],
    "clades_last_6_months": [
      {"N": "antigens", "select": {"clade": "3C.3", "younger_than_days": 183},   "outline": "black", "fill": "cornflowerblue", "legend": {"label": "3C.3", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.3A", "younger_than_days": 183},  "outline": "black", "fill": "green", "legend": {"label": "3a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.3B", "younger_than_days": 183},  "outline": "black", "fill": "blue", "legend": {"label": "3b", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.2A", "younger_than_days": 183},  "outline": "black", "fill": "red", "legend": {"label": "2a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.2A1", "younger_than_days": 183}, "outline": "black", "fill": "#ffab91", "legend": {"label": "2a1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.2A1A", "younger_than_days": 183}, "outline": "black", "fill": "#E040FB", "legend": {"label": "2a1a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.2A1B", "younger_than_days": 183}, "outline": "black", "fill": "#CD5C5C", "legend": {"label": "2a1b", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.2A2", "younger_than_days": 183}, "outline": "black", "fill": "#EEB422", "legend": {"label": "2a2", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "6B", "younger_than_days": 183},   "outline": "black", "fill": "cornflowerblue", "legend": {"label": "6B", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "6B1", "younger_than_days": 183},   "outline": "black", "fill": "blue", "legend": {"label": "6B1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "6B2", "younger_than_days": 183},   "outline": "black", "fill": "red", "legend": {"label": "6B2", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "V1B", "younger_than_days": 183}, "outline": "black", "fill": "red", "legend": {"label": "V1.B", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "V1", "younger_than_days": 183}, "outline": "black", "fill": "blue", "legend": {"label": "V1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "V1A", "younger_than_days": 183}, "outline": "black", "fill": "cornflowerblue", "legend": {"label": "V1.A", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2DEL2017", "younger_than_days": 183}, "outline": "black", "fill": "#DE8244", "legend": {"label": "2-Del mutants", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3DEL2017", "younger_than_days": 183},                         "fill": "#ff85ff", "legend": {"label": "3-Del mutants", "count": true}, "outline": "black", "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3DEL2017", "amino_acid": ["133G"], "younger_than_days": 183}, "fill": "#ee0000", "legend": {"label": "3-Del mutants 133G", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3DEL2017", "amino_acid": ["133R"], "younger_than_days": 183}, "fill": "#70ff70", "legend": {"label": "3-Del mutants 133R", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3DEL2017", "amino_acid": ["133E"], "younger_than_days": 183}, "fill": "#ff80ff", "legend": {"label": "3-Del mutants 133E", "count": true}, "outline": "black", "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "Y2", "younger_than_days": 183, "subtype": "BYAM"}, "outline": "black", "fill": "cornflowerblue", "legend": {"label": "Y2", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "Y3", "younger_than_days": 183, "subtype": "BYAM"}, "outline": "black", "fill": "red", "legend": {"label": "Y3", "count": true}, "order": "raise", "report": true}
    ],
    "clades_last_12_months": [
      {"N": "antigens", "select": {"clade": "3C.3", "younger_than_days": 365},   "outline": "black", "fill": "cornflowerblue", "legend": {"label": "3C.3", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.3A", "younger_than_days": 365},  "outline": "black", "fill": "green", "legend": {"label": "3a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.3B", "younger_than_days": 365},  "outline": "black", "fill": "blue", "legend": {"label": "3b", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.2A", "younger_than_days": 365},  "outline": "black", "fill": "red", "legend": {"label": "2a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.2A1", "younger_than_days": 365}, "outline": "black", "fill": "#ffab91", "legend": {"label": "2a1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.2A1A", "younger_than_days": 365}, "outline": "black", "fill": "#E040FB", "legend": {"label": "2a1a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.2A1B", "younger_than_days": 365}, "outline": "black", "fill": "#CD5C5C", "legend": {"label": "2a1b", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.2A2", "younger_than_days": 365}, "outline": "black", "fill": "#EEB422", "legend": {"label": "2a2", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "6B", "younger_than_days": 365},   "outline": "black", "fill": "cornflowerblue", "legend": {"label": "6B", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "6B1", "younger_than_days": 365},   "outline": "black", "fill": "blue", "legend": {"label": "6B1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "6B2", "younger_than_days": 365},   "outline": "black", "fill": "red", "legend": {"label": "6B2", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "V1B", "younger_than_days": 365}, "outline": "black", "fill": "red", "legend": {"label": "V1.B", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "V1", "younger_than_days": 365}, "outline": "black", "fill": "blue", "legend": {"label": "V1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "V1A", "younger_than_days": 365}, "outline": "black", "fill": "cornflowerblue", "legend": {"label": "V1.A", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2DEL2017", "younger_than_days": 365}, "outline": "black", "fill": "#DE8244", "legend": {"label": "2-Del mutants", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3DEL2017", "younger_than_days": 365},                         "fill": "#ff85ff", "legend": {"label": "3-Del mutants", "count": true}, "outline": "black", "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3DEL2017", "amino_acid": ["133G"], "younger_than_days": 365}, "fill": "#ee0000", "legend": {"label": "3-Del mutants 133G", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3DEL2017", "amino_acid": ["133R"], "younger_than_days": 365}, "fill": "#70ff70", "legend": {"label": "3-Del mutants 133R", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3DEL2017", "amino_acid": ["133E"], "younger_than_days": 365}, "fill": "#ff80ff", "legend": {"label": "3-Del mutants 133E", "count": true}, "outline": "black", "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "Y2", "younger_than_days": 365, "subtype": "BYAM"}, "outline": "black", "fill": "cornflowerblue", "legend": {"label": "Y2", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "Y3", "younger_than_days": 365, "subtype": "BYAM"}, "outline": "black", "fill": "red", "legend": {"label": "Y3", "count": true}, "order": "raise", "report": true}
    ],
    "clades_light": [
      {"N": "antigens", "select": {"clade": "3C.3"},   "outline": "#BFBFBF", "fill": "#8F9DBF", "legend": {"label": "3C.3", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.3A"},  "outline": "#BFBFBF", "fill": "#BFFFBF", "legend": {"label": "3a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.3B"},  "outline": "#BFBFBF", "fill": "#BFBFFF", "legend": {"label": "3b", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.2A"},  "outline": "#BFBFBF", "fill": "#FFBFBF", "legend": {"label": "2a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.2A1"},    "outline": "#BFBFBF", "fill": "#fbe9e7", "legend": {"label": "2a1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.2A1A"},    "outline": "#BFBFBF", "fill": "#e1bee7", "legend": {"label": "2a1a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.2A1B"},    "outline": "#BFBFBF", "fill": "#AF8585", "legend": {"label": "2a1b", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C.2A2"},    "outline": "#BFBFBF", "fill": "#EED69A", "legend": {"label": "2a2", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "6B"},    "outline": "#BFBFBF", "fill": "#8F9DBF", "legend": {"label": "6B", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "6B1"},   "outline": "#BFBFBF", "fill": "#BFBFFF", "legend": {"label": "6B1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "6B2"},   "outline": "#BFBFBF", "fill": "#FFBFBF", "legend": {"label": "6B2", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "V1B"},                              "fill": "#FFBFBF", "outline": "#BFBFBF", "legend": {"label": "V1.B", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "V1"},                               "fill": "#BFBFFF", "outline": "#BFBFBF", "legend": {"label": "V1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "V1A"},                              "fill": "#c2d2ed", "outline": "#BFBFBF", "legend": {"label": "V1.A", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2DEL2017"},                         "fill": "#debda7", "legend": {"label": "2-Del mutants", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3DEL2017"},                         "fill": "#ffdoff", "outline": "#BFBFBF", "legend": {"label": "3-Del mutants", "count": true}, "outline": "black", "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3DEL2017", "amino_acid": ["133G"]}, "fill": "#eeb3b3", "outline": "#BFBFBF", "legend": {"label": "3-Del mutants 133G", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3DEL2017", "amino_acid": ["133R"]}, "fill": "#d1ffd1", "outline": "#BFBFBF", "legend": {"label": "3-Del mutants 133R", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3DEL2017", "amino_acid": ["133E"]}, "fill": "#ffd1ff", "outline": "#BFBFBF", "legend": {"label": "3-Del mutants 133E", "count": true}, "outline": "black", "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "Y2", "subtype": "BYAM"}, "outline": "#BFBFBF", "fill": "#8F9DBF", "legend": {"label": "Y2", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "Y3", "subtype": "BYAM"}, "outline": "#BFBFBF", "fill": "#FFBFBF", "legend": {"label": "Y3", "count": true}, "order": "raise", "report": true}
    ],
    "sequenced": [
      {"N": "antigens", "select": "sequenced", "outline": "black", "fill": "cornflowerblue", "legend": {"label": "sequenced", "count": true}, "order": "raise", "report": true}
    ],
    "egg": [
      {"N": "antigens", "select": {"passage": "egg"}, "shape": "egg", "aspect": 1.0, "?aspect": 0.75},
      {"N": "antigens", "select": {"passage": "reassortant"}, "shape": "egg", "aspect": 1.0, "rotation": 0.2},
      {"N": "sera", "select": {"passage": "egg"}, "shape": "uglyegg", "aspect": 1.0},
      {"N": "sera", "select": {"passage": "reassortant"}, "shape": "uglyegg", "aspect": 1.0, "rotation": 0.2}
    ],
    "continents": [
      {"N": "antigens", "select": {"continent": "europe"}, "fill": "green", "outline": "black", "report": true, "legend": {"label": "Europe", "count": true}},
      {"N": "antigens", "select": {"continent": "central-america"}, "fill": "#AAF9FF", "outline": "black", "report": true, "legend": {"label": "Central America", "count": true}},
      {"N": "antigens", "select": {"continent": "middle-east"}, "fill": "#8000FF", "outline": "black", "report": true, "legend": {"label": "Middle East", "count": true}},
      {"N": "antigens", "select": {"continent": "north-america"}, "fill": "blue4", "outline": "black", "report": true, "legend": {"label": "North America", "count": true}},
      {"N": "antigens", "select": {"continent": "africa"}, "fill": "darkorange1", "outline": "black", "report": true, "legend": {"label": "Africa", "count": true}},
      {"N": "antigens", "select": {"continent": "asia"}, "fill": "red", "outline": "black", "report": true, "legend": {"label": "Asia", "count": true}},
      {"N": "antigens", "select": {"continent": "russia"}, "fill": "maroon", "outline": "black", "report": true, "legend": {"label": "Russia", "count": true}},
      {"N": "antigens", "select": {"continent": "australia-oceania"}, "fill": "hotpink", "outline": "black", "report": true, "legend": {"label": "Oceania", "count": true}},
      {"N": "antigens", "select": {"continent": "south-america"}, "fill": "turquoise", "outline": "black", "report": true, "legend": {"label": "South America", "count": true}},
      {"N": "antigens", "select": {"continent": "antarctica"}, "fill": "grey50", "outline": "black", "report": true, "legend": {"label": "Antarctica", "count": true}}
    ],
    "vaccines": [
      {"N": "antigens", "select": {"vaccine": {"type": "previous"}}, "report": false, "outline": "black", "fill": "blue", "size": 15, "show": true, "order": "raise"},
      {"N": "antigens", "select": {"vaccine": {"type": "current", "passage": "cell"}}, "report": false, "outline": "black", "fill": "red", "size": 15, "show": true, "order": "raise"},
      {"N": "antigens", "select": {"vaccine": {"type": "current", "passage": "egg"}}, "report": false, "outline": "black", "fill": "red", "size": 15, "show": true, "order": "raise"},
      {"N": "antigens", "select": {"vaccine": {"type": "current", "passage": "reassortant"}}, "report": false, "outline": "black", "fill": "green", "size": 15, "show": true, "order": "raise"},
      {"N": "antigens", "select": {"vaccine": {"type": "surrogate"}}, "report": false, "outline": "black", "fill": "pink", "size": 15, "show": true, "order": "raise"}
    ],
    "vaccines_h3_201902": [
      {"N": "antigens", "select": {"vaccine": {"type": "previous", "passage": "cell", "name": "TEXAS/50/2012"}},                 "show": true, "fill": "blue", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "TX/12"}},
      {"N": "antigens", "select": {"vaccine": {"type": "previous", "passage": "cell", "name": "SWITZERLAND/9715293/2013"}},      "show": true, "fill": "blue", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "Sw/13"}},
      {"N": "antigens", "select": {"vaccine": {"type": "previous", "passage": "cell", "name": "HONG KONG/4801/2014"}},           "show": true, "fill": "blue", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "HK/14"}},
      {"N": "antigens", "select": {"vaccine": {"type": "previous", "passage": "cell", "name": "SINGAPORE/INFIMH-16-0019/2016"}}, "show": true, "fill": "blue", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "Si/16"}},

      {"N": "antigens", "select": {"vaccine": {"type": "previous", "passage": "egg", "name": "TEXAS/50/2012"}},                 "show": false, "fill": "blue", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "TX/12"}},
      {"N": "antigens", "select": {"vaccine": {"type": "previous", "passage": "egg", "name": "SWITZERLAND/9715293/2013"}},      "show": false, "fill": "blue", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "Sw/13"}},
      {"N": "antigens", "select": {"vaccine": {"type": "previous", "passage": "egg", "name": "HONG KONG/4801/2014"}},           "show": true,  "fill": "blue", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "HK/14"}},
      {"N": "antigens", "select": {"vaccine": {"type": "previous", "passage": "egg", "name": "SINGAPORE/INFIMH-16-0019/2016"}}, "show": true,  "fill": "blue", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "Si/16"}},

      {"N": "antigens", "select": {"vaccine": {"type": "previous", "passage": "reassortant", "name": "TEXAS/50/2012"}},                 "show": false, "fill": "blue", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "TX/12"}},
      {"N": "antigens", "select": {"vaccine": {"type": "previous", "passage": "reassortant", "name": "SWITZERLAND/9715293/2013"}},      "show": false, "fill": "blue", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "Sw/13"}},
      {"N": "antigens", "select": {"vaccine": {"type": "previous", "passage": "reassortant", "name": "HONG KONG/4801/2014"}},           "show": false, "fill": "blue", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "HK/14"}},
      {"N": "antigens", "select": {"vaccine": {"type": "previous", "passage": "reassortant", "name": "SINGAPORE/INFIMH-16-0019/2016"}}, "show": true,  "fill": "blue", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "Si/16"}},

      {"N": "antigens", "select": {"vaccine": {"type": "current", "passage": "cell", "name": "SWITZERLAND/8060/2017"}}, "show": true, "fill": "red", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "Sw/17"}},
      {"N": "antigens", "select": {"vaccine": {"type": "current", "passage": "cell", "name": "KANSAS/14/2017"}},        "show": true, "fill": "red", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "KS/17"}},

      {"N": "antigens", "select": {"vaccine": {"type": "current", "passage": "egg", "name": "SWITZERLAND/8060/2017"}}, "show": true, "fill": "red", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "Sw/17"}},
      {"N": "antigens", "select": {"vaccine": {"type": "current", "passage": "egg", "name": "KANSAS/14/2017"}},        "show": true, "fill": "red", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "KS/17"}},

      {"N": "antigens", "select": {"vaccine": {"type": "current", "passage": "reassortant", "name": "SWITZERLAND/8060/2017"}}, "show": true, "fill": "green", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "Sw/17"}},
      {"N": "antigens", "select": {"vaccine": {"type": "current", "passage": "reassortant", "name": "KANSAS/14/2017"}},        "show": true, "fill": "green", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "KS/17"}},

      {"N": "antigens", "select": {"vaccine": {"type": "surrogate", "passage": "cell", "name": "SAITAMA/103/2014"}},    "show": true, "fill": "pink", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "Sa/14"}},
      {"N": "antigens", "select": {"vaccine": {"type": "surrogate", "passage": "cell", "name": "HONG KONG/7295/2014"}}, "show": true, "fill": "pink", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "HK/14"}},

      {"N": "antigens", "select": {"vaccine": {"type": "surrogate", "passage": "egg", "name": "SAITAMA/103/2014"}},    "show": true, "fill": "pink", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "Sa/14"}},
      {"N": "antigens", "select": {"vaccine": {"type": "surrogate", "passage": "egg", "name": "HONG KONG/7295/2014"}}, "show": true, "fill": "pink", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "HK/14"}},

      {"N": "antigens", "select": {"vaccine": {"type": "surrogate", "passage": "reassortant", "name": "SAITAMA/103/2014"}},    "show": true, "fill": "pink", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "Sa/14"}},
      {"N": "antigens", "select": {"vaccine": {"type": "surrogate", "passage": "reassortant", "name": "HONG KONG/7295/2014"}}, "show": true, "fill": "pink", "outline": "black", "report": false, "size": 15, "order": "raise", "label": {"offset": [0, 1], "name_type": "abbreviated_with_passage_type", "size": 12, "display_name": "HK/14"}}
    ]
  }
}
)";

rjson::value settings_builtin_mods()
{
    try {
        return rjson::parse_string(SETTINGS_BUILTIN_MODS, rjson::remove_comments::no);
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: parsing SETTINGS_BUILTIN_MODS: " << err.what() << '\n';
        throw;
    }
}

// ----------------------------------------------------------------------
// see acmacs-map-draw/doc/map-draw-settings.json
//

// static const char* const SETTINGS_HELP_MODS = R"(
// {"N": "antigens", "select": {<select>},
//  "outline": "black", "fill": "red", "aspect": 1.0, "rotation": 0.0,
//  "fill_saturation": 1.0, "fill_brightness": 1.0, "outline_saturation": 1.0, "outline_brightness": 1.0,
//  "size": 1.0, "outline_width": 1.0,
//  "show": true, "shape": "circle|box|triangle",
//  "order": "raise|lower",
//  "label": {"show": true,
//            "display_name": "NAME",
//            "name_type": "full|abbreviated|abbreviated_with_passage_type",
//            "color": "black", "size": 12.0, "offset": [0, 1],
//            "weight": "bold", "slant": "italic", "font_family": "monospace"},
//  "legend": {"show": true, "label": "<TEXT>", "count": true, "replace": false},
//  "report": false, "report_names_threshold": 30}
//
// {"N": "sera", "select": {<select>},
//  "outline": "black", "fill": "red", "aspect": 1.0, "rotation": 0.0,
//  "fill_saturation": 1.0, "fill_brightness": 1.0, "outline_saturation": 1.0, "outline_brightness": 1.0,
//  "size": 1.0, "outline_width": 1.0,
//  "show": true, "shape": "circle|box|triangle",
//  "order": "raise|lower",
//  "label": {"show": true,
//            "display_name": "NAME",
//            "name_type": "full|abbreviated|abbreviated_with_passage_type",
//            "color": "black", "size": 12.0, "offset": [0, 1],
//            "weight": "bold", "slant": "italic", "font_family": "monospace"},
//  "report": false, "report_names_threshold": 30}
//
// {"N": "amino-acids", "pos": [159], "?colors": {"K": "#FF0000", "R": "#0000FF", "X": "grey25"},
//   "color_set": "ana|google", "outline": "black", "outline_width": 1.0,
//   "aspect": 1.0, "rotation": 0.0, "size": 1.0, "order": "raise|lower",
//   "legend": {"count": true},
//   "centroid": false,
//   "report": false},
// {"N": "amino-acids",
//   "groups": [{"pos_aa": ["121K", "144K"], "fill": "cornflowerblue", "?outline": "black"}],
//   "outline": "black", "outline_width": 1.0,
//   "aspect": 1.0, "rotation": 0.0, "size": 1.0, "order": "raise|lower",
//   "legend": {},
//   "report": false},
//
// "all_grey"
// "size_reset"
// "clades"
// "clades_last_6_months"
// "clades_last_12_months"
// "continents"
//   {"N": "continents", "legend": {"type": "continent_map", "offset": [-1, -1], "show": true, "size": 100}, "outline": "black"}
//
// {"N": "legend", "offset": [-10, 10], "show": true,
//  "label_size": 14, "point_size": 10,
//  "data": [{"display_name": "163-del", "outline": "black", "fill": "red"}]}
//
// {"N": "title", "show": true, "offset": [10, 10], "padding": 10, "size": 1,
//  "background": "grey97", "border_color": "black", "border_width": 0.1,
//  "text_color": "black", "text_size": 12, "interline": 2,
//  "font_weight": "normal", "font_slant": "normal", "font_family": "sans serif",
//  "?display_name": ["Line 1 {lab} {assay} {assay_short} {virus_type} {lineage}", "Line 2", "Another line"]}
//
// {"N": "line", "from": [0, 0], "to": [1, 1], "transform": false, "width": 1, "color": "red"}
// {"N": "line", "from_antigen": {<antigen-select>}, "to": [0, 0], "transform": false, "width": 1, "color": "red", "report": true},
// {"N": "line", "from_antigen": {"reference": true}, "to_antigen": {"test": true}, "width": 1, "color": "green", "report": true}
// {"N": "line", "from_antigen": {"reference": true}, "to_serum": {"all": true}, "width": 1, "color": "green", "report": true},
// {"N": "line", "from_serum": {"reference": true}, "to_antigen": {"all": true}, "width": 1, "color": "green", "report":
//
// {"N": "arrow", "to_antigen": {<antigen-select>}, "from": [0, 0], "transform": false, "width": 1, "color": "red", "head_filled": true, "head_color": "magenta", "arrow_width": 10, "report": true},
// {"N": "arrow", "from_antigen": {<antigen-select>}, "to": [0, 0], "transform": false, "width": 1, "color": "red", "head_filled": true, "head_color": "magenta", "arrow_width": 10, "report": true},
// {"N": "arrow", "to_serum": {<serum-select>}, "from": [0, 0], "transform": false, "width": 1, "color": "red", "head_filled": true, "head_color": "magenta", "arrow_width": 10, "report": true},
// {"N": "arrow", "from_serum": {<serum-select>}, "to": [0, 0], "transform": false, "width": 1, "color": "red", "head_filled": true, "head_color": "magenta", "arrow_width": 10, "report": true},
//
// {"N": "rectangle", "c1": [-2, -1.5], "c2": [0.5, 3], "filled": true, "color": "#80FF0000"}
// {"N": "circle", "center": [0, 0], "size": 2, "aspect": 1.0, "rotation": 0, "fill": "#80FFA500", "outline": "#80FF0000", "outline_width": 10}
//
// {"N": "rotate", "degrees": 30, "radians": 1} positive -> counter-clockwise
// {"N": "flip", "direction": "ew|ns"}
// {"N": "viewport", "rel": [-1, 1, -5], "?abs": [-5, -5, 10]}
// {"N": "background", "color": "white"}
// {"N": "border", "color": "black", "line_width": 1}
// {"N": "grid", "color": "grey80", "line_width": 1}
// {"N": "point_scale", "scale": 1, "outline_scale": 1}
//
// {"N": "move_antigens", "select": {"reference": true}, "?to": [5, 5], "?relative": [1, 1], "?to_antigen": {"index": 10}, "?to_serum": {"index": 10},
//   "?flip_over_serum_line": 1, "report": true} // flip_over_serum_line: scale (1 - mirror, 0.1 - close to serum line, 0 - move to serum line)
// {"N": "move_sera", "select": {"all": true}, "?to": [5, 5], "?relative": [1, 1], "?to_antigen": {"index": 10}, "to_serum": {"index": 1}, "report": true}
// {"N": "move_antigens_stress", "select": {"reference": true}, "?to": [5, 5], "?relative": [1, 1], "?fill": "pink", "?outline": "grey", "?order": "raise", "?size": 1,
//   "report": true}
//
// {"N": "serum_circle", "serum": {"index": 0}, "?antigen": {"index": 0}, "report": true,
//  "?fold": 2.0, "? fold": "2 - 4fold, 3 - 8fold",
//  "empirical":   {"fill": "#C08080FF", "outline": "#4040FF", "outline_width": 2, "?outline_dash": "dash2", "?angle_degrees": [0, 30], "?radius_line_dash": "dash2", "?radius_line_color": "red", "?radius_line_width": 1, "show": true},
//  "theoretical": {"fill": "#C08080FF", "outline": "#0000C0", "outline_width": 2, "?outline_dash": "dash2", "?angle_degrees": [0, 30], "?radius_line_dash": "dash2", "?radius_line_color": "red", "?radius_line_width": 1, "show": true},
//  "fallback":    {"fill": "#C08080FF", "outline": "#0000C0", "outline_width": 2, "outline_dash": "dash3",  "?angle_degrees": [0, 30], "?radius_line_dash": "dash2", "?radius_line_color": "red", "?radius_line_width": 1, "radius": 3, "show": true},
//  "mark_serum":   {"fill": "lightblue", "outline": "black", "order": "raise", "label": {"name_type": "full", "offset": [0, 1.2], "color": "black", "size": 12}},
//  "mark_antigen": {"fill": "lightblue", "outline": "black", "order": "raise", "label": {"name_type": "full", "offset": [0, 1.2], "color": "black", "size": 12}}}
//
// {obsolete! "N": "serum_circle", "serum": {"index": 0}, "?antigen": {"index": 0}, "?homologous_titer": "1280", "report": true,
//  "type": "empirical (default) | theoretical",
//  "circle": {"fill": "#C08080FF", "outline": "blue", "outline_width": 2, "angle_degrees": [0, 30], "radius_line_dash": "dash2", "?radius_line_color": "red", "?radius_line_width": 1},
//  "mark_serum": {"fill": "lightblue", "outline": "black", "order": "raise", "label": {"name_type": "full", "offset": [0, 1.2], "color": "black", "size": 12}},
//  "mark_antigen": {"fill": "lightblue", "outline": "black", "order": "raise", "label": {"name_type": "full", "offset": [0, 1.2], "color": "black", "size": 12}}}
//
// {"N": "serum_coverage", "serum": {<select>}, "?antigen": {<select>}, "?homologous_titer": "1280", "report": true,
//  "mark_serum": {"fill": "red", "outline": "black", "order": "raise", "label": {"name_type": "full", "offset": [0, 1.2], "color": "black", "size": 12, "weight": "bold"}},
//  "?fold": 2.0, "? fold": "2 - 4fold, 3 - 8fold",
//  "within_4fold": {"outline": "pink", "outline_width": 3, "order": "raise"},
//  "outside_4fold": {"fill": "grey50", "outline": "black", "order": "raise"}}
//
// {"N": "serum_coverage_circle", "serum": {<select>}, "?antigen": {<select>}, "?homologous_titer": "1280", "report": true,
//  "mark_serum": {"fill": "red", "outline": "black", "order": "raise", "label": {"name_type": "full", "offset": [0, 1.2], "color": "black", "size": 12, "weight": "bold"}},
//  "empirical": {"show": true, "fill": "#C0FF8080", "outline": "red", "outline_width": 2, "?outline_dash": "dash2", "angle_degrees": [0, 30], "radius_line_dash": "dash2", "?radius_line_color": "red", "?radius_line_width": 1},
//  "theoretical": {"show": true, "fill": "#C08080FF", "outline": "blue", "outline_width": 2, "?outline_dash": "dash2", "angle_degrees": [0, 30], "radius_line_dash": "dash2", "?radius_line_color": "red", "?radius_line_width": 1},
//  "?fold": 2.0, "? fold": "2 - 4fold, 3 - 8fold",
//  "within_4fold": {"outline": "pink", "outline_width": 3, "order": "raise"},
//  "outside_4fold": {"fill": "grey50", "outline": "black", "order": "raise"}}
//
// {"N": "procrustes_arrows", "chart": "secondary.ace", "projection": 0, "match": "auto", "?match": "auto, strict, relaxed, ignored", "scaling": false, "report": false,
//  "?subset": "all, sera, antigens, reference, test", "?subset_antigens": {"clade": "2a1"}, "?subset_sera": {"clade": "2a1"},
//  "threshold": 0.005, "?threshold": "do not show arrows shorter than this value in units",
//  "arrow": {"color": "black", "head_color": "black", "head_filled": true, "line_width": 1, "arrow_width": 5}}
//
// {"N": "serum_line", "color": "red", "line_width": 1}
//
// {"N": "connection_lines", "antigens": {<select>}, "sera": {<select>}, "color": "grey", "line_width": 1},
// {"N": "error_lines", "antigens": {<select>}, "sera": {<select>}, "line_width": 1},
// {"N": "color_by_number_of_connection_lines", "antigens": {<select>}, "sera": {<select>}, "start": "", "end": ""},
//
// {"N": "blobs", "select": {<select-antigens>}, "stress_diff": 0.5, "number_of_drections": 36, "stress_diff_precision": 1e-5,
//  "fill": "transparent", "color": "pink", "line_width": 1, "report": false}
//
// --------------------------------------------------
//
// SSM time series:
//     {"N": "title", "background": "transparent", "border_width": 0, "text_size": 24, "font_weight": "bold", "display_name": ["CDC H3 HI March 2017"]},
//     "continents",
//     {"N": "antigens", "select": "reference", "outline": "grey80", "fill": "transparent"},
//     {"N": "antigens", "select": "test", "show": false},
//     {"N": "antigens", "select": {"test": true, "date_range": ["2017-03-01", "2017-04-01"]}, "size": 8, "order": "raise", "show": true},
//     {"N": "vaccines", "size": 25, "report": false},
//     {"N": "point_scale", "scale": 2.5, "outline_scale": 1},
//     {"N": "viewport", "rel": [6.5, 7.5, -11]},
//
// --------------------------------------------------
//
// TODO:
// path path:[], line_width: color: close: false filled: false
//
// )";
//
// const char* settings_help_mods()
// {
//     return SETTINGS_HELP_MODS;
// }
//
// // ----------------------------------------------------------------------
//
// static const char* const SETTINGS_HELP_SELECT = R"(
// Antigens:
// ---------
// "all",
// "reference",
// "test",
// "egg", "cell", "reassortant",
//   {"passage": "egg"}, {"passage": "cell"}, {"passage": "reassortant"},
// {"date_range": ["2016-01-01", "2016-09-01"]},
//   {"date_range": ["", "2016-09-01"]},
//   {"date_range": ["2016-01-01", ""]},
//   {"older_than_days": 365},
//   {"younger_than_days": 365},
// {"index": 11},
//   {"indexes": [55, 66]},
// {"country": "sweden"},
//   {"continent": "europe"},
// "sequenced",
//   "not_sequenced",
//   {"clade": "3C3a"},
//   {"amino_acid": {"aa": "T", "pos": 141}},
//   {"amino_acid": ["144R", "93A", "!160K"]},
// {"name": "SWITZERLAND/9715293/2013", "exclude_distinct": true},
//   {"name": "SWITZERLAND/9715293/2013", "passage": "reassortant"},
//   {"full_name": "A(H1N1)/MICHIGAN/2/2009 MDCK1", "no": 0},
// "vaccine",
//   {"vaccine": {"type": "previous", "no": 0, "passage": "egg", "name": "SWITZERLAND"}},
// {"in_rectangle": {"c1": [0.0, 0.0], "c2": [1.0, 1.0]}}, {"in_circle": {"center": [2.0, 2.0], "radius": 5.0}}
// {"lab": "CDC"} - returns empty index list for other labs
// {"subtype": "H1 A(H1N1) H3 A(H3N2) B BV BVIC BY BYAM"}
// {"outlier": 1.0} // threshold in units (distance from centroid of pre-selected points), must be after other select args, e.g. after "clade"
// {"table": "20170216"}
//   {"table": "MELB:HI:turkey:20170216"}
// {"relative_to_serum_line": {"distance_min": 0, "distance_max": 10000, "direction": 1}} // direction: 1, -1, 0
// {"titrated_against_sera": {<select>}}
//
// Sera:
// -----
// "all",
// {"serum_id": "CDC 2016-003"},
// {"index": 11}, {"indexes": [55, 66]},
// {"country": "sweden"}, {"continent": "europe"},
// {"name": "SWITZERLAND/9715293/2013"},
// {"full_name": "A(H1N1)/MICHIGAN/2/2009 CDC 2015-121"},
// {"table": "20170216"}
//   {"table": "MELB:HI:turkey:20170216"}
// {"clade": "3C3a"} // via seqdb.clades_for_name()
// {"titrated_against_antigens": {<select>}}
//
//
// )";
//
// const char* settings_help_select()
// {
//     return SETTINGS_HELP_SELECT;
// }
//
// ----------------------------------------------------------------------

// static const char* const DEFAULT_SETTINGS_JSON =
// {
//   "mods": {
//     "all_red": [
//       {"N": "antigens", "select": "test", "outline": "black", "fill": "red"},
//       {"N": "antigens", "select": "reference", "outline": "red", "fill": "transparent"}
//     ]
//   },
//   "clade_fill": {
//     "?": "H3",
//     "3C3": "cornflowerblue",
//     "3C2a": "red",
//     "3C2a1": "darkred",
//     "3C3a": "green",
//     "3C3b": "blue",

//     "?": "H1pdm",
//     "6B1": "blue",
//     "6B2": "red",

//     "?": "B/Yam",
//     "Y2": "cornflowerblue",
//     "Y3": "red",

//     "?": "B/Vic",
//     "1": "blue",
//     "1A": "cornflowerblue",
//     "1B": "red",
//     "2DEL2017": "#000080",
//     "3DEL2017": "#46f0f0"
//   }
// }

// ----------------------------------------------------------------------

// flip
//   value: [0, 1]
//   ew
//   ns
// rotate_degrees angle:
// rotate_radians angle:
// viewport value:
// point_scale
// {"N": "background", "color": "white"},
// {"N": "grid", "color": "grey80", "line_width": 1},
// {"N": "border", "color": "black", "line_width": 1},
// title size: background: border_color: border_width: display_name: []
// legend data:[{"label": {<label-data>} "display_name": "163-del", "outline": "black", "fill": "#03569b"}] "offset": [-10, -10], "show": True, "size": 50, "label_size": 8, "point_size": 5
//
// antigens
//   show
//   size, shape, fill, outline, outline_width, aspect, rotation, raise_ (order: raise, lower, no-change), report, report_names_threshold
//   label: name_type, offset, display_name, color, size, weight, slant, font_family
// {"N": "aa_substitutions", "positions": [159], "legend": {"show": True, "offset": [-10, -10], "point_count": True, "background": "grey99", "border_color": "black", "border_width": 0.1, "label_size": 12, "point_size": 8}},
// {"N": "aa_substitution_groups", "groups": [{"pos_aa": ["121K", "144K"], "color": "cornflowerblue"}]},
//
// sera
//   size, shape, fill, outline, outline_width, aspect, rotation, raise_ (order: raise, lower, no-change), report, report_names_threshold
//   label: name_type, offset, display_name, color, size, weight, slant, font_family
//   serum_circle
//   serum_coverage
//
//  arrow
//  line
//  rectangle corner1: corner2:
//  circle  center: radius: aspect: rotation:
//
// Application
//  lab
//  labs
//  lineage
//
// Derived:
//  all-grey
//  continents

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
