#pragma once

#include "acmacs-chart-2/point-index-list.hh"
#include "acmacs-draw/draw-arrow.hh"
#include "acmacs-map-draw/map-elements.hh"

// ----------------------------------------------------------------------

namespace map_elements::v2
{
    struct Coordinates
    {
      public:
        struct viewport : public acmacs::PointCoordinates
        {
            acmacs::PointCoordinates get(const ChartDraw& chart_draw) const;
        };

        struct layout : public acmacs::PointCoordinates
        {
            acmacs::PointCoordinates get(const ChartDraw& chart_draw) const;
        };

        struct transformed : public acmacs::PointCoordinates
        {
            acmacs::PointCoordinates get(const ChartDraw& chart_draw) const;
        };

        struct points : public acmacs::chart::PointIndexList
        {
            using acmacs::chart::PointIndexList::get;
            acmacs::PointCoordinates get(const ChartDraw& chart_draw) const;
        };

        using coordinates_t = std::variant<viewport, layout, transformed, points>;

        coordinates_t coordinates;

        template <typename Src> Coordinates(const Src& src) : coordinates{src} {}
        acmacs::PointCoordinates get(const ChartDraw& chart_draw) const;

        bool operator==(const Coordinates& rhs) const { return coordinates == rhs.coordinates; }

    }; // class Coordinates

    // ----------------------------------------------------------------------

    class Circle : public Element
    {
      public:
        Circle() : Element{"circle-v2", Elements::AfterPoints} {}

        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        constexpr void radius(Scaled radius) { radius_ = radius; }
        constexpr void aspect(Aspect aspect) { aspect_ = aspect; }
        constexpr void rotation(Rotation rotation) { rotation_ = rotation; }
        void fill(const acmacs::color::Modifier& fill) { fill_.add(fill); }
        void outline(const acmacs::color::Modifier& outline) { outline_.add(outline); }
        constexpr void outline_width(Pixels outline_width) { outline_width_ = outline_width; }
        void center(const Coordinates& center) { center_ = center; }

      protected:
        Coordinates center_{Coordinates::layout{acmacs::PointCoordinates{0.0, 0.0}}};
        Scaled radius_{1.0};
        acmacs::color::Modifier fill_{TRANSPARENT};
        acmacs::color::Modifier outline_{PINK};
        Pixels outline_width_{1.0};
        Aspect aspect_{AspectNormal};
        Rotation rotation_{NoRotation};

    }; // class Circle

    // ----------------------------------------------------------------------

    struct PathData
    {
        std::vector<Coordinates> vertices;
        bool close{true};
    };

    using ArrowData = acmacs::draw::PathWithArrows::ArrowData;

    class Path : public Element
    {
      public:
        Path() : Element{"path-v2", Elements::AfterPoints} {}

        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        constexpr auto& data() { return data_; }
        constexpr const auto& data() const { return data_; }

        void fill(const acmacs::color::Modifier& fill) { fill_.add(fill); }
        void outline(const acmacs::color::Modifier& outline) { outline_.add(outline); }
        constexpr void outline_width(Pixels outline_width) { outline_width_ = outline_width; }
        constexpr auto& arrows() { return arrows_; }

      private:
        PathData data_;
        acmacs::color::Modifier fill_{TRANSPARENT};
        acmacs::color::Modifier outline_{PINK};
        Pixels outline_width_{1};
        std::vector<ArrowData> arrows_;

    }; // class Path

} // namespace map_elements::v2

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
