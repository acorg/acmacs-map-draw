#pragma once

#include <string>
#include <vector>
#include <memory>
#include <typeinfo>

#include "acmacs-base/size-scale.hh"
#include "acmacs-base/color.hh"
#include "acmacs-base/text-style.hh"
#include "acmacs-base/line.hh"
#include "acmacs-draw/surface.hh"

// ----------------------------------------------------------------------

namespace acmacs::surface { class Surface; }
namespace acmacs::draw { class DrawElements; }
class ChartDraw;

// ----------------------------------------------------------------------

namespace map_elements
{
    class Element;

    class Elements
    {
     public:
        enum Order { BeforePoints, AfterPoints };

        Elements();
        Element& operator[](std::string aKeyword);
        Element& add(std::string aKeyword);
        void remove(std::string aKeyword);
        bool exists(std::string aKeyword) const;

        void draw(acmacs::surface::Surface& aSurface, Order aOrder, const ChartDraw& aChartDraw) const;
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const;

     private:
        std::vector<std::shared_ptr<Element>> mElements;

    }; // class Elements

// ----------------------------------------------------------------------

    class Element
    {
     public:
        Element(std::string aKeyword, Elements::Order aOrder) : mKeyword(aKeyword), mOrder(aOrder) {}
        Element(const Element&) = default;
        virtual ~Element();

        std::string keyword() const { return mKeyword; }
        Elements::Order order() const { return mOrder; }
        virtual void draw(acmacs::surface::Surface& /* aSurface*/, const ChartDraw& /*aChartDraw*/) const {} // obsolete
        virtual void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const = 0; // { std::cerr << "WARNING: map_elements::Element::draw " << typeid(this).name() << '\n'; }

     protected:
        virtual acmacs::Location2D subsurface_origin(acmacs::surface::Surface& aSurface, acmacs::Location2D aPixelOrigin, const acmacs::Size& aScaledSubsurfaceSize) const;

        void keyword(std::string aKeyword) { mKeyword = aKeyword; }

     private:
        std::string mKeyword;
        Elements::Order mOrder;
    };

// ----------------------------------------------------------------------

    class BackgroundBorderGrid : public Element
    {
     public:
        BackgroundBorderGrid()
            : Element("background-border-grid", Elements::BeforePoints),
              mBackground("white"), mGridColor("grey80"), mGridLineWidth(1), mBorderColor("black"), mBorderWidth(1) {}

        void draw(acmacs::surface::Surface& aSurface, const ChartDraw& aChartDraw) const override;
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        void background_color(Color aBackground) { mBackground = aBackground; }
        void grid(Color aGridColor, double aGridLineWidth) { mGridColor = aGridColor; mGridLineWidth = aGridLineWidth; }
        void border(Color aBorderColor, double aBorderWidth) { mBorderColor = aBorderColor; mBorderWidth = aBorderWidth; }

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
        auto& offset_width(acmacs::Location2D aOrigin, Pixels aWidthInParent) { mOrigin = aOrigin; mWidthInParent = aWidthInParent; return *this; }

     private:
        acmacs::Location2D mOrigin{-1, -1};
        Pixels mWidthInParent{100};

    }; // class ContinentMap

// ----------------------------------------------------------------------

    class LegendPointLabel : public Element
    {
     public:
        struct Line
        {
            Line(Color aOutline, Color aFill, std::string aLabel) : outline(aOutline), fill(aFill), label(aLabel) {}
            Color outline, fill;
            std::string label;
            friend inline std::ostream& operator<<(std::ostream& out, const Line& line) { return out << "(LegendPointLabel line \"" << line.label << "\" " << line.fill << ' ' << line.outline << ')'; }
        };

        LegendPointLabel();

        void draw(acmacs::surface::Surface& aSurface, const ChartDraw& aChartDraw) const override;
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        void offset(acmacs::Location2D aOrigin) { mOrigin = aOrigin; }
        void add_line(Color outline, Color fill, std::string label) { mLines.emplace_back(outline, fill, label); }
        void remove_line(std::string label) { mLines.erase(std::remove_if(mLines.begin(), mLines.end(), [&label](const auto& elt) { return elt.label == label; }), mLines.end()); }
        void label_size(double aLabelSize) { mLabelSize = aLabelSize; }
        void point_size(double aPointSize) { mPointSize = aPointSize; }
        void background(Color aBackground) { mBackground = aBackground; }
        void border_color(Color aBorderColor) { mBorderColor = aBorderColor; }
        void border_width(double aBorderWidth) { mBorderWidth = aBorderWidth; }

