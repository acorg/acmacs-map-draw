#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>

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

        bool add_v1(std::string aKeyword);

    }; // class Elements

// ----------------------------------------------------------------------

    class Element
    {
     public:
        Element(std::string aKeyword, Elements::Order aOrder) : mKeyword(aKeyword), mOrder(aOrder) {}
        Element(const Element&) = default;
        virtual ~Element() = default;

        std::string keyword() const { return mKeyword; }
        Elements::Order order() const { return mOrder; }
        virtual void draw(acmacs::surface::Surface& /* aSurface*/, const ChartDraw& /*aChartDraw*/) const {} // obsolete
        virtual void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const = 0; // { std::cerr << "WARNING: map_elements::Element::draw " << typeid(this).name() << '\n'; }

     protected:
        virtual acmacs::PointCoordinates subsurface_origin(acmacs::surface::Surface& aSurface, const acmacs::PointCoordinates& aPixelOrigin, const acmacs::Size& aScaledSubsurfaceSize) const;

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
            Line(Color aOutline, Color aFill, std::string aLabel) : outline(aOutline), fill(aFill), label(aLabel) {}
            Color outline, fill;
            std::string label;
        };

        LegendPointLabel();

        void draw(acmacs::surface::Surface& aSurface, const ChartDraw& aChartDraw) const override;
        void draw(acmacs::draw::DrawElements& aDrawElements, const ChartDraw& aChartDraw) const override;

        void offset(const acmacs::PointCoordinates& aOrigin) { mOrigin = aOrigin; }
        void add_line(Color outline, Color fill, std::string label) { mLines.emplace_back(outline, fill, label); }
        void remove_line(std::string label) { mLines.erase(std::remove_if(mLines.begin(), mLines.end(), [&label](const auto& elt) { return elt.label == label; }), mLines.end()); }
        void label_size(double aLabelSize) { mLabelSize = Pixels{aLabelSize}; }
        void point_size(double aPointSize) { mPointSize = Pixels{aPointSize}; }
        void background(Color aBackground) { mBackground = aBackground; }
        void border_color(Color aBorderColor) { mBorderColor = aBorderColor; }
        void border_width(double aBorderWidth) { mBorderWidth = Pixels{aBorderWidth}; }

        const auto& lines() const { return mLines; }

      private:
        acmacs::PointCoordinates mOrigin;
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
        Title& offset(const acmacs::PointCoordinates& aOrigin) { mOrigin = aOrigin; return *this; }
          // Title& offset(double x, double y) { mOrigin = acmacs::PointCoordinates(x, y); return *this; }
        Title& padding(double x) { mPadding = Pixels{x}; return *this; }
        Title& remove_all_lines() { mLines.clear(); return *this; }
        Title& add_line(std::string aText) { mLines.emplace_back(aText); return *this; }
        Title& text_size(double aTextSize) { mTextSize = Pixels{aTextSize}; return *this; }
        Title& text_color(Color aTextColor) { mTextColor = aTextColor; return *this; }
        Title& background(Color aBackground) { mBackground = aBackground; return *this; }
        Title& border_color(Color aBorderColor) { mBorderColor = aBorderColor; return *this; }
        Title& border_width(double aBorderWidth) { mBorderWidth = Pixels{aBorderWidth}; return *this; }
        Title& weight(std::string aWeight) { mTextStyle.weight = aWeight; return *this; }
        Title& slant(std::string aSlant) { mTextStyle.slant = aSlant; return *this; }
        Title& font_family(std::string aFamily) { mTextStyle.font_family = aFamily; return *this; }

     private:
        bool mShow;
        acmacs::PointCoordinates mOrigin;
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
        SerumCircle& outline(Color aOutline, double aOutlineWidth) { mOutlineColor = aOutline; mOutlineWidth = Pixels{aOutlineWidth}; return *this; }
        SerumCircle& outline_no_dash() { mOutlineDash = acmacs::surface::Dash::NoDash; return *this; }
        SerumCircle& outline_dash1() { mOutlineDash = acmacs::surface::Dash::Dash1; return *this; }
        SerumCircle& outline_dash2() { mOutlineDash = acmacs::surface::Dash::Dash2; return *this; }
        SerumCircle& outline_dash3() { mOutlineDash = acmacs::surface::Dash::Dash3; return *this; }

        SerumCircle& radius_line(Color aRadius, double aRadiusWidth) { mRadiusColor = aRadius; mRadiusWidth = Pixels{aRadiusWidth}; return *this; }
        SerumCircle& angles(double aStart, double aEnd) { mStart = Rotation{aStart}; mEnd = Rotation{aEnd}; return *this; }
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


} // namespace map_elements


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
