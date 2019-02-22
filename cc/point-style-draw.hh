#pragma once

#include "acmacs-base/point-style.hh"
#include "acmacs-base/layout.hh"
#include "acmacs-base/rjson-forward.hh"

// ----------------------------------------------------------------------

namespace acmacs::surface { class Surface; }

// ----------------------------------------------------------------------

acmacs::PointStyle point_style_from_json(const rjson::value& aSource);
void draw_point(acmacs::surface::Surface& aSurface, const acmacs::PointStyle& aStyle, const acmacs::PointCoordinates& aCoord);

// ----------------------------------------------------------------------

enum class PointDrawingOrder { NoChange, Raise, Lower };

PointDrawingOrder drawing_order_from_json(const rjson::value& aSource);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
