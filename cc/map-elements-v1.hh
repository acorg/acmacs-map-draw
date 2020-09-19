#pragma once

#include "acmacs-map-draw/map-elements.hh"

// ----------------------------------------------------------------------

namespace map_elements::v1
{
    class BackgroundBorderGrid : public Element
    {
     public:
        BackgroundBorderGrid()
            : Element("background-border-grid", Elements::BeforePoints),
              mBackground("white"), mGridColor("grey80"), mGridLineWidth(1), mBorderColor("black"), mBorderWidth(1) {}

        void draw(acmacs::surface::Surface& aSurface, const ChartDraw& aChartDraw) const override;
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        void background_color(Color aBackground) { mBackground = aBackground; }
        void grid(Color aGridColor, double aGridLineWidth) { mGridColor = aGridColor; mGridLineWidth = Pixels{aGridLineWidth}; }
        void border(Color aBorderColor, double aBorderWidth) { mBorderColor = aBorderColor; mBorderWidth = Pixels{aBorderWidth}; }

     private:
        Color mBackground;
        Color mGridColor;
        Pixels mGridLineWidth;
        Color mBorderColor;
        Pixels mBorderWidth;

    }; // class BackgroundBorderGrid

    // ----------------------------------------------------------------------

    class ContinentMap : public Element
    {
     public:
        ContinentMap() : Element("continent-map", Elements::AfterPoints) {}

        void draw(acmacs::surface::Surface& aSurface, const ChartDraw& aChartDraw) const override;
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;
        auto& offset_width(const acmacs::PointCoordinates& aOrigin, Pixels aWidthInParent) { mOrigin = aOrigin; mWidthInParent = aWidthInParent; return *this; }

     private:
        acmacs::PointCoordinates mOrigin{-1.0, -1.0};
        Pixels mWidthInParent{100};

    }; // class ContinentMap

    // ----------------------------------------------------------------------

    class LegendPointLabel : public Element
    {
     public:
        struct Line
        {
            Line(const acmacs::color::Modifier& aOutline, Pixels aOutlineWidth, const acmacs::color::Modifier& aFill, std::string_view aLabel)
                : outline{aOutline}, outline_width{aOutlineWidth}, fill{aFill}, label{aLabel}
            {
            }
            Line(std::string_view aLabel) : outline{TRANSPARENT}, fill{TRANSPARENT}, label{aLabel} {}
            acmacs::color::Modifier outline;
            Pixels outline_width{1};
            acmacs::color::Modifier fill;
            std::string label;
        };

        LegendPointLabel() : Element("legend-point-label", Elements::AfterPoints) {}

        void draw(acmacs::surface::Surface& aSurface, const ChartDraw& aChartDraw) const override;
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        void offset(const acmacs::PointCoordinates& aOrigin) { mOrigin = aOrigin; }
        void add_line(const acmacs::color::Modifier& outline, const acmacs::color::Modifier& fill, std::string_view label) { mLines.emplace_back(outline, Pixels{1},  fill, label); }
        void remove_line(std::string_view label) { mLines.erase(std::remove_if(mLines.begin(), mLines.end(), [&label](const auto& elt) { return elt.label == label; }), mLines.end()); }
        void label_size(Pixels aLabelSize) { mLabelSize = aLabelSize; }
        void point_size(Pixels aPointSize) { mPointSize = aPointSize; }
        void background(Color aBackground) { mBackground = aBackground; }
        void border_color(Color aBorderColor) { mBorderColor = aBorderColor; }
        void border_width(double aBorderWidth) { mBorderWidth = Pixels{aBorderWidth}; }

        constexpr const auto& lines() const { return mLines; }
        constexpr auto& lines() { return mLines; }

      private:
        acmacs::PointCoordinates mOrigin{-10, -10};
        Color mBackground{WHITE};
        Color mBorderColor{BLACK};
        Pixels mBorderWidth{0.3};
        Pixels mPointSize{8};
        Color mLabelColor{BLACK};
        Pixels mLabelSize{12};
        acmacs::TextStyle mLabelStyle;
        double mInterline{2.0};
        std::vector<Line> mLines;

    }; // class LegendPointLabel

    // ----------------------------------------------------------------------

    class Title : public Element
    {
     public:
        Title(): Element{"title", Elements::AfterPoints} {}

        virtual void draw(acmacs::surface::Surface& aSurface) const;
        void draw(acmacs::surface::Surface& aSurface, const ChartDraw&) const override { draw(aSurface); }
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        size_t number_of_lines() const { return mLines.size(); }

