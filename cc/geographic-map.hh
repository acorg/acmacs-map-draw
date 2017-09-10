#pragma once

#include <string>
#include <map>

#include "acmacs-base/time-series.hh"
#include "acmacs-map-draw/point-style-draw.hh"
#include "map-elements.hh"

// ----------------------------------------------------------------------

class Surface;
class LocDb;
namespace hidb { class HiDb; }
namespace seqdb { class Seqdb; }
class GeographicMapColoring;    // private, defined in geographic-map.cc

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
    inline GeographicMapDraw(Color aOutline, Pixels aOutlineWidth) : mOutline(aOutline), mOutlineWidth(aOutlineWidth) {}
    inline GeographicMapDraw(const GeographicMapDraw&) = default;
    virtual ~GeographicMapDraw();

    virtual void prepare(Surface& aSurface);
    virtual void draw(std::string aFilename, double aImageWidth);

    void add_point(double aLat, double aLong, Color aFill, Pixels aSize);
    inline Title& title() { return mTitle; }

 private:
    Color mOutline;
    Pixels mOutlineWidth;
    GeographicMapPoints mPoints;
    Title mTitle;

    virtual void draw(Surface& aOutlineSurface, Surface& aPointSurface) const;

}; // class GeographicMapDraw

// ----------------------------------------------------------------------

class GeographicMapColoring
{
 public:
    virtual ~GeographicMapColoring();

    virtual Color color(const hidb::AntigenData& aAntigen) const = 0;
};

// ----------------------------------------------------------------------

class ColoringByContinent : public GeographicMapColoring
{
 public:
    inline ColoringByContinent(const std::map<std::string, std::string>& aContinentColor, const LocDb& aLocDb)
        : mColors{aContinentColor.begin(), aContinentColor.end()}, mLocDb(aLocDb) {}

    virtual Color color(const hidb::AntigenData& aAntigen) const
        {
            try {
                return mColors.at(mLocDb.continent(virus_name::location(aAntigen.data().name())));
            }
            catch (...) {
                return "grey50";
            }
        }

 private:
    std::map<std::string, Color> mColors;
    const LocDb& mLocDb;

}; // class ColoringByContinent

// ----------------------------------------------------------------------

class ColoringByClade : public GeographicMapColoring
{
 public:
    inline ColoringByClade(const std::map<std::string, std::string>& aCladeColor, const seqdb::Seqdb& aSeqdb)
        : mColors{aCladeColor.begin(), aCladeColor.end()}, mSeqdb(aSeqdb) {}

    virtual Color color(const hidb::AntigenData& aAntigen) const
        {
            Color result("grey50");
            try {
                const auto* entry_seq = mSeqdb.find_hi_name(aAntigen.full_name());
                if (entry_seq) {
                    for (const auto& clade: entry_seq->seq().clades()) {
                        try {
                            result = mColors.at(clade); // find first clade that has corresponding entry in mColors and use it
                        }
                        catch (...) {
                        }
                    }
                }
            }
            catch (...) {
            }
            return result;
        }

 private:
    std::map<std::string, Color> mColors;
    const seqdb::Seqdb& mSeqdb;

}; // class ColoringByClade

// ----------------------------------------------------------------------

class ColoringByLineage : public GeographicMapColoring
{
 public:
    inline ColoringByLineage(const std::map<std::string, std::string>& aLineageColor)
        : mColors{aLineageColor.begin(), aLineageColor.end()} {}

    virtual Color color(const hidb::AntigenData& aAntigen) const
        {
            try {
                return mColors.at(aAntigen.data().lineage());
            }
            catch (...) {
                return "grey50";
            }
        }

 private:
    std::map<std::string, Color> mColors;

}; // class ColoringByLineage

// ----------------------------------------------------------------------

class ColoringByLineageAndDeletionMutants : public GeographicMapColoring
{
 public:
    inline ColoringByLineageAndDeletionMutants(const std::map<std::string, std::string>& aLineageColor, std::string aDeletionMutantColor, const seqdb::Seqdb& aSeqdb)
        : mColors(aLineageColor.begin(), aLineageColor.end()), mDeletionMutantColor{aDeletionMutantColor}, mSeqdb(aSeqdb) {}

    virtual Color color(const hidb::AntigenData& aAntigen) const
        {
            try {
                const auto* entry_seq = mSeqdb.find_hi_name(aAntigen.full_name());
                if (entry_seq && entry_seq->seq().has_clade("DEL2017"))
                    return mDeletionMutantColor;
                else
                    return mColors.at(aAntigen.data().lineage());
            }
            catch (...) {
                return "grey50";
            }
        }

 private:
    std::map<std::string, Color> mColors;
    std::string mDeletionMutantColor;
    const seqdb::Seqdb& mSeqdb;

}; // class ColoringByLineage

// ----------------------------------------------------------------------

