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

    virtual void prepare(Surface& aSurface);
    virtual void draw(std::string aFilename);

    void add_point(double aLat, double aLong, Color aFill, Pixels aSize);

 private:
    GeographicMapPoints mPoints;

    virtual void draw(Surface& aOutlineSurface, Surface& aPointSurface) const;

}; // class GeographicMapDraw

// ----------------------------------------------------------------------

class GeographicMapWithPointsFromHidb : public GeographicMapDraw
{
 public:
    inline GeographicMapWithPointsFromHidb(const hidb::HiDb& aHiDb, const LocDb& aLocDb, double aPointSizeInPixels, double aPointDensity)
        : mHiDb(aHiDb), mLocDb(aLocDb), mPointSize(aPointSizeInPixels), mDensity(aPointDensity) {}

    virtual void prepare(Surface& aSurface);

    void add_points_from_hidb(std::string aStartDate, std::string aEndDate);

 private:
    const hidb::HiDb& mHiDb;
    const LocDb& mLocDb;
    Pixels mPointSize;
    double mDensity;

    class PointsAtLocationIterator;

    class PointsAtLocation : public std::map<Color, size_t>
    {
     public:
        PointsAtLocationIterator iterator() const { return PointsAtLocationIterator(*this); }
    };

    class PointsAtLocationIterator
    {
     public:
        inline Color operator*() const { return mCurrent->first; }
        inline void operator++() { if (mCurrent != mEnd) { ++mCurrentCount; if (mCurrentCount >= mCurrent->second) { ++mCurrent; mCurrentCount = 0; } } }
        inline size_t left() const { return std::accumulate(mCurrent, mEnd, 0U, [](size_t sum, auto elt) -> size_t {return sum + elt.second; }) - mCurrentCount; }
        inline operator bool() const { return mCurrent != mEnd; }

     private:
        PointsAtLocation::const_iterator mCurrent;
        PointsAtLocation::const_iterator mEnd;
        size_t mCurrentCount;

        friend class PointsAtLocation;
        inline PointsAtLocationIterator(const PointsAtLocation& aSource) : mCurrent(aSource.begin()), mEnd(aSource.end()), mCurrentCount(0) {}
    };

    class Points : public std::map<std::string, PointsAtLocation>
    {
          // location-name to color to number-of-points
     public:
        inline void add(std::string aLocation, Color aColor) { ++(*this)[aLocation][aColor]; }
          // static inline size_t number_of_points_at_location(const PointsAtLocation& colors) { return std::accumulate(colors.begin(), colors.end(), 0U, [](size_t sum, auto elt) -> size_t {return sum + elt.second; }); }
    };

    Points mPoints;

}; // class GeographicMapWithPointsFromHidb

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
