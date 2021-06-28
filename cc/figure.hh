#pragma once

#include "acmacs-map-draw/coordinates.hh"

// ----------------------------------------------------------------------

namespace acmacs::mapi::inline v1
{
    struct FigureRaw
    {
        std::vector<map_elements::v2::Coordinates> vertices;
        bool close;
    };

    struct Figure
    {
        Figure() = default;
        Figure(const FigureRaw& figure_raw, const ChartDraw& chart_draw);

        std::vector<acmacs::PointCoordinates> vertices;
        bool close;

        bool empty() const { return vertices.empty(); }
        bool inside(const acmacs::PointCoordinates& point) const { return winding_number(point, vertices) != 0; }
        bool outside(const acmacs::PointCoordinates& point) const { return winding_number(point, vertices) == 0; }

      private:
        // returns winding number, i.e. 0 if point is outside polygon defined by path, non-zero otherwise
        static int winding_number(const acmacs::PointCoordinates& point, const std::vector<acmacs::PointCoordinates>& path);

    };

} // namespace acmacs::mapi::inline v1

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