        Title& show(bool aShow) { mShow = aShow; return *this; }
        Title& offset(const acmacs::PointCoordinates& aOrigin) { mOrigin = aOrigin; return *this; }
          // Title& offset(double x, double y) { mOrigin = acmacs::PointCoordinates(x, y); return *this; }
        Title& padding(Pixels x) { mPadding = x; return *this; }
        Title& remove_all_lines() { mLines.clear(); return *this; }
        Title& add_line(std::string_view aText) { mLines.emplace_back(aText); return *this; }
        Title& text_size(Pixels aTextSize) { mTextSize = aTextSize; return *this; }
        Title& text_color(const acmacs::color::Modifier& aTextColor) { mTextColor.add(aTextColor); return *this; }
        Title& interline(double interline) { mInterline = interline; return *this; }
        Title& background(const acmacs::color::Modifier& aBackground) { mBackground.add(aBackground); return *this; }
        Title& border_color(const acmacs::color::Modifier& aBorderColor) { mBorderColor.add(aBorderColor); return *this; }
        Title& border_width(Pixels aBorderWidth) { mBorderWidth = aBorderWidth; return *this; }
        Title& weight(std::string_view aWeight) { mTextStyle.weight = aWeight; return *this; }
        Title& slant(std::string_view aSlant) { mTextStyle.slant = aSlant; return *this; }
        Title& font_family(std::string_view aFamily) { mTextStyle.font_family = aFamily; return *this; }

     protected:
        virtual std::string update_line_before_drawing(std::string_view line, const ChartDraw& aChartDraw) const;

     private:
       bool mShow{true};
       acmacs::PointCoordinates mOrigin{10, 10};
       Pixels mPadding{10};
       acmacs::color::Modifier mBackground{TRANSPARENT};
       acmacs::color::Modifier mBorderColor{BLACK};
       Pixels mBorderWidth{0};
       acmacs::color::Modifier mTextColor{BLACK};
       Pixels mTextSize{12};
       acmacs::TextStyle mTextStyle;
       double mInterline{2.0};
       std::vector<std::string> mLines;

    }; // class Title

    // ----------------------------------------------------------------------

    class SerumCircle : public Element
    {
     public:
        SerumCircle()
            : Element("serum-circle", Elements::AfterPoints), mSerumNo(static_cast<size_t>(-1)),
              mFillColor("transparent"), mOutlineColor("pink"), mOutlineWidth(1),
              mRadiusColor("pink"), mRadiusWidth(1), mRadiusDash(acmacs::surface::Dash::Dash1), mStart(0), mEnd(0) {}

        void draw(acmacs::surface::Surface& aSurface, const ChartDraw& aChartDraw) const override;
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        auto radius() const { return mRadius; }

        SerumCircle& serum_no(size_t aSerumNo) { mSerumNo = aSerumNo; return *this; }
        SerumCircle& radius(Scaled aRadius) { mRadius = aRadius; return *this; }
        SerumCircle& fill(Color aFill) { mFillColor = aFill; return *this; }
        SerumCircle& outline(Color aOutline, Pixels aOutlineWidth) { mOutlineColor = aOutline; mOutlineWidth = aOutlineWidth; return *this; }
        SerumCircle& outline_no_dash() { mOutlineDash = acmacs::surface::Dash::NoDash; return *this; }
        SerumCircle& outline_dash1() { mOutlineDash = acmacs::surface::Dash::Dash1; return *this; }
        SerumCircle& outline_dash2() { mOutlineDash = acmacs::surface::Dash::Dash2; return *this; }
        SerumCircle& outline_dash3() { mOutlineDash = acmacs::surface::Dash::Dash3; return *this; }

        SerumCircle& radius_line(Color aRadius, Pixels aRadiusWidth) { mRadiusColor = aRadius; mRadiusWidth = aRadiusWidth; return *this; }
        SerumCircle& angles(Rotation aStart, Rotation aEnd) { mStart = aStart; mEnd = aEnd; return *this; }
        SerumCircle& radius_line_no_dash() { mRadiusDash = acmacs::surface::Dash::NoDash; return *this; }
        SerumCircle& radius_line_dash1() { mRadiusDash = acmacs::surface::Dash::Dash1; return *this; }
        SerumCircle& radius_line_dash2() { mRadiusDash = acmacs::surface::Dash::Dash2; return *this; }
        SerumCircle& radius_line_dash3() { mRadiusDash = acmacs::surface::Dash::Dash3; return *this; }

     private:
        size_t mSerumNo;
        Scaled mRadius;
        Color mFillColor;
        Color mOutlineColor;
        Pixels mOutlineWidth;
        acmacs::surface::Dash mOutlineDash;
        Color mRadiusColor;
        Pixels mRadiusWidth;
        acmacs::surface::Dash mRadiusDash;
        Rotation mStart;
        Rotation mEnd;

    }; // class SerumCircle

    // ----------------------------------------------------------------------

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
        Circle() : Element{"circle-v1", Elements::AfterPoints}, mFillColor{"transparent"}, mOutlineColor{"pink"}, mOutlineWidth{1}, mAspect{AspectNormal}, mRotation{NoRotation} {}

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
