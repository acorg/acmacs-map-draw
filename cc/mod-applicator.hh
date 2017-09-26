#pragma once

#include <stdexcept>

#include "acmacs-base/rjson.hh"

// ----------------------------------------------------------------------

class ChartDraw;

// aMods is a list of:
//  - string with mod name, e.g. "all_grey"
//  - object: {"N": "mod-name", mod parameters}
// aModData is an object for mod parameters and non-standard mod lookup (see settings.cc):
//  {"mod-name": [<mods to apply>]}

void apply_mods(ChartDraw& aChartDraw, const rjson::array& aMods, const rjson::object& aModData, bool aIgnoreUnrecognized = false);

class unrecognized_mod : public std::runtime_error { public: using std::runtime_error::runtime_error; };

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
