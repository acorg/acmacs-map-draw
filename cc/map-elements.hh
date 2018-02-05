#pragma once

#include <string>
#include <vector>
#include <memory>

#include "acmacs-base/size-scale.hh"
#include "acmacs-base/color-target.hh"
#include "acmacs-base/text-style.hh"
#include "acmacs-draw/surface.hh"

// ----------------------------------------------------------------------

class Surface;
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

        void draw(Surface& aSurface, Order aOrder, const ChartDraw& aChartDraw) const;

     private:
        std::vector<std::shared_ptr<Element>> mElements;

    }; // class Elements

// ----------------------------------------------------------------------

    class Element
    {
     public:
        inline Element(std::string aKeyword, Elements::Order aOrder) : mKeyword(aKeyword), mOrder(aOrder) {}
        inline Element(const Element&) = default;
        virtual ~Element();

        inline std::string keyword() const { return mKeyword; }
        inline Elements::Order order() const { return mOrder; }
        virtual void draw(Surface& aSurface, const ChartDraw& aChartDraw) const = 0;

     protected:
        virtual acmacs::Location subsurface_origin(Surface& aSurface, const acmacs::Location& aPixelOrigin, const acmacs::Size& aScaledSubsurfaceSize) const;

        inline void keyword(std::string aKeyword) { mKeyword = aKeyword; }

     private:
        std::string mKeyword;
        Elements::Order mOrder;
    };

// ----------------------------------------------------------------------

    class BackgroundBorderGrid : public Element
    {
     public:
        inline BackgroundBorderGrid()
            : Element("background-border-grid", Elements::BeforePoints),
              mBackgroud("white"), mGridColor("grey80"), mGridLineWidth(1), mBorderColor("black"), mBorderWidth(1) {}

        void draw(Surface& aSurface, const ChartDraw& aChartDraw) const override;

        inline void background_color(Color aBackgroud) { mBackgroud = aBackgroud; }
        inline void grid(Color aGridColor, double aGridLineWidth) { mGridColor = aGridColor; mGridLineWidth = aGridLineWidth; }
        inline void border(Color aBorderColor, double aBorderWidth) { mBorderColor = aBorderColor; mBorderWidth = aBorderWidth; }

     private:
        Color mBackgroud;
        Color mGridColor;
        Pixels mGridLineWidth;
        Color mBorderColor;
        Pixels mBorderWidth;

    }; // class BackgroundBorderGrid

// ----------------------------------------------------------------------

    class ContinentMap : public Element
    {
     public:
        inline ContinentMap() : Element("continent-map", Elements::AfterPoints), mOrigin{0, 0}, mWidthInParent(100) {}

        void draw(Surface& aSurface, const ChartDraw& aChartDraw) const override;
        inline void offset_width(const acmacs::Location& aOrigin, Pixels aWidthInParent) { mOrigin = aOrigin; mWidthInParent = aWidthInParent; }

     private:
        acmacs::Location mOrigin;
        Pixels mWidthInParent;

    }; // class ContinentMap

