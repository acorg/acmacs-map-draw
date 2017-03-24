#pragma once

#include <string>
#include <vector>
#include <memory>

#include "acmacs-base/color.hh"
#include "acmacs-base/size-scale.hh"
#include "acmacs-draw/text-style.hh"
#include "acmacs-draw/surface.hh"

// ----------------------------------------------------------------------

class MapElement;
class Surface;
class ChartDraw;

// ----------------------------------------------------------------------

class MapElements
{
 public:
    enum Order { BeforePoints, AfterPoints };

    MapElements();
    MapElement& operator[](std::string aKeyword);
    MapElement& add(std::string aKeyword);

    void draw(Surface& aSurface, Order aOrder, const ChartDraw& aChartDraw) const;

 private:
    std::vector<std::shared_ptr<MapElement>> mElements;

}; // class MapElements

// ----------------------------------------------------------------------

class MapElement
{
 public:
    inline MapElement(std::string aKeyword, MapElements::Order aOrder) : mKeyword(aKeyword), mOrder(aOrder) {}
    inline MapElement(const MapElement&) = default;
    virtual ~MapElement();

    inline std::string keyword() const { return mKeyword; }
    inline MapElements::Order order() const { return mOrder; }
    virtual void draw(Surface& aSurface, const ChartDraw& aChartDraw) const = 0;

 protected:
    virtual Location subsurface_origin(Surface& aSurface, const Location& aPixelOrigin, const Size& aScaledSubsurfaceSize) const;

 private:
    std::string mKeyword;
    MapElements::Order mOrder;
};

// ----------------------------------------------------------------------

class BackgroundBorderGrid : public MapElement
{
 public:
    inline BackgroundBorderGrid()
        : MapElement("background-border-grid", MapElements::BeforePoints),
          mBackgroud("white"), mGridColor("grey80"), mGridLineWidth(1), mBorderColor("black"), mBorderWidth(1) {}

    virtual void draw(Surface& aSurface, const ChartDraw& aChartDraw) const;

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

class ContinentMap : public MapElement
{
 public:
    inline ContinentMap() : MapElement("continent-map", MapElements::AfterPoints), mOrigin{0, 0}, mWidthInParent(100) {}

    virtual void draw(Surface& aSurface, const ChartDraw& aChartDraw) const;
    inline void offset_width(const Location& aOrigin, Pixels aWidthInParent) { mOrigin = aOrigin; mWidthInParent = aWidthInParent; }

 private:
    Location mOrigin;
    Pixels mWidthInParent;

}; // class ContinentMap

// ----------------------------------------------------------------------

class LegendPointLabel : public MapElement
{
 public:
    struct Line
    {
        inline Line(Color aOutline, Color aFill, std::string aLabel) : outline(aOutline), fill(aFill), label(aLabel) {}
        Color outline, fill;
        std::string label;
    };

    LegendPointLabel();

    virtual void draw(Surface& aSurface, const ChartDraw& aChartDraw) const;
    inline void offset(const Location& aOrigin) { mOrigin = aOrigin; }
    inline void add_line(Color outline, Color fill, std::string label) { mLines.emplace_back(outline, fill, label); }
    inline void label_size(double aLabelSize) { mLabelSize = aLabelSize; }
    inline void point_size(double aPointSize) { mPointSize = aPointSize; }
    inline void background(Color aBackgroud) { mBackgroud = aBackgroud; }
    inline void border_color(Color aBorderColor) { mBorderColor = aBorderColor; }
    inline void border_width(double aBorderWidth) { mBorderWidth = aBorderWidth; }

 private:
    Location mOrigin;
    Color mBackgroud;
    Color mBorderColor;
    Pixels mBorderWidth;
    Pixels mPointSize;
    Color mLabelColor;
    Pixels mLabelSize;
    TextStyle mLabelStyle;
    double mInterline;
    std::vector<Line> mLines;

}; // class ContinentMap

// ----------------------------------------------------------------------

class Title : public MapElement
{
 public:
    Title();

    virtual void draw(Surface& aSurface) const;
    virtual inline void draw(Surface& aSurface, const ChartDraw&) const { draw(aSurface); }
    inline void offset(const Location& aOrigin) { mOrigin = aOrigin; }
    inline void offset(double x, double y) { mOrigin.set(x, y); }
    inline void add_line(std::string aText) { mLines.emplace_back(aText); }
    inline void text_size(double aTextSize) { mTextSize = aTextSize; }
    inline void background(Color aBackgroud) { mBackgroud = aBackgroud; }
    inline void border_color(Color aBorderColor) { mBorderColor = aBorderColor; }
    inline void border_width(double aBorderWidth) { mBorderWidth = aBorderWidth; }
    inline void weight(std::string aWeight) { mTextStyle.weight(aWeight); }
    inline void slant(std::string aSlant) { mTextStyle.slant(aSlant); }
    inline void font_family(std::string aFamily) { mTextStyle.font_family(aFamily); }

 private:
    Location mOrigin;
    Color mBackgroud;
    Color mBorderColor;
    Pixels mBorderWidth;
    Color mTextColor;
    Pixels mTextSize;
    TextStyle mTextStyle;
    double mInterline;
    std::vector<std::string> mLines;

}; // class ContinentMap

// ----------------------------------------------------------------------

class SerumCircle : public MapElement
{
 public:
    inline SerumCircle()
        : MapElement("serum-circle", MapElements::AfterPoints), mSerumNo(static_cast<size_t>(-1)),
          mFillColor("transparent"), mOutlineColor("pink"), mOutlineWidth(1),
          mRadiusColor("pink"), mRadiusWidth(1), mRadiusDash(Surface::Dash::Dash1), mStart(0), mEnd(0) {}

    virtual void draw(Surface& aSurface, const ChartDraw& aChartDraw) const;

    inline void serum_no(size_t aSerumNo) { mSerumNo = aSerumNo; }
    inline void radius(Scaled aRadius) { mRadius = aRadius; }
    inline void fill(Color aFill) { mFillColor = aFill; }
    inline void outline(Color aOutline, double aOutlineWidth) { mOutlineColor = aOutline; mOutlineWidth = aOutlineWidth; }
    inline void radius_line(Color aRadius, double aRadiusWidth) { mRadiusColor = aRadius; mRadiusWidth = aRadiusWidth; }
    inline void angles(double aStart, double aEnd) { mStart = aStart; mEnd = aEnd; }
    inline void radius_line_no_dash() { mRadiusDash = Surface::Dash::NoDash; }
    inline void radius_line_dash1() { mRadiusDash = Surface::Dash::Dash1; }
    inline void radius_line_dash2() { mRadiusDash = Surface::Dash::Dash2; }

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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
