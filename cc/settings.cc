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
      {"N": "antigens", "select": "test", "size": 5.0},
      {"N": "antigens", "select": "reference", "size": 8.0},
      {"N": "sera", "select": "all", "size": 6.5}
    ],
    "clades": [
      {"N": "antigens", "select": {"sequenced": true},   "outline": "black", "fill": "yellow", "legend": {"label": "sequenced", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "3C.3"}, "fill": "cornflowerblue", "outline": "black", "legend": {"label": "3C.3", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3A"},   "fill": "green",          "outline": "black", "legend": {"label": "3a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3B"},   "fill": "blue",           "outline": "black", "legend": {"label": "3b", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2A"},   "fill": "#FF0000",        "outline": "black", "legend": {"label": "2a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2A1"},  "fill": "#ffab91",        "outline": "black", "legend": {"label": "2a1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2A1A"}, "fill": "#E040FB",        "outline": "black", "legend": {"label": "2a1a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2A1B"}, "fill": "#CD5C5C",        "outline": "black", "legend": {"label": "2a1b", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2A2"},  "fill": "#EEB422",        "outline": "black", "legend": {"label": "2a2", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "6B"},   "outline": "black", "fill": "cornflowerblue", "legend": {"label": "6B", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "6B1"},   "outline": "black", "fill": "blue", "legend": {"label": "6B1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "6B2"},   "outline": "black", "fill": "red", "legend": {"label": "6B2", "count": true}, "order": "raise", "report": true, "report_names_threshold": 10},

      {"N": "antigens", "select": {"clade": "V1", "subtype": "BVIC"}, "outline": "black", "fill": "blue", "legend": {"label": "V1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "V1A", "subtype": "BVIC"}, "outline": "black", "fill": "cornflowerblue", "legend": {"label": "V1A", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "V1B", "subtype": "BVIC"}, "outline": "black", "fill": "red", "legend": {"label": "V1B", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "DEL2017", "subtype": "BVIC"}, "outline": "black", "fill": "#DE8244", "legend": {"label": "Del mutants", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "TRIPLEDEL2017", "subtype": "BVIC"}, "outline": "black", "fill": "#BF3EFF", "legend": {"label": "Triple del mutants", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "Y2", "subtype": "BYAM"}, "outline": "black", "fill": "cornflowerblue", "legend": {"label": "Y2", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "Y3", "subtype": "BYAM"}, "outline": "black", "fill": "red", "legend": {"label": "Y3", "count": true}, "order": "raise", "report": true}
    ],
    "clades_last_6_months": [
      {"N": "antigens", "select": {"clade": "3C.3", "younger_than_days": 183},   "outline": "black", "fill": "cornflowerblue", "legend": {"label": "3C.3", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3A", "younger_than_days": 183},  "outline": "black", "fill": "green", "legend": {"label": "3a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3B", "younger_than_days": 183},  "outline": "black", "fill": "blue", "legend": {"label": "3b", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2A", "younger_than_days": 183},  "outline": "black", "fill": "red", "legend": {"label": "2a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2A1", "younger_than_days": 183}, "outline": "black", "fill": "#ffab91", "legend": {"label": "2a1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2A1A", "younger_than_days": 183}, "outline": "black", "fill": "#E040FB", "legend": {"label": "2a1a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2A1B", "younger_than_days": 183}, "outline": "black", "fill": "#CD5C5C", "legend": {"label": "2a1b", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2A2", "younger_than_days": 183}, "outline": "black", "fill": "#EEB422", "legend": {"label": "2a2", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "6B", "younger_than_days": 183},   "outline": "black", "fill": "cornflowerblue", "legend": {"label": "6B", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "6B1", "younger_than_days": 183},   "outline": "black", "fill": "blue", "legend": {"label": "6B1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "6B2", "younger_than_days": 183},   "outline": "black", "fill": "red", "legend": {"label": "6B2", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "V1", "younger_than_days": 183, "subtype": "BVIC"}, "outline": "black", "fill": "blue", "legend": {"label": "V1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "V1A", "younger_than_days": 183, "subtype": "BVIC"}, "outline": "black", "fill": "cornflowerblue", "legend": {"label": "V1A", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "V1B", "younger_than_days": 183, "subtype": "BVIC"}, "outline": "black", "fill": "red", "legend": {"label": "V1B", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "DEL2017", "younger_than_days": 183, "subtype": "BVIC"}, "outline": "black", "fill": "#DE8244", "legend": {"label": "Del mutants", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "TRIPLEDEL2017", "younger_than_days": 183, "subtype": "BVIC"}, "outline": "black", "fill": "#BF3EFF", "legend": {"label": "Triple del mutants", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "Y2", "younger_than_days": 183, "subtype": "BYAM"}, "outline": "black", "fill": "cornflowerblue", "legend": {"label": "Y2", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "Y3", "younger_than_days": 183, "subtype": "BYAM"}, "outline": "black", "fill": "red", "legend": {"label": "Y3", "count": true}, "order": "raise", "report": true}
    ],
    "clades_last_12_months": [
      {"N": "antigens", "select": {"clade": "3C.3", "younger_than_days": 365},   "outline": "black", "fill": "cornflowerblue", "legend": {"label": "3C.3", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3A", "younger_than_days": 365},  "outline": "black", "fill": "green", "legend": {"label": "3a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3B", "younger_than_days": 365},  "outline": "black", "fill": "blue", "legend": {"label": "3b", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2A", "younger_than_days": 365},  "outline": "black", "fill": "red", "legend": {"label": "2a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2A1", "younger_than_days": 365}, "outline": "black", "fill": "#ffab91", "legend": {"label": "2a1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2A1A", "younger_than_days": 365}, "outline": "black", "fill": "#E040FB", "legend": {"label": "2a1a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2A1B", "younger_than_days": 365}, "outline": "black", "fill": "#CD5C5C", "legend": {"label": "2a1b", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2A2", "younger_than_days": 365}, "outline": "black", "fill": "#EEB422", "legend": {"label": "2a2", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "6B", "younger_than_days": 365},   "outline": "black", "fill": "cornflowerblue", "legend": {"label": "6B", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "6B1", "younger_than_days": 365},   "outline": "black", "fill": "blue", "legend": {"label": "6B1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "6B2", "younger_than_days": 365},   "outline": "black", "fill": "red", "legend": {"label": "6B2", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "V1", "younger_than_days": 365, "subtype": "BVIC"}, "outline": "black", "fill": "blue", "legend": {"label": "V1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "V1A", "younger_than_days": 365, "subtype": "BVIC"}, "outline": "black", "fill": "cornflowerblue", "legend": {"label": "V1A", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "V1B", "younger_than_days": 365, "subtype": "BVIC"}, "outline": "black", "fill": "red", "legend": {"label": "V1B", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "DEL2017", "younger_than_days": 365, "subtype": "BVIC"}, "outline": "black", "fill": "#DE8244", "legend": {"label": "Del mutants", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "TRIPLEDEL2017", "younger_than_days": 365, "subtype": "BVIC"}, "outline": "black", "fill": "#BF3EFF", "legend": {"label": "Triple del mutants", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "Y2", "younger_than_days": 365, "subtype": "BYAM"}, "outline": "black", "fill": "cornflowerblue", "legend": {"label": "Y2", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "Y3", "younger_than_days": 365, "subtype": "BYAM"}, "outline": "black", "fill": "red", "legend": {"label": "Y3", "count": true}, "order": "raise", "report": true}
    ],
    "clades_light": [
      {"N": "antigens", "select": {"clade": "3C.3"},   "outline": "#BFBFBF", "fill": "#8F9DBF", "legend": {"label": "3C.3", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3A"},  "outline": "#BFBFBF", "fill": "#BFFFBF", "legend": {"label": "3a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3B"},  "outline": "#BFBFBF", "fill": "#BFBFFF", "legend": {"label": "3b", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2A"},  "outline": "#BFBFBF", "fill": "#FFBFBF", "legend": {"label": "2a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2A1"},    "outline": "#BFBFBF", "fill": "#fbe9e7", "legend": {"label": "2a1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2A1A"},    "outline": "#BFBFBF", "fill": "#e1bee7", "legend": {"label": "2a1a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2A1B"},    "outline": "#BFBFBF", "fill": "#AF8585", "legend": {"label": "2a1b", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "2A2"},    "outline": "#BFBFBF", "fill": "#EED69A", "legend": {"label": "2a2", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "6B"},    "outline": "#BFBFBF", "fill": "#8F9DBF", "legend": {"label": "6B", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "6B1"},   "outline": "#BFBFBF", "fill": "#BFBFFF", "legend": {"label": "6B1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "6B2"},   "outline": "#BFBFBF", "fill": "#FFBFBF", "legend": {"label": "6B2", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "1", "subtype": "BVIC"}, "outline": "#BFBFBF", "fill": "#BFBFFF", "legend": {"label": "1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "1A", "subtype": "BVIC"}, "outline": "#BFBFBF", "fill": "#8F9DBF", "legend": {"label": "1A", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "1B", "subtype": "BVIC"}, "outline": "#BFBFBF", "fill": "#FFBFBF", "legend": {"label": "1B", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "DEL2017", "subtype": "BVIC"}, "outline": "#BFBFBF", "fill": "#DE8244", "legend": {"label": "Del mutants", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "TRIPLEDEL2017", "subtype": "BVIC"}, "outline": "#BFBFBF", "fill": "#BF3EFF", "legend": {"label": "Triple del mutants", "count": true}, "order": "raise", "report": true},

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

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
