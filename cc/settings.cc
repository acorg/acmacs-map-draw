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
  "mods": {
    "all_grey": [
      {"N": "antigens", "select": "test", "outline": "grey80", "fill": "grey80"},
      {"N": "antigens", "select": "reference", "outline": "grey80", "fill": "transparent"},
      {"N": "sera", "select": "all", "outline": "grey80", "fill": "transparent"}
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
      {"N": "antigens", "select": {"clade": "3C3"},   "outline": "black", "fill": "cornflowerblue", "legend": {"label": "3C3", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C3A"},  "outline": "black", "fill": "green", "legend": {"label": "3C3a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C3B"},  "outline": "black", "fill": "blue", "legend": {"label": "3C3b", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C2A"},  "outline": "black", "fill": "red", "legend": {"label": "3C2a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C2A1"}, "outline": "black", "fill": "darkred", "legend": {"label": "3C2a1", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "6B1"},   "outline": "black", "fill": "blue", "legend": {"label": "6B1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "6B2"},   "outline": "black", "fill": "red", "legend": {"label": "6B2", "count": true}, "order": "raise", "report": true, "report_names_threshold": 10},

      {"N": "antigens", "select": {"clade": "1", "subtype": "BVIC"}, "outline": "black", "fill": "blue", "legend": {"label": "1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "1A", "subtype": "BVIC"}, "outline": "black", "fill": "cornflowerblue", "legend": {"label": "1A", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "1B", "subtype": "BVIC"}, "outline": "black", "fill": "red", "legend": {"label": "1B", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "DEL2017", "subtype": "BVIC"}, "outline": "black", "fill": "#DE8244", "legend": {"label": "Del mutants", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "TRIPLEDEL2017", "subtype": "BVIC"}, "outline": "black", "fill": "#BF3EFF", "legend": {"label": "Triple del mutants", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "Y2", "subtype": "BYAM"}, "outline": "black", "fill": "cornflowerblue", "legend": {"label": "Y2", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "Y3", "subtype": "BYAM"}, "outline": "black", "fill": "red", "legend": {"label": "Y3", "count": true}, "order": "raise", "report": true}
    ],
    "clades_last_6_months": [
      {"N": "antigens", "select": {"clade": "3C3", "younger_than_days": 183},   "outline": "black", "fill": "cornflowerblue", "legend": {"label": "3C3", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C3A", "younger_than_days": 183},  "outline": "black", "fill": "green", "legend": {"label": "3C3a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C3B", "younger_than_days": 183},  "outline": "black", "fill": "blue", "legend": {"label": "3C3b", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C2A", "younger_than_days": 183},  "outline": "black", "fill": "red", "legend": {"label": "3C2a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C2A1", "younger_than_days": 183}, "outline": "black", "fill": "darkred", "legend": {"label": "3C2a1", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "6B1", "younger_than_days": 183},   "outline": "black", "fill": "blue", "legend": {"label": "6B1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "6B2", "younger_than_days": 183},   "outline": "black", "fill": "red", "legend": {"label": "6B2", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "1", "younger_than_days": 183, "subtype": "BVIC"}, "outline": "black", "fill": "blue", "legend": {"label": "1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "1A", "younger_than_days": 183, "subtype": "BVIC"}, "outline": "black", "fill": "cornflowerblue", "legend": {"label": "1A", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "1B", "younger_than_days": 183, "subtype": "BVIC"}, "outline": "black", "fill": "red", "legend": {"label": "1B", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "DEL2017", "younger_than_days": 183, "subtype": "BVIC"}, "outline": "black", "fill": "#DE8244", "legend": {"label": "Del mutants", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "TRIPLEDEL2017", "younger_than_days": 183, "subtype": "BVIC"}, "outline": "black", "fill": "#BF3EFF", "legend": {"label": "Triple del mutants", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "Y2", "younger_than_days": 183, "subtype": "BYAM"}, "outline": "black", "fill": "cornflowerblue", "legend": {"label": "Y2", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "Y3", "younger_than_days": 183, "subtype": "BYAM"}, "outline": "black", "fill": "red", "legend": {"label": "Y3", "count": true}, "order": "raise", "report": true}
    ],
    "clades_last_12_months": [
      {"N": "antigens", "select": {"clade": "3C3", "younger_than_days": 365},   "outline": "black", "fill": "cornflowerblue", "legend": {"label": "3C3", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C3A", "younger_than_days": 365},  "outline": "black", "fill": "green", "legend": {"label": "3C3a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C3B", "younger_than_days": 365},  "outline": "black", "fill": "blue", "legend": {"label": "3C3b", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C2A", "younger_than_days": 365},  "outline": "black", "fill": "red", "legend": {"label": "3C2a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C2A1", "younger_than_days": 365}, "outline": "black", "fill": "darkred", "legend": {"label": "3C2a1", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "6B1", "younger_than_days": 365},   "outline": "black", "fill": "blue", "legend": {"label": "6B1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "6B2", "younger_than_days": 365},   "outline": "black", "fill": "red", "legend": {"label": "6B2", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "1", "younger_than_days": 365, "subtype": "BVIC"}, "outline": "black", "fill": "blue", "legend": {"label": "1", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "1A", "younger_than_days": 365, "subtype": "BVIC"}, "outline": "black", "fill": "cornflowerblue", "legend": {"label": "1A", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "1B", "younger_than_days": 365, "subtype": "BVIC"}, "outline": "black", "fill": "red", "legend": {"label": "1B", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "DEL2017", "younger_than_days": 365, "subtype": "BVIC"}, "outline": "black", "fill": "#DE8244", "legend": {"label": "Del mutants", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "TRIPLEDEL2017", "younger_than_days": 365, "subtype": "BVIC"}, "outline": "black", "fill": "#BF3EFF", "legend": {"label": "Triple del mutants", "count": true}, "order": "raise", "report": true},

      {"N": "antigens", "select": {"clade": "Y2", "younger_than_days": 365, "subtype": "BYAM"}, "outline": "black", "fill": "cornflowerblue", "legend": {"label": "Y2", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "Y3", "younger_than_days": 365, "subtype": "BYAM"}, "outline": "black", "fill": "red", "legend": {"label": "Y3", "count": true}, "order": "raise", "report": true}
    ],
    "clades_light": [
      {"N": "antigens", "select": {"clade": "3C3"},   "outline": "#BFBFBF", "fill": "#8F9DBF", "legend": {"label": "3C3", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C3A"},  "outline": "#BFBFBF", "fill": "#BFFFBF", "legend": {"label": "3C3a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C3B"},  "outline": "#BFBFBF", "fill": "#BFBFFF", "legend": {"label": "3C3b", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C2A"},  "outline": "#BFBFBF", "fill": "#FFBFBF", "legend": {"label": "3C2a", "count": true}, "order": "raise", "report": true},
      {"N": "antigens", "select": {"clade": "3C2A1"}, "outline": "#BFBFBF", "fill": "#AF8585", "legend": {"label": "3C2a1", "count": true}, "order": "raise", "report": true},

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
    "egg": [
      {"N": "antigens", "select": {"passage": "egg"}, "aspect": 0.75},
      {"N": "antigens", "select": {"passage": "reassortant"}, "aspect": 0.75, "rotation": 0.5}
    ],
    "continents": [
      {"N": "antigens", "select": {"continent": "europe"}, "fill": "green", "outline": "black", "report": true},
      {"N": "antigens", "select": {"continent": "central-america"}, "fill": "#AAF9FF", "outline": "black", "report": true},
      {"N": "antigens", "select": {"continent": "middle-east"}, "fill": "#8000FF", "outline": "black", "report": true},
      {"N": "antigens", "select": {"continent": "north-america"}, "fill": "blue4", "outline": "black", "report": true},
      {"N": "antigens", "select": {"continent": "africa"}, "fill": "darkorange1", "outline": "black", "report": true},
      {"N": "antigens", "select": {"continent": "asia"}, "fill": "red", "outline": "black", "report": true},
      {"N": "antigens", "select": {"continent": "russia"}, "fill": "maroon", "outline": "black", "report": true},
      {"N": "antigens", "select": {"continent": "australia-oceania"}, "fill": "hotpink", "outline": "black", "report": true},
      {"N": "antigens", "select": {"continent": "south-america"}, "fill": "turquoise", "outline": "black", "report": true},
      {"N": "antigens", "select": {"continent": "antarctica"}, "fill": "grey50", "outline": "black", "report": true}
    ],
    "vaccines": [
      {"N": "antigens", "select": {"vaccine": {"type": "previous"}}, "report": false, "outline": "black", "fill": "blue", "size": 15, "show": true, "order": "raise"},
      {"N": "antigens", "select": {"vaccine": {"type": "current", "passage": "cell"}}, "report": false, "outline": "black", "fill": "red", "size": 15, "show": true, "order": "raise"},
      {"N": "antigens", "select": {"vaccine": {"type": "current", "passage": "egg"}}, "report": false, "outline": "black", "fill": "red", "size": 15, "show": true, "order": "raise"},
      {"N": "antigens", "select": {"vaccine": {"type": "current", "passage": "reassortant"}}, "report": false, "outline": "black", "fill": "green", "size": 15, "show": true, "order": "raise"},
      {"N": "antigens", "select": {"vaccine": {"type": "surrogate"}}, "report": false, "outline": "black", "fill": "pink", "size": 15, "show": true, "order": "raise"}
    ]
  }
}
)";

rjson::value settings_builtin_mods()
{
    try {
        return rjson::parse_string(SETTINGS_BUILTIN_MODS, rjson::remove_comments::No);
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: parsing SETTINGS_BUILTIN_MODS: " << err.what() << '\n';
        throw;
    }
}

// ----------------------------------------------------------------------

static const char* const SETTINGS_HELP_MODS = R"(
{"N": "antigens", "select": {<select>},
 "outline": "black", "fill": "red", "aspect": 1.0, "rotation": 0.0,
 "size": 1.0, "outline_width": 1.0,
 "show": true, "shape": "circle|box|triangle",
 "order": "raise|lower",
 "label": {"show": true,
           "display_name": "NAME",
           "name_type": "full|abbreviated|abbreviated_with_passage_type",
           "color": "black", "size": 12.0, "offset": [0, 1],
           "weight": "bold", "slant": "italic", "font_family": "monospace"},
 "legend": {"show": true, "label": "<TEXT>", "count": true},
 "report": false, "report_names_threshold": 10}

{"N": "sera", "select": {<select>},
 "outline": "black", "fill": "red", "aspect": 1.0, "rotation": 0.0,
 "size": 1.0, "outline_width": 1.0,
 "show": true, "shape": "circle|box|triangle",
 "order": "raise|lower",
 "label": {"show": true,
           "display_name": "NAME",
           "name_type": "full|abbreviated|abbreviated_with_passage_type",
           "color": "black", "size": 12.0, "offset": [0, 1],
           "weight": "bold", "slant": "italic", "font_family": "monospace"},
 "report": false, "report_names_threshold": 10}

{"N": "amino-acids", "pos": [159], "?colors": ["#FF0000", "#0000FF"],
  "?X_color": "grey25", "outline": "black", "outline_width": 1.0,
  "aspect": 1.0, "rotation": 0.0, "size": 1.0, "order": "raise|lower",
  "legend": {},
  "report": false},
{"N": "amino-acids",
  "groups": [{"pos": ["121K", "144K"], "fill": "cornflowerblue" "?outline": "black"}],
  "outline": "black", "outline_width": 1.0,
  "aspect": 1.0, "rotation": 0.0, "size": 1.0, "order": "raise|lower",
  "legend": {},
  "report": false},

"all_grey"
"size_reset"
"clades"
"clades_last_6_months"
"clades_last_12_months"
"continents"

{"N": "legend", "offset": [-10, 10], "show": true, "size": 50,
 "label_size": 14, "point_size": 10,
 "data": [{"display_name": "163-del", "outline": "black", "fill": "red"}]}

{"N": "title", "show": true, "offset": [10, 10], "padding": 10, "size": 1,
 "background": "grey97", "border_color": "black", "border_width": 0.1,
 "text_color": "black", "text_size": 12, "interline": 2,
 "font_weight": "normal", "font_slant": "normal", "font_family": "sans serif",
 "?display_name": ["Line 1", "Line 2", "Another line"]}

{"N": "line", "from": [0, 0], "to": [1, 1], "width": 1, "color": "red"}
{"N": "line", "from_antigen": {<antigen-select>}, "to": [0, 0], "width": 1, "color": "red", "report": true},
{"N": "line", "from_antigen": {"reference": true}, "to_antigen": {"test": true}, "width": 1, "color": "green", "report": true}
{"N": "line", "from_antigen": {"reference": true}, "to_serum": {"all": true}, "width": 1, "color": "green", "report": true},
{"N": "line", "from_serum": {"reference": true}, "to_antigen": {"all": true}, "width": 1, "color": "green", "report":

{"N": "arrow", "to_antigen": {<antigen-select>}, "from": [0, 0], "width": 1, "color": "red", "head_filled": true, "head_color": "magenta", "arrow_width": 10, "report": true},

{"N": "rectangle", "corner1": [-2, -1.5], "corner2": [0.5, 3], "filled": true, "color": "#80FF0000"}
{"N": "circle", "center": [0, 0], "size": 2, "aspect": 1.0, "rotation": 0, "fill": "#80FFA500", "outline": "#80FF0000", "outline_width": 10}

{"N": "rotate", "degrees": 30, "radians": 1} positive -> counter-clockwise
{"N": "flip", "direction": "ew|ns"}
{"N": "viewport", "rel": [-1, 1, -5], "?abs": [-5, -5, 10]}
{"N": "background", "color": "white"}
{"N": "border", "color": "black", "line_width": 1}
{"N": "grid", "color": "grey80", "line_width": 1}
{"N": "point_scale", "scale": 1, "outline_scale": 1}

{"N": "move_antigens", "select": {"reference": true}, "?to": [5, 5], "?to_antigen": {"index": 10}, "to_serum": {"index": 10}, "report": true}
{"N": "move_sera", "select": {"all": true}, "?to": [5, 5], "?to_antigen": {"index": 10}, "to_serum": {"index": 1}, "report": true}

{"N": "serum_circle", "serum": {"index": 0}, "?antigen": {"index": 0}, "report": true,
 "circle": {"fill": "#C08080FF", "outline": "blue", "outline_width": 2, "angle_degrees": [0, 30], "radius_line_dash": "dash2", "?radius_line_color": "red", "?radius_line_width": 1},
 "mark_serum": {"fill": "lightblue", "outline": "black", "order": "raise", "label": {"name_type": "full", "offset": [0, 1.2], "color": "black", "size": 12}},
 "mark_antigen": {"fill": "lightblue", "outline": "black", "order": "raise", "label": {"name_type": "full", "offset": [0, 1.2], "color": "black", "size": 12}}}

{"N": "serum_coverage", "serum": {"index": 0}, "?antigen": {"index": 1}, "report": true,
 "mark_serum": {"fill": "red", "outline": "black", "order": "raise", "label": {"name_type": "full", "offset": [0, 1.2], "color": "black", "size": 12, "weight": "bold"}},
 "within_4fold": {"outline": "pink", "outline_width": 3, "order": "raise"},
 "outside_4fold": {"fill": "grey50", "outline": "black", "order": "raise"}}

--------------------------------------------------

SSM time series:
    {"N": "title", "background": "transparent", "border_width": 0, "text_size": 24, "font_weight": "bold", "display_name": ["CDC H3 HI March 2017"]},
    "continents",
    {"N": "antigens", "select": "reference", "outline": "grey80", "fill": "transparent"},
    {"N": "antigens", "select": "test", "show": false},
    {"N": "antigens", "select": {"test": true, "date_range": ["2017-03-01", "2017-04-01"]}, "size": 8, "order": "raise", "show": true},
    {"N": "vaccines", "size": 25, "report": false},
    {"N": "point_scale", "scale": 2.5, "outline_scale": 1},
    {"N": "viewport", "rel": [6.5, 7.5, -11]},

--------------------------------------------------

TODO:
path path:[], line_width: color: close: false filled: false

)";

const char* settings_help_mods()
{
    return SETTINGS_HELP_MODS;
}

// ----------------------------------------------------------------------

static const char* const SETTINGS_HELP_SELECT = R"(
Antigens:
---------
"all",
"reference",
"test",
"egg", "cell", "reassortant",
  {"passage": "egg"}, {"passage": "cell"}, {"passage": "reassortant"},
{"date_range": ["2016-01-01", "2016-09-01"]},
  {"date_range": ["", "2016-09-01"]},
  {"date_range": ["2016-01-01", ""]},
  {"older_than_days": 365},
  {"younger_than_days": 365},
{"index": 11},
  {"indices": [55, 66]},
{"country": "sweden"},
  {"continent": "europe"},
"sequenced",
  "not_sequenced",
  {"clade": "3C3a"},
{"name": "SWITZERLAND/9715293/2013"},
  {"name": "SWITZERLAND/9715293/2013", "passage": "reassortant"},
  {"full_name": "A(H1N1)/MICHIGAN/2/2009 MDCK1"},
"vaccine",
  {"vaccine": {"type": "previous", "no": 0, "passage": "egg", "name": "SWITZERLAND"}},
{"in_rectangle": {"c1": [0.0, 0.0], "c2": [1.0, 1.0]}}, {"in_circle": {"center": [2.0, 2.0], "radius": 5.0}}
{"lab": "CDC"} - returns empty index list for other labs
{"subtype": "H1 A(H1N1) H3 A(H3N2) B BV BVIC BY BYAM"}
{"outlier": 1.0} // threshold in units (distance from centroid of pre-selected points), must be after other select args, e.g. after "clade"
{"table": "20170216"}
  {"table": "MELB:HI:turkey:20170216"}

Sera:
-----
"all",
{"serum_id": "CDC 2016-003"},
{"index": 11}, {"indices": [55, 66]},
{"country": "sweden"}, {"continent": "europe"},
{"name": "SWITZERLAND/9715293/2013"},
{"full_name": "A(H1N1)/MICHIGAN/2/2009 CDC 2015-121"},
{"table": "20170216"}
  {"table": "MELB:HI:turkey:20170216"}


)";

const char* settings_help_select()
{
    return SETTINGS_HELP_SELECT;
}

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
//     "DEL2017": "#000080",
//     "TRIPLEDEL2017": "#46f0f0"
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
