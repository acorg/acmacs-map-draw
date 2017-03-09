#pragma once

#include <string>

#include "acmacs-map-draw/point-style-draw.hh"

class Surface;
class LocDb;
namespace hidb { class HiDb; }

// ----------------------------------------------------------------------

class GeographicMapPoint : public PointStyleDraw
{
 public:
    inline GeographicMapPoint() : mLongLat{0, 0} {}
    inline GeographicMapPoint(const Coordinates& aLongLat) : mLongLat(aLongLat) {}

    inline void draw(Surface& aSurface) const { PointStyleDraw::draw(aSurface, mLongLat); }

 private:
    Coordinates mLongLat;
};

// ----------------------------------------------------------------------

class GeographicMapPoints : public std::vector<GeographicMapPoint>
{
 public:
    void draw(Surface& aSurface) const;
};

// ----------------------------------------------------------------------

class GeographicMapDraw
{
 public:
    inline GeographicMapDraw() = default;

    void prepare();
    void draw(Surface& aSurface);
    void draw(std::string aFilename);

    void add_point(double aLat, double aLong, Color aFill, Pixels aSize);
    void add_points_from_hidb(const hidb::HiDb& aHiDb, const LocDb& aLocDb, std::string aStartDate, std::string aEndDate);

 private:
    GeographicMapPoints mPoints;

}; // class GeographicMapDraw

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
