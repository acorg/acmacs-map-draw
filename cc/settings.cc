#include "settings.hh"

// ----------------------------------------------------------------------

static const char* const DEFAULT_SETTINGS_JSON = R"(
{
  "clades": {
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
    "1B": "red"
  }
}
)";

// ----------------------------------------------------------------------

rjson::value default_settings()
{
    return rjson::parse_string(DEFAULT_SETTINGS_JSON);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
