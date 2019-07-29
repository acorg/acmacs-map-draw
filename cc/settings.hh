#pragma once

#include "acmacs-base/rjson.hh"

// ----------------------------------------------------------------------

rjson::value settings_default();
rjson::value settings_builtin_mods();

// const char* settings_help_mods();
// const char* settings_help_select();

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
