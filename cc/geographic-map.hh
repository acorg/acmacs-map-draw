#pragma once

#include <string>
#include <map>

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
    virtual ~GeographicMapDraw();

    virtual void prepare();
    virtual void draw(Surface& aSurface) const;
    virtual void draw(std::string aFilename) const;

    void add_point(double aLat, double aLong, Color aFill, Pixels aSize);

 private:
    GeographicMapPoints mPoints;

}; // class GeographicMapDraw

// ----------------------------------------------------------------------

class GeographicMapWithPointsFromHidb : public GeographicMapDraw
{
 public:
    class Points : public std::map<std::string, std::map<Color, size_t>>
    {
          // location-name to color to number-of-points
     public:
        inline void add(std::string aLocation, Color aColor) { ++(*this)[aLocation][aColor]; }
        static inline size_t number_of_points_at_location(const std::map<Color, size_t>& colors) { return std::accumulate(colors.begin(), colors.end(), 0U, [](size_t sum, auto elt) -> size_t {return sum + elt.second; }); }
    };

    inline GeographicMapWithPointsFromHidb(const hidb::HiDb& aHiDb, const LocDb& aLocDb) : mHiDb(aHiDb), mLocDb(aLocDb) {}

    virtual void prepare();
      // virtual void draw(Surface& aSurface) const;

    void add_points_from_hidb(std::string aStartDate, std::string aEndDate);

 private:
    const hidb::HiDb& mHiDb;
    const LocDb& mLocDb;
    Points mPoints;

}; // class GeographicMapWithPointsFromHidb

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
