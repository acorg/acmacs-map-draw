#pragma once

#include <string>
#include <vector>
#include <memory>

#include "acmacs-base/color.hh"
#include "acmacs-draw/size-scale.hh"

// ----------------------------------------------------------------------

class MapElement;
class Surface;

// ----------------------------------------------------------------------

class MapElements
{
 public:
    enum Order { BeforePoints, AfterPoints };

    MapElements();
    MapElement& operator[](std::string aKeyword);

    void draw(Surface& aSurface, Order aOrder) const;

 private:
    std::vector<std::shared_ptr<MapElement>> mElements;

    MapElement& add(std::string aKeyword);
};

// ----------------------------------------------------------------------

class MapElement
{
 public:
    inline MapElement(std::string aKeyword, MapElements::Order aOrder) : mKeyword(aKeyword), mOrder(aOrder) {}
    inline MapElement(const MapElement&) = default;
    virtual ~MapElement();

    inline std::string keyword() const { return mKeyword; }
    inline MapElements::Order order() const { return mOrder; }
    virtual void draw(Surface& aSurface) const = 0;

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

    virtual void draw(Surface& aSurface) const;

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

    virtual void draw(Surface& aSurface) const;
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

    virtual void draw(Surface& aSurface) const;
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
    inline void offset(const Location& aOrigin) { mOrigin = aOrigin; }
    inline void add_line(std::string aText) { mLines.emplace_back(aText); }
    inline void text_size(double aTextSize) { mTextSize = aTextSize; }
    inline void background(Color aBackgroud) { mBackgroud = aBackgroud; }
    inline void border_color(Color aBorderColor) { mBorderColor = aBorderColor; }
    inline void border_width(double aBorderWidth) { mBorderWidth = aBorderWidth; }

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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
