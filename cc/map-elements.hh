#pragma once

#include <string>
#include <vector>
#include <memory>

#include "acmacs-draw/color.hh"
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
    virtual ~MapElement();

    inline std::string keyword() const { return mKeyword; }
    inline MapElements::Order order() const { return mOrder; }
    virtual void draw(Surface& aSurface) const = 0;

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
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