        const auto& lines() const { return mLines; }

      private:
        acmacs::Location2D mOrigin;
        Color mBackground;
        Color mBorderColor;
        Pixels mBorderWidth;
        Pixels mPointSize;
        Color mLabelColor;
        Pixels mLabelSize;
        acmacs::TextStyle mLabelStyle;
        double mInterline;
        std::vector<Line> mLines;

    }; // class ContinentMap

// ----------------------------------------------------------------------

    class Title : public Element
    {
     public:
        Title();

        virtual void draw(acmacs::surface::Surface& aSurface) const;
        void draw(acmacs::surface::Surface& aSurface, const ChartDraw&) const override { draw(aSurface); }
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        Title& show(bool aShow) { mShow = aShow; return *this; }
        Title& offset(acmacs::Location2D aOrigin) { mOrigin = aOrigin; return *this; }
          // Title& offset(double x, double y) { mOrigin = Location2D{x, y}; return *this; }
        Title& padding(double x) { mPadding = x; return *this; }
        Title& remove_all_lines() { mLines.clear(); return *this; }
        Title& add_line(std::string aText) { mLines.emplace_back(aText); return *this; }
        Title& text_size(double aTextSize) { mTextSize = aTextSize; return *this; }
        Title& text_color(Color aTextColor) { mTextColor = aTextColor; return *this; }
        Title& background(Color aBackground) { mBackground = aBackground; return *this; }
        Title& border_color(Color aBorderColor) { mBorderColor = aBorderColor; return *this; }
        Title& border_width(double aBorderWidth) { mBorderWidth = aBorderWidth; return *this; }
        Title& weight(std::string aWeight) { mTextStyle.weight = aWeight; return *this; }
        Title& slant(std::string aSlant) { mTextStyle.slant = aSlant; return *this; }
        Title& font_family(std::string aFamily) { mTextStyle.font_family = aFamily; return *this; }

     private:
        bool mShow;
        acmacs::Location2D mOrigin;
        Pixels mPadding;
        Color mBackground;
        Color mBorderColor;
        Pixels mBorderWidth;
        Color mTextColor;
        Pixels mTextSize;
        acmacs::TextStyle mTextStyle;
        double mInterline;
        std::vector<std::string> mLines;

    }; // class ContinentMap

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

        SerumCircle& serum_no(size_t aSerumNo) { mSerumNo = aSerumNo; return *this; }
        SerumCircle& radius(Scaled aRadius) { mRadius = aRadius; return *this; }
        SerumCircle& fill(Color aFill) { mFillColor = aFill; return *this; }
        SerumCircle& outline(Color aOutline, double aOutlineWidth) { mOutlineColor = aOutline; mOutlineWidth = aOutlineWidth; return *this; }
        SerumCircle& radius_line(Color aRadius, double aRadiusWidth) { mRadiusColor = aRadius; mRadiusWidth = aRadiusWidth; return *this; }
        SerumCircle& angles(double aStart, double aEnd) { mStart = aStart; mEnd = aEnd; return *this; }
        SerumCircle& radius_line_no_dash() { mRadiusDash = acmacs::surface::Dash::NoDash; return *this; }
        SerumCircle& radius_line_dash1() { mRadiusDash = acmacs::surface::Dash::Dash1; return *this; }
        SerumCircle& radius_line_dash2() { mRadiusDash = acmacs::surface::Dash::Dash2; return *this; }

     private:
        size_t mSerumNo;
        Scaled mRadius;
        Color mFillColor;
        Color mOutlineColor;
        Pixels mOutlineWidth;
        Color mRadiusColor;
        Pixels mRadiusWidth;
        acmacs::surface::Dash mRadiusDash;
        Rotation mStart;
        Rotation mEnd;

    }; // class SerumCircle

