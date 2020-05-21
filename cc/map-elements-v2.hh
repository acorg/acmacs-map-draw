#pragma once

#include "acmacs-map-draw/map-elements.hh"

// ----------------------------------------------------------------------

namespace map_elements::v2
{
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

      protected:
        // acmacs::PointCoordinates mCenter{acmacs::number_of_dimensions_t{2}};
        Scaled radius_{1.0};
        acmacs::color::Modifier fill_{TRANSPARENT};
        acmacs::color::Modifier outline_{PINK};
        Pixels outline_width_{1.0};
        Aspect aspect_{AspectNormal};
        Rotation rotation_{NoRotation};

    }; // class Circle

} // namespace map_elements::v2

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
