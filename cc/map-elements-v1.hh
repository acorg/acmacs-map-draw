#pragma once

#include "acmacs-map-draw/map-elements.hh"

// ----------------------------------------------------------------------

namespace map_elements::v1
{
    class Path : public Element
    {
      public:
        Path() : Element{"path", Elements::AfterPoints}, mLineColor{"pink"}, mLineWidth{1} {}

        void draw(acmacs::surface::Surface& aSurface, const ChartDraw& aChartDraw) const override;
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        Path& color(Color aColor)
        {
            mLineColor = aColor;
            return *this;
        }
        Path& line_width(double aLineWidth)
        {
            mLineWidth = Pixels{aLineWidth};
            return *this;
        }
        Path& add(const acmacs::PointCoordinates& aPoint)
        {
            mPath.push_back(aPoint);
            return *this;
        }
        Path& close(Color aFill)
        {
            close_and_fill_ = aFill;
            return *this;
        }

      protected:
        Color mLineColor;
        Pixels mLineWidth;
        std::vector<acmacs::PointCoordinates> mPath;
        std::optional<Color> close_and_fill_;

    }; // class Path

    // ----------------------------------------------------------------------

    class Line : public Element
    {
      public:
        Line() : Element{"line", Elements::AfterPoints}, mLineColor{"pink"}, mLineWidth{1} {}

        Line& color(Color aColor)
        {
            mLineColor = aColor;
            return *this;
        }
        Line& line_width(double aLineWidth)
        {
            mLineWidth = Pixels{aLineWidth};
            return *this;
        }

      protected:
        Color mLineColor;
        Pixels mLineWidth;

    }; // class Line

    class LineFromTo : public Line
    {
      public:
        void draw(acmacs::surface::Surface& aSurface, const ChartDraw& aChartDraw) const override;
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        Line& from_to(const acmacs::PointCoordinates& aBegin, const acmacs::PointCoordinates& aEnd)
        {
            mBegin = aBegin;
            mEnd = aEnd;
            return *this;
        }

      protected:
        acmacs::PointCoordinates mBegin{acmacs::number_of_dimensions_t{2}};
        acmacs::PointCoordinates mEnd{acmacs::number_of_dimensions_t{2}};

    }; // class Line

    class LineSlope : public Line
    {
      public:
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        Line& line(acmacs::LineDefinedByEquation line)
        {
            line_ = line;
            return *this;
        }
        Line& apply_map_transformation(bool apply = true)
        {
            apply_map_transformation_ = apply;
            return *this;
        }

      protected:
        acmacs::LineDefinedByEquation line_;
        bool apply_map_transformation_ = false;

    }; // class LineSlope

    // ----------------------------------------------------------------------

    class Arrow : public LineFromTo
    {
      public:
        Arrow() : mArrowHeadColor{"pink"}, mArrowHeadFilled{true}, mArrowWidth{5} { keyword("arrow"); }

        void draw(acmacs::surface::Surface& aSurface, const ChartDraw& aChartDraw) const override;
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        Arrow& from_to(const acmacs::PointCoordinates& aBegin, const acmacs::PointCoordinates& aEnd)
        {
            LineFromTo::from_to(aBegin, aEnd);
            return *this;
        }
        Arrow& color(Color aLineColor, Color aArrowHeadColor)
        {
            LineFromTo::color(aLineColor);
            mArrowHeadColor = aArrowHeadColor;
            return *this;
        }
        Arrow& color(Color aColor) { return color(aColor, aColor); }
        Arrow& arrow_head_filled(bool aFilled)
        {
            mArrowHeadFilled = aFilled;
            return *this;
        }
        Arrow& line_width(double aLineWidth)
        {
            LineFromTo::line_width(aLineWidth);
            return *this;
        }
        Arrow& arrow_width(double aArrowWidth)
        {
            mArrowWidth = Pixels{aArrowWidth};
            return *this;
        }

      private:
        Color mArrowHeadColor;
        bool mArrowHeadFilled;
        Pixels mArrowWidth;

    }; // class Arrow

    // ----------------------------------------------------------------------

