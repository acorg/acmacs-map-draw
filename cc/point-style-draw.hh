#pragma once

#include "acmacs-base/point-style.hh"
#include "acmacs-base/layout.hh"
#include "acmacs-base/rjson-forward.hh"

// ----------------------------------------------------------------------

namespace acmacs::surface { class Surface; }

// ----------------------------------------------------------------------

acmacs::PointStyleModified point_style_from_json(const rjson::value& aSource, Color passage_color = PINK);
void draw_point(acmacs::surface::Surface& aSurface, const acmacs::PointStyle& aStyle, const acmacs::PointCoordinates& aCoord);

// ----------------------------------------------------------------------

enum class PointDrawingOrder { NoChange, Raise, Lower };

PointDrawingOrder drawing_order_from(const rjson::value& aSource);
PointDrawingOrder drawing_order_from(std::string_view order);
inline PointDrawingOrder drawing_order_from(const std::string& order) { return drawing_order_from(std::string_view{order}); }

// ----------------------------------------------------------------------
