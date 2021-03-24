#pragma once

#include "acmacs-map-draw/coordinates.hh"

// ----------------------------------------------------------------------

namespace acmacs::mapi::inline v1
{
    using vertices_t = std::vector<map_elements::v2::Coordinates>;

    struct Figure
    {
        vertices_t vertices;
        bool close;

        bool inside(const acmacs::PointCoordinates& point, const ChartDraw& chart_draw) const { return winding_number(point, vertices, chart_draw) != 0; }
        bool outside(const acmacs::PointCoordinates& point, const ChartDraw& chart_draw) const { return winding_number(point, vertices, chart_draw) == 0; }

      private:
        // returns winding number, i.e. 0 if point is outside polygon defined by path, non-zero otherwise
        static int winding_number(const acmacs::PointCoordinates& point, const vertices_t& path, const ChartDraw& chart_draw);

    };

} // namespace acmacs::mapi::inline v1

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
