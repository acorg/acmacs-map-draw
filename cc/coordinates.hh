#pragma once

#include "acmacs-base/point-coordinates.hh"
#include "acmacs-chart-2/point-index-list.hh"

// ----------------------------------------------------------------------

class ChartDraw;

namespace map_elements::v2
{
    struct Coordinates
    {
      public:
        struct viewport : public acmacs::PointCoordinates
        {
            acmacs::PointCoordinates get_transformed(const ChartDraw& chart_draw) const;
            acmacs::PointCoordinates get_not_transformed(const ChartDraw& chart_draw) const;
        };

        struct not_transformed : public acmacs::PointCoordinates
        {
            acmacs::PointCoordinates get_transformed(const ChartDraw& chart_draw) const;
            acmacs::PointCoordinates get_not_transformed(const ChartDraw& chart_draw) const;
        };

        struct transformed : public acmacs::PointCoordinates
        {
            acmacs::PointCoordinates get_transformed(const ChartDraw& chart_draw) const;
            acmacs::PointCoordinates get_not_transformed(const ChartDraw& chart_draw) const;
        };

        struct points : public acmacs::chart::PointIndexList
        {
            points(size_t index) { push_back(index); }
            points(const acmacs::chart::PointIndexList& index_list) : acmacs::chart::PointIndexList{index_list} {}
            acmacs::PointCoordinates get_transformed(const ChartDraw& chart_draw) const;
            acmacs::PointCoordinates get_not_transformed(const ChartDraw& chart_draw) const;
        };

        using coordinates_t = std::variant<viewport, not_transformed, transformed, points>;

        coordinates_t coordinates;

        template <typename Src> Coordinates(const Src& src) : coordinates{src} {}
        acmacs::PointCoordinates get_transformed(const ChartDraw& chart_draw) const;
        acmacs::PointCoordinates get_not_transformed(const ChartDraw& chart_draw) const;

        bool operator==(const Coordinates& rhs) const { return coordinates == rhs.coordinates; }

    }; // class Coordinates

} // namespace map_elements::v2

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