    class Rectangle : public Element
    {
      public:
        Rectangle() : Element{"rectangle", Elements::AfterPoints}, mColor{0x8000FFFF}, mFilled{true}, mLineWidth{1} {}

        void draw(acmacs::surface::Surface& aSurface, const ChartDraw& aChartDraw) const override;
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        Rectangle& corners(const acmacs::PointCoordinates& aCorner1, const acmacs::PointCoordinates& aCorner2)
        {
            mCorner1 = aCorner1;
            mCorner2 = aCorner2;
            return *this;
        }
        Rectangle& color(Color aColor)
        {
            mColor = aColor;
            return *this;
        }
        Rectangle& filled(bool aFilled)
        {
            mFilled = aFilled;
            return *this;
        }
        Rectangle& line_width(double aLineWidth)
        {
            mLineWidth = Pixels{aLineWidth};
            return *this;
        }

      protected:
        acmacs::PointCoordinates mCorner1{acmacs::number_of_dimensions_t{2}}, mCorner2{acmacs::number_of_dimensions_t{2}};
        Color mColor;
        bool mFilled;
        Pixels mLineWidth;

    }; // class Rectangle

    // ----------------------------------------------------------------------

    class Circle : public Element
    {
      public:
        Circle() : Element{"circle", Elements::AfterPoints}, mFillColor{"transparent"}, mOutlineColor{"pink"}, mOutlineWidth{1}, mAspect{AspectNormal}, mRotation{NoRotation} {}

        void draw(acmacs::surface::Surface& aSurface, const ChartDraw& aChartDraw) const override;
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        Circle& center(const acmacs::PointCoordinates& aCenter)
        {
            mCenter = aCenter;
            return *this;
        }
        Circle& size(Scaled aSize)
        {
            mSize = aSize;
            return *this;
        }
        Circle& color(Color aFillColor, Color aOutlineColor)
        {
            mFillColor = aFillColor;
            mOutlineColor = aOutlineColor;
            return *this;
        }
        Circle& outline_width(double aOutlineWidth)
        {
            mOutlineWidth = Pixels{aOutlineWidth};
            return *this;
        }
        Circle& aspect(Aspect aAspect)
        {
            mAspect = aAspect;
            return *this;
        }
        Circle& rotation(Rotation aRotation)
        {
            mRotation = aRotation;
            return *this;
        }

      protected:
        acmacs::PointCoordinates mCenter{acmacs::number_of_dimensions_t{2}};
        Scaled mSize;
        Color mFillColor;
        Color mOutlineColor;
        Pixels mOutlineWidth;
        Aspect mAspect;
        Rotation mRotation;

    }; // class Circle

    // ----------------------------------------------------------------------

    class Point : public Element
    {
      public:
        Point() : Element{"point", Elements::AfterPoints}, mSize{10}, mFillColor{"pink"}, mOutlineColor{"pink"}, mOutlineWidth{1}, mAspect{AspectNormal}, mRotation{NoRotation} {}

        void draw(acmacs::surface::Surface& aSurface, const ChartDraw& aChartDraw) const override;
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        Point& center(const acmacs::PointCoordinates& aCenter)
        {
            mCenter = aCenter;
            return *this;
        }
        Point& size(Pixels aSize)
        {
            mSize = aSize;
            return *this;
        }
        Point& color(Color aFillColor, Color aOutlineColor)
        {
            mFillColor = aFillColor;
            mOutlineColor = aOutlineColor;
            return *this;
        }
        Point& outline_width(double aOutlineWidth)
        {
            mOutlineWidth = Pixels{aOutlineWidth};
            return *this;
        }
        Point& label(std::string aLabel)
        {
            mLabel = aLabel;
            return *this;
        }

      private:
        acmacs::PointCoordinates mCenter{acmacs::number_of_dimensions_t{2}};
        Pixels mSize;
        Color mFillColor;
        Color mOutlineColor;
        Pixels mOutlineWidth;
        Aspect mAspect;
        Rotation mRotation;
        std::string mLabel;

    }; // class Point

} // namespace map_elements::v1

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
