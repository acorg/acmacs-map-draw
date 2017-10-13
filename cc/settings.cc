#include "settings.hh"

// ----------------------------------------------------------------------

static const char* const SETTINGS_DEFAULT = R"(
{
  "apply": [
    "all_grey",
    "egg",
    "vaccines"
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
    "egg": [
      {"N": "antigens", "select": {"passage": "egg"}, "aspect": 0.75},
      {"N": "antigens", "select": {"passage": "reassortant"}, "aspect": 0.75, "rotation": 0.5}
    ],
    "vaccines": [
      {"N": "antigens", "select": {"vaccine": {"type": "previous"}}, "report": true, "outline": "black", "fill": "blue", "size": 15, "order": "raise"},
      {"N": "antigens", "select": {"vaccine": {"type": "current", "passage": "cell"}}, "label": {}, "report": true, "outline": "black", "fill": "red", "size": 15, "order": "raise"},
      {"N": "antigens", "select": {"vaccine": {"type": "current", "passage": "egg"}}, "label": {"display_name": "VWVIiV", "color": "red", "size": 30, "offset": [0, -1], "weight": "bold", "font_family":"monospace"}, "report": true, "outline": "black", "fill": "red", "size": 15, "order": "raise"},
      {"N": "antigens", "select": {"vaccine": {"type": "current", "passage": "reassortant"}}, "label": {}, "report": true, "outline": "black", "fill": "green", "size": 15, "order": "raise"},
      {"N": "antigens", "select": {"vaccine": {"type": "surrogate"}}, "report": true, "outline": "black", "fill": "pink", "size": 15, "order": "raise"}
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
 "label": {"show": true, "display_name": "NAME", "color": "black",
           "size": 12.0, "offset": [0, 1], "weight": "bold",
           "slant": "italic", "font_family": "monospace"},
 "report": false}

{"N": "sera", "select": {<select>},
 "outline": "black", "fill": "red", "aspect": 1.0, "rotation": 0.0,
 "size": 1.0, "outline_width": 1.0,
 "show": true, "shape": "circle|box|triangle",
 "order": "raise|lower",
 "report": false}

"all_grey"
"vaccines"

// flip
//   value: [0, 1]
//   ew
//   ns
// rotate_degrees angle:
// rotate_radians angle:
// viewport value:
// use_chart_plot_spec
// point_scale
// {"N": "background", "color": "white"},
// {"N": "grid", "color": "grey80", "line_width": 1},
// {"N": "border", "color": "black", "line_width": 1},
// title size: background: border_color: border_width: display_name: []
// legend data:[{"label": {<label-data>} "display_name": "163-del", "outline": "black", "fill": "#03569b"}] "offset": [-10, -10], "show": True, "size": 50, "label_size": 8, "point_size": 5
// move_antigens
// move_sera
// --> export ace with new plot spec
// --> export lispmds with new plot spec
//
// antigens
//   show
//   report, report_names_threshold
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
//  continents

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
  {"date_range": ["", "2016-09-01"]}, {"date_range": ["2016-01-01", ""]},
  {"older_than_days": 365}, {"younger_than_days": 365},
{"index": 11}, {"indices": [55, 66]},
{"country": "sweden"}, {"continent": "europe"},
"sequenced", "not_sequenced", {"clade": "3C3a"},
{"name": "SWITZERLAND/9715293/2013"}, {"name": "SWITZERLAND/9715293/2013", "passage": "reassortant"},
{"full_name": "A(H1N1)/MICHIGAN/2/2009 MDCK1"},
"vaccine", {"vaccine": {"type": "previous", "no": 0, "passage": "egg", "name": "SWITZERLAND"}},
{"in_rectangle": {"c1": [0.0, 0.0], "c2": [1.0, 1.0]}}, {"in_circle": {"center": [2.0, 2.0], "radius": 5.0}}

Sera:
-----
"all",
{"serum_id": "CDC 2016-003"},
{"index": 11}, {"indices": [55, 66]},
{"country": "sweden"}, {"continent": "europe"},
{"name": "SWITZERLAND/9715293/2013"},
{"full_name": "A(H1N1)/MICHIGAN/2/2009 CDC 2015-121"},


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
// use_chart_plot_spec
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