// ----------------------------------------------------------------------

    class LegendPointLabel : public Element
    {
     public:
        struct Line
        {
            inline Line(Color aOutline, Color aFill, std::string aLabel) : outline(aOutline), fill(aFill), label(aLabel) {}
            Color outline, fill;
            std::string label;
        };

        LegendPointLabel();

        void draw(Surface& aSurface, const ChartDraw& aChartDraw) const override;

        inline void offset(const acmacs::Location& aOrigin) { mOrigin = aOrigin; }
        inline void add_line(Color outline, Color fill, std::string label) { mLines.emplace_back(outline, fill, label); }
        inline void label_size(double aLabelSize) { mLabelSize = aLabelSize; }
        inline void point_size(double aPointSize) { mPointSize = aPointSize; }
        inline void background(Color aBackgroud) { mBackgroud = aBackgroud; }
        inline void border_color(Color aBorderColor) { mBorderColor = aBorderColor; }
        inline void border_width(double aBorderWidth) { mBorderWidth = aBorderWidth; }

     private:
        acmacs::Location mOrigin;
        Color mBackgroud;
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

        virtual void draw(Surface& aSurface) const;
        inline void draw(Surface& aSurface, const ChartDraw&) const override { draw(aSurface); }

        inline Title& show(bool aShow) { mShow = aShow; return *this; }
        inline Title& offset(const acmacs::Location& aOrigin) { mOrigin = aOrigin; return *this; }
        inline Title& offset(double x, double y) { mOrigin.set(x, y); return *this; }
        inline Title& padding(double x) { mPadding = x; return *this; }
        inline Title& remove_all_lines() { mLines.clear(); return *this; }
        inline Title& add_line(std::string aText) { mLines.emplace_back(aText); return *this; }
        inline Title& text_size(double aTextSize) { mTextSize = aTextSize; return *this; }
        inline Title& text_color(Color aTextColor) { mTextColor = aTextColor; return *this; }
        inline Title& background(Color aBackgroud) { mBackgroud = aBackgroud; return *this; }
        inline Title& border_color(Color aBorderColor) { mBorderColor = aBorderColor; return *this; }
        inline Title& border_width(double aBorderWidth) { mBorderWidth = aBorderWidth; return *this; }
        inline Title& weight(std::string aWeight) { mTextStyle.weight = aWeight; return *this; }
        inline Title& slant(std::string aSlant) { mTextStyle.slant = aSlant; return *this; }
        inline Title& font_family(std::string aFamily) { mTextStyle.font_family = aFamily; return *this; }

     private:
        bool mShow;
        acmacs::Location mOrigin;
        Pixels mPadding;
        Color mBackgroud;
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
        inline SerumCircle()
            : Element("serum-circle", Elements::AfterPoints), mSerumNo(static_cast<size_t>(-1)),
              mFillColor("transparent"), mOutlineColor("pink"), mOutlineWidth(1),
              mRadiusColor("pink"), mRadiusWidth(1), mRadiusDash(Surface::Dash::Dash1), mStart(0), mEnd(0) {}

        void draw(Surface& aSurface, const ChartDraw& aChartDraw) const override;

        inline SerumCircle& serum_no(size_t aSerumNo) { mSerumNo = aSerumNo; return *this; }
        inline SerumCircle& radius(Scaled aRadius) { mRadius = aRadius; return *this; }
        inline SerumCircle& fill(Color aFill) { mFillColor = aFill; return *this; }
        inline SerumCircle& outline(Color aOutline, double aOutlineWidth) { mOutlineColor = aOutline; mOutlineWidth = aOutlineWidth; return *this; }
        inline SerumCircle& radius_line(Color aRadius, double aRadiusWidth) { mRadiusColor = aRadius; mRadiusWidth = aRadiusWidth; return *this; }
        inline SerumCircle& angles(double aStart, double aEnd) { mStart = aStart; mEnd = aEnd; return *this; }
        inline SerumCircle& radius_line_no_dash() { mRadiusDash = Surface::Dash::NoDash; return *this; }
        inline SerumCircle& radius_line_dash1() { mRadiusDash = Surface::Dash::Dash1; return *this; }
        inline SerumCircle& radius_line_dash2() { mRadiusDash = Surface::Dash::Dash2; return *this; }

     private:
        size_t mSerumNo;
        Scaled mRadius;
        Color mFillColor;
        Color mOutlineColor;
        Pixels mOutlineWidth;
        Color mRadiusColor;
        Pixels mRadiusWidth;
        Surface::Dash mRadiusDash;
        Rotation mStart;
        Rotation mEnd;

    }; // class SerumCircle

// ----------------------------------------------------------------------

    class Line : public Element
    {
     public:
        inline Line()
            : Element{"line", Elements::AfterPoints}, mLineColor{"pink"}, mLineWidth{1} {}

        void draw(Surface& aSurface, const ChartDraw& aChartDraw) const override;

        inline Line& from_to(const acmacs::Location& aBegin, const acmacs::Location& aEnd) { mBegin = aBegin; mEnd = aEnd; return *this; }
        inline Line& color(Color aColor) { mLineColor = aColor; return *this; }
        inline Line& line_width(double aLineWidth) { mLineWidth = aLineWidth; return *this; }

     protected:
        acmacs::Location mBegin;
        acmacs::Location mEnd;
        Color mLineColor;
        Pixels mLineWidth;

    }; // class Line