// ----------------------------------------------------------------------

    class Line : public Element
    {
     public:
        Line()
            : Element{"line", Elements::AfterPoints}, mLineColor{"pink"}, mLineWidth{1} {}

        Line& color(Color aColor) { mLineColor = aColor; return *this; }
        Line& line_width(double aLineWidth) { mLineWidth = aLineWidth; return *this; }

     protected:
        Color mLineColor;
        Pixels mLineWidth;

    }; // class Line

    class LineFromTo : public Line
    {
     public:
        void draw(acmacs::surface::Surface& aSurface, const ChartDraw& aChartDraw) const override;
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        Line& from_to(acmacs::Location2D aBegin, acmacs::Location2D aEnd) { mBegin = aBegin; mEnd = aEnd; return *this; }

     protected:
        acmacs::Location2D mBegin;
        acmacs::Location2D mEnd;

    }; // class Line

    class LineSlope : public Line
    {
     public:
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        Line& line(acmacs::LineDefinedByEquation line) { line_ = line; return *this; }
        Line& apply_map_transformation(bool apply = true) { apply_map_transformation_ = apply; return *this; }

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

        Arrow& from_to(acmacs::Location2D aBegin, acmacs::Location2D aEnd) { LineFromTo::from_to(aBegin, aEnd); return *this; }
        Arrow& color(Color aLineColor, Color aArrowHeadColor) { LineFromTo::color(aLineColor); mArrowHeadColor = aArrowHeadColor; return *this; }
        Arrow& color(Color aColor) { return color(aColor, aColor); }
        Arrow& arrow_head_filled(bool aFilled) { mArrowHeadFilled = aFilled; return *this; }
        Arrow& line_width(double aLineWidth) { LineFromTo::line_width(aLineWidth); return *this; }
        Arrow& arrow_width(double aArrowWidth) { mArrowWidth = aArrowWidth; return *this; }

     private:
        Color mArrowHeadColor;
        bool mArrowHeadFilled;
        Pixels mArrowWidth;

    }; // class Arrow

// ----------------------------------------------------------------------

    class Rectangle : public Element
    {
     public:
        Rectangle()
            : Element{"rectangle", Elements::AfterPoints}, mColor{0x8000FFFF}, mFilled{true}, mLineWidth{1} {}

        void draw(acmacs::surface::Surface& aSurface, const ChartDraw& aChartDraw) const override;
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        Rectangle& corners(acmacs::Location2D aCorner1, acmacs::Location2D aCorner2) { mCorner1 = aCorner1; mCorner2 = aCorner2; return *this; }
        Rectangle& color(Color aColor) { mColor = aColor; return *this; }
        Rectangle& filled(bool aFilled) { mFilled = aFilled; return *this; }
        Rectangle& line_width(double aLineWidth) { mLineWidth = aLineWidth; return *this; }

     protected:
        acmacs::Location2D mCorner1, mCorner2;
        Color mColor;
        bool mFilled;
        Pixels mLineWidth;

    }; // class Rectangle

// ----------------------------------------------------------------------

    class Circle : public Element
    {
     public:
        Circle()
            : Element{"circle", Elements::AfterPoints}, mFillColor{"transparent"}, mOutlineColor{"pink"}, mOutlineWidth{1}, mAspect{AspectNormal}, mRotation{NoRotation} {}

        void draw(acmacs::surface::Surface& aSurface, const ChartDraw& aChartDraw) const override;
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        Circle& center(acmacs::Location2D aCenter) { mCenter = aCenter; return *this; }
        Circle& size(Scaled aSize) { mSize = aSize; return *this; }
        Circle& color(Color aFillColor, Color aOutlineColor) { mFillColor = aFillColor; mOutlineColor = aOutlineColor; return *this; }
        Circle& outline_width(double aOutlineWidth) { mOutlineWidth = aOutlineWidth; return *this; }
        Circle& aspect(Aspect aAspect) { mAspect = aAspect; return *this; }
        Circle& rotation(Rotation aRotation) { mRotation = aRotation; return *this; }

     protected:
        acmacs::Location2D mCenter;
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
        Point()
            : Element{"point", Elements::AfterPoints}, mSize{10}, mFillColor{"pink"}, mOutlineColor{"pink"},
              mOutlineWidth{1}, mAspect{AspectNormal}, mRotation{NoRotation} {}

        void draw(acmacs::surface::Surface& aSurface, const ChartDraw& aChartDraw) const override;
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        Point& center(acmacs::Location2D aCenter) { mCenter = aCenter; return *this; }
        Point& size(Pixels aSize) { mSize = aSize; return *this; }
        Point& color(Color aFillColor, Color aOutlineColor) { mFillColor = aFillColor; mOutlineColor = aOutlineColor; return *this; }
        Point& outline_width(double aOutlineWidth) { mOutlineWidth = aOutlineWidth; return *this; }

     private:
        acmacs::Location2D mCenter;
        Pixels mSize;
        Color mFillColor;
        Color mOutlineColor;
        Pixels mOutlineWidth;
        Aspect mAspect;
        Rotation mRotation;

    }; // class Point

} // namespace map_elements

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
