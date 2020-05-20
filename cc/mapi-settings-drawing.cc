#include "acmacs-map-draw/mapi-settings.hh"
#include "acmacs-map-draw/map-elements-v2.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

bool acmacs::mapi::v1::Settings::apply_circle()
{
    auto& circle = chart_draw().map_elements().add<map_elements::v2::Circle>();
    return true;

} // acmacs::mapi::v1::Settings::apply_circle

// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
