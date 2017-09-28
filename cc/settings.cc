#include "settings.hh"

// ----------------------------------------------------------------------

static const char* const DEFAULT_SETTINGS_JSON = R"(
{
  "clade_fill": {
    "?": "H3",
    "3C3": "cornflowerblue",
    "3C2a": "red",
    "3C2a1": "darkred",
    "3C3a": "green",
    "3C3b": "blue",

    "?": "H1pdm",
    "6B1": "blue",
    "6B2": "red",

    "?": "B/Yam",
    "Y2": "cornflowerblue",
    "Y3": "red",

    "?": "B/Vic",
    "1": "blue",
    "1A": "cornflowerblue",
    "1B": "red",
    "DEL2017": "#000080",
    "TRIPLEDEL2017": "#46f0f0"
  }
}
)";

// ----------------------------------------------------------------------

// Select antigens
// "all"
// "reference"
// "test"
// "egg", "cell", "reassortant", {"passage": "egg"}, {"passage": "cell"}, {"passage": "reassortant"}
//  {"date_range": ["2016-01-01", "2016-09-01"]}, {"date_range": ["", "2016-09-01"]}, {"date_range": ["2016-01-01", ""]}, {"older_than_days": 365}, {"younger_than_days": 365}
//  {"index": 11}, {"indices": [55, 66]}
//  {"country": "sweden"}, {"continent": "europe"}
//
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
//   select
//     + all
//     + reference
//     + test
//     + passage:
//     +   egg
//     +   cell
//     +   reassortant
//     name
//     + index
//     + indices []
//     + date_range
//     + older_than_days
//     + younger_than_days
//     + country
//     + continent
//     layout rectangle area, circle area
//     vaccine passage: type: (current, previous, surrogate) name:
//     sequenced
//     not_sequenced
//     clade
//   show
//   size, shape, fill, outline, outline_width, aspect, rotation, raise_ (order: raise, lower, no-change), report, report_names_threshold
//   label: name_type, offset, display_name, color, size, weight, slant, font_family
// {"N": "aa_substitutions", "positions": [159], "legend": {"show": True, "offset": [-10, -10], "point_count": True, "background": "grey99", "border_color": "black", "border_width": 0.1, "label_size": 12, "point_size": 8}},
// {"N": "aa_substitution_groups", "groups": [{"pos_aa": ["121K", "144K"], "color": "cornflowerblue"}]},
//
// sera
//   select
//     all
//     name
//     index
//     indices []
//     country
//     continent
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

rjson::value default_settings()
{
    return rjson::parse_string(DEFAULT_SETTINGS_JSON);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
