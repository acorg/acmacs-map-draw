#pragma once

#include "acmacs-base/point-style.hh"
#include "acmacs-base/layout.hh"

// ----------------------------------------------------------------------

namespace acmacs::surface { class Surface; }
namespace rjson { class object; }

// ----------------------------------------------------------------------

acmacs::PointStyle point_style_from_json(const rjson::object& aSource);
void draw_point(acmacs::surface::Surface& aSurface, const acmacs::PointStyle& aStyle, const acmacs::Coordinates& aCoord);

// ----------------------------------------------------------------------

enum class PointDrawingOrder { NoChange, Raise, Lower };

PointDrawingOrder drawing_order_from_json(const rjson::object& aSource);

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