class ColorOverride : public GeographicMapColoring
{
 public:
    inline ColorOverride(const std::map<std::string, std::string>& aNameColor)
        {
            for (auto [name, color]: aNameColor) {
                if (!name.empty() && name[0] != '?' && !color.empty() && color[0] != '?')
                    mColors.emplace(name, color);
            }
        }

    virtual Color color(const hidb::AntigenData& aAntigen) const
        {
            try {
                return mColors.at(aAntigen.name());
            }
            catch (...) {
                return ColorNoChange;
            }
        }

 private:
    std::map<std::string, Color> mColors;

}; // class ColorOverride

// ----------------------------------------------------------------------

class GeographicMapWithPointsFromHidb : public GeographicMapDraw
{
 public:
    inline GeographicMapWithPointsFromHidb(const hidb::HiDb& aHiDb, const LocDb& aLocDb, double aPointSizeInPixels, double aPointDensity, std::string aOutlineColor, double aOutlineWidth)
        : GeographicMapDraw(aOutlineColor, Pixels{aOutlineWidth}), mHiDb(aHiDb), mLocDb(aLocDb), mPointSize(aPointSizeInPixels), mDensity(aPointDensity) {}

    virtual void prepare(Surface& aSurface);

    void add_points_from_hidb_colored_by(const GeographicMapColoring& aColoring, const ColorOverride& aColorOverride, std::string aStartDate, std::string aEndDate);

    inline void add_points_from_hidb_colored_by_continent(const std::map<std::string, std::string>& aContinentColor, const std::map<std::string, std::string>& aColorOverride, std::string aStartDate, std::string aEndDate) { add_points_from_hidb_colored_by(ColoringByContinent(aContinentColor, mLocDb), ColorOverride(aColorOverride), aStartDate, aEndDate); }
    inline void add_points_from_hidb_colored_by_clade(const std::map<std::string, std::string>& aCladeColor, const std::map<std::string, std::string>& aColorOverride, const seqdb::Seqdb& aSeqdb, std::string aStartDate, std::string aEndDate) { add_points_from_hidb_colored_by(ColoringByClade(aCladeColor, aSeqdb), ColorOverride(aColorOverride), aStartDate, aEndDate); }
    inline void add_points_from_hidb_colored_by_lineage(const std::map<std::string, std::string>& aLineageColor, const std::map<std::string, std::string>& aColorOverride, std::string aStartDate, std::string aEndDate) { add_points_from_hidb_colored_by(ColoringByLineage(aLineageColor), ColorOverride(aColorOverride), aStartDate, aEndDate); }

    inline const LocDb& locdb() const { return mLocDb; }

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

class GeographicTimeSeriesBase
{
 public:
    inline GeographicTimeSeriesBase(const hidb::HiDb& aHiDb, const LocDb& aLocDb, double aPointSizeInPixels, double aPointDensity, std::string aOutlineColor, double aOutlineWidth)
        : mMap(aHiDb, aLocDb, aPointSizeInPixels, aPointDensity, aOutlineColor, aOutlineWidth) {}
    virtual ~GeographicTimeSeriesBase();

    inline Title& title() { return mMap.title(); }
    virtual void draw(std::string aFilenamePrefix, const GeographicMapColoring& aColoring, const ColorOverride& aColorOverride, double aImageWidth) const = 0;
    void draw(std::string aFilenamePrefix, TimeSeriesIterator& aBegin, const TimeSeriesIterator& aEnd, const GeographicMapColoring& aColoring, const ColorOverride& aColorOverride, double aImageWidth) const;

    inline const LocDb& locdb() const { return mMap.locdb(); }

 private:
    GeographicMapWithPointsFromHidb mMap;

}; // class GeographicTimeSeriesBase

// ----------------------------------------------------------------------

template <typename TimeSeries> class GeographicTimeSeries : public GeographicTimeSeriesBase
{
 public:
    inline GeographicTimeSeries(std::string aStart, std::string aEnd, const hidb::HiDb& aHiDb, const LocDb& aLocDb, double aPointSizeInPixels, double aPointDensity, std::string aOutlineColor, double aOutlineWidth)
        : GeographicTimeSeriesBase(aHiDb, aLocDb, aPointSizeInPixels, aPointDensity, aOutlineColor, aOutlineWidth), mTS(aStart, aEnd) {}

    virtual inline void draw(std::string aFilenamePrefix, const GeographicMapColoring& aColoring, const ColorOverride& aColorOverride, double aImageWidth) const { auto start = mTS.begin(), end = mTS.end(); GeographicTimeSeriesBase::draw(aFilenamePrefix, start, end, aColoring, aColorOverride, aImageWidth); }

 private:
    TimeSeries mTS;

}; // class GeographicTimeSeries

using GeographicTimeSeriesMonthly = GeographicTimeSeries<MonthlyTimeSeries>;
using GeographicTimeSeriesYearly = GeographicTimeSeries<YearlyTimeSeries>;
using GeographicTimeSeriesWeekly = GeographicTimeSeries<WeeklyTimeSeries>;

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