// ----------------------------------------------------------------------

    class Arrow : public Line
    {
     public:
        inline Arrow() : Line(), mArrowHeadColor{"pink"}, mArrowHeadFilled{true}, mArrowWidth{5} { keyword("arrow"); }

        void draw(Surface& aSurface, const ChartDraw& aChartDraw) const override;

        inline Arrow& from_to(const acmacs::Location& aBegin, const acmacs::Location& aEnd) { Line::from_to(aBegin, aEnd); return *this; }
        inline Arrow& color(Color aLineColor, Color aArrowHeadColor) { Line::color(aLineColor); mArrowHeadColor = aArrowHeadColor; return *this; }
        inline Arrow& color(Color aColor) { return color(aColor, aColor); }
        inline Arrow& arrow_head_filled(bool aFilled) { mArrowHeadFilled = aFilled; return *this; }
        inline Arrow& line_width(double aLineWidth) { Line::line_width(aLineWidth); return *this; }
        inline Arrow& arrow_width(double aArrowWidth) { mArrowWidth = aArrowWidth; return *this; }

     private:
        Color mArrowHeadColor;
        bool mArrowHeadFilled;
        Pixels mArrowWidth;

    }; // class Arrow

// ----------------------------------------------------------------------

    class Rectangle : public Element
    {
     public:
        inline Rectangle()
            : Element{"rectangle", Elements::AfterPoints}, mColor{0x8000FFFF}, mFilled{true}, mLineWidth{1} {}

        void draw(Surface& aSurface, const ChartDraw& aChartDraw) const override;

        inline Rectangle& corners(const acmacs::Location& aCorner1, const acmacs::Location& aCorner2) { mCorner1 = aCorner1; mCorner2 = aCorner2; return *this; }
        inline Rectangle& color(Color aColor) { mColor = aColor; return *this; }
        inline Rectangle& filled(bool aFilled) { mFilled = aFilled; return *this; }
        inline Rectangle& line_width(double aLineWidth) { mLineWidth = aLineWidth; return *this; }

     protected:
        acmacs::Location mCorner1, mCorner2;
        Color mColor;
        bool mFilled;
        Pixels mLineWidth;

    }; // class Rectangle

// ----------------------------------------------------------------------

    class Circle : public Element
    {
     public:
        inline Circle()
            : Element{"circle", Elements::AfterPoints}, mFillColor{"transparent"}, mOutlineColor{"pink"}, mOutlineWidth{1}, mAspect{AspectNormal}, mRotation{NoRotation} {}

        void draw(Surface& aSurface, const ChartDraw& aChartDraw) const override;

        inline Circle& center(const acmacs::Location& aCenter) { mCenter = aCenter; return *this; }
        inline Circle& size(Scaled aSize) { mSize = aSize; return *this; }
        inline Circle& color(Color aFillColor, Color aOutlineColor) { mFillColor = aFillColor; mOutlineColor = aOutlineColor; return *this; }
        inline Circle& outline_width(double aOutlineWidth) { mOutlineWidth = aOutlineWidth; return *this; }
        inline Circle& aspect(Aspect aAspect) { mAspect = aAspect; return *this; }
        inline Circle& rotation(Rotation aRotation) { mRotation = aRotation; return *this; }

     protected:
        acmacs::Location mCenter;
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
        inline Point()
            : Element{"point", Elements::AfterPoints}, mSize{10}, mFillColor{"pink"}, mOutlineColor{"pink"},
              mOutlineWidth{1}, mAspect{AspectNormal}, mRotation{NoRotation} {}

        void draw(Surface& aSurface, const ChartDraw& aChartDraw) const override;

        inline Point& center(const acmacs::Location& aCenter) { mCenter = aCenter; return *this; }
        inline Point& size(Pixels aSize) { mSize = aSize; return *this; }
        inline Point& color(Color aFillColor, Color aOutlineColor) { mFillColor = aFillColor; mOutlineColor = aOutlineColor; return *this; }
        inline Point& outline_width(double aOutlineWidth) { mOutlineWidth = aOutlineWidth; return *this; }

     private:
        acmacs::Location mCenter;
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
