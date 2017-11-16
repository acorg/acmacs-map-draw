#pragma once

#include <string>
#include <map>

#include "acmacs-base/time-series.hh"
#include "hidb/hidb.hh"
#include "seqdb/seqdb.hh"
#include "locationdb/locdb.hh"
#include "acmacs-chart-2/layout.hh"
#include "acmacs-map-draw/point-style-draw.hh"
#include "acmacs-map-draw/map-elements.hh"

// ----------------------------------------------------------------------

class Surface;
class GeographicMapColoring;    // private, defined in geographic-map.cc

// ----------------------------------------------------------------------

class GeographicMapPoint : public PointStyleDraw
{
 public:
    inline GeographicMapPoint() = default; // : mLongLat{0, 0} {}
    inline GeographicMapPoint(const acmacs::chart::Coordinates& aLongLat, long aPriority) : mLongLat(aLongLat), mPriority{aPriority} {}

    inline void draw(Surface& aSurface) const { PointStyleDraw::draw(aSurface, mLongLat); }

    inline bool operator<(const GeographicMapPoint& aNother) const { return mPriority < aNother.mPriority; }

 private:
    acmacs::chart::Coordinates mLongLat{0, 0};
    long mPriority{0};
};

// ----------------------------------------------------------------------

class GeographicMapPoints : public std::vector<GeographicMapPoint>
{
 public:
    void draw(Surface& aSurface) const;
    inline void sort() { std::sort(begin(), end()); }
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

    void add_point(long aPriority, double aLat, double aLong, Color aFill, Pixels aSize, Color aOutline = "transparent", Pixels aOutlineWidth = Pixels{0});
    inline map_elements::Title& title() { return mTitle; }

 protected:
    inline void sort_points() { mPoints.sort(); }

 private:
    Color mOutline;
    Pixels mOutlineWidth;
    GeographicMapPoints mPoints;
    map_elements::Title mTitle;

    virtual void draw(Surface& aOutlineSurface, Surface& aPointSurface) const;

}; // class GeographicMapDraw

// ----------------------------------------------------------------------

class GeographicMapColoring
{
 public:
    struct ColoringData
    {
        inline ColoringData() = default;
          // inline ColoringData(Color aFill, Color aOutline = "black", double aOutlineWidth = 0) : fill{aFill}, outline{aOutline}, outline_width{aOutlineWidth} {}
        inline ColoringData(std::string aFill, std::string aOutline = "black", double aOutlineWidth = 0) : fill{aFill}, outline{aOutline}, outline_width{aOutlineWidth} {}
        inline bool operator<(const ColoringData& aNother) const { return fill == aNother.fill ? (outline == aNother.outline ? outline_width < aNother.outline_width : outline < aNother.outline) : fill < aNother.fill; }

        Color fill;
        Color outline{"black"};
        Pixels outline_width{0};
    };

    using TagToColor = std::map<std::string, ColoringData>;
    using TagColor = std::pair<std::string, ColoringData>;

    virtual ~GeographicMapColoring();

    virtual TagColor color(const hidb::AntigenData& aAntigen) const = 0;
};

// ----------------------------------------------------------------------

class ColoringByContinent : public GeographicMapColoring
{
 public:
    inline ColoringByContinent(const std::map<std::string, std::string>& aContinentColor) : mColors{aContinentColor.begin(), aContinentColor.end()} {}
    inline ColoringByContinent(const TagToColor& aContinentColor) : mColors{aContinentColor.begin(), aContinentColor.end()} {}

    TagColor color(const hidb::AntigenData& aAntigen) const override
        {
            try {
                auto continent = get_locdb().continent(virus_name::location(aAntigen.data().name()));
                return {continent, mColors.at(continent)};
            }
            catch (...) {
                return {"UNKNOWN", {"grey50"}};
            }
        }

 private:
    TagToColor mColors;

}; // class ColoringByContinent

// ----------------------------------------------------------------------

class ColoringByClade : public GeographicMapColoring
{
 public:
    inline ColoringByClade(const std::map<std::string, std::string>& aCladeColor) : mColors{aCladeColor.begin(), aCladeColor.end()} {}
    inline ColoringByClade(const TagToColor& aCladeColor) : mColors{aCladeColor.begin(), aCladeColor.end()} {}

    TagColor color(const hidb::AntigenData& aAntigen) const override
        {
            ColoringData result("grey50");
            std::string tag{"UNKNOWN"};
            try {
                const auto* entry_seq = seqdb::get().find_hi_name(aAntigen.full_name());
                if (entry_seq) {
                    for (const auto& clade: entry_seq->seq().clades()) {
                        try {
                            result = mColors.at(clade); // find first clade that has corresponding entry in mColors and use it
                            tag = clade;
                            break;
                        }
                        catch (...) {
                        }
                    }
                }
            }
            catch (...) {
            }
            return {tag, result};
        }

 private:
    TagToColor mColors;

}; // class ColoringByClade

// ----------------------------------------------------------------------

class ColoringByLineage : public GeographicMapColoring
{
 public:
    inline ColoringByLineage(const std::map<std::string, std::string>& aLineageColor) : mColors{aLineageColor.begin(), aLineageColor.end()} {}
    inline ColoringByLineage(const TagToColor& aLineageColor) : mColors{aLineageColor.begin(), aLineageColor.end()} {}

    TagColor color(const hidb::AntigenData& aAntigen) const override
        {
            try {
                std::string lineage{aAntigen.data().lineage()};
                return {lineage, mColors.at(lineage)};
            }
            catch (...) {
                return {"UNKNOWN", {"grey50"}};
            }
        }

 private:
    TagToColor mColors;

}; // class ColoringByLineage

// ----------------------------------------------------------------------

class ColoringByLineageAndDeletionMutants : public GeographicMapColoring
{
 public:
    inline ColoringByLineageAndDeletionMutants(const std::map<std::string, std::string>& aLineageColor, std::string aDeletionMutantColor = std::string{})
        : mColors(aLineageColor.begin(), aLineageColor.end()), mDeletionMutantColor{aDeletionMutantColor} {}
    inline ColoringByLineageAndDeletionMutants(const TagToColor& aLineageColor, std::string aDeletionMutantColor = std::string{})
        : mColors(aLineageColor.begin(), aLineageColor.end()), mDeletionMutantColor{aDeletionMutantColor} {}

    TagColor color(const hidb::AntigenData& aAntigen) const override
        {
            try {
                const auto* entry_seq = seqdb::get().find_hi_name(aAntigen.full_name());
                if (entry_seq && entry_seq->seq().has_clade("DEL2017"))
                    return {"VICTORIA_DEL", mDeletionMutantColor.empty() ? mColors.at("VICTORIA_DEL") : ColoringData{mDeletionMutantColor}};
                else {
                    std::string lineage{aAntigen.data().lineage()};
                    return {lineage, mColors.at(lineage)};
                }
            }
            catch (...) {
                return {"UNKNOWN", {"grey50"}};
            }
        }

 private:
    TagToColor mColors;
    std::string mDeletionMutantColor;

}; // class ColoringByLineage

// ----------------------------------------------------------------------

class ColorOverride : public GeographicMapColoring
{
 public:
    inline ColorOverride() = default;
    inline ColorOverride(const std::map<std::string, std::string>& aNameColor)
        {
            for (auto [name, color]: aNameColor) {
                if (!name.empty() && name[0] != '?' && !color.empty() && color[0] != '?')
                    mColors.emplace(name, color);
            }
        }

    TagColor color(const hidb::AntigenData& aAntigen) const override
        {
            try {
                return TagColor{aAntigen.name(), mColors.at(aAntigen.name())};
            }
            catch (...) {
                return TagColor{"UNKNOWN", {ColorNoChange}};
            }
        }

 private:
    std::map<std::string, Color> mColors;

}; // class ColorOverride

// ----------------------------------------------------------------------

class GeographicMapWithPointsFromHidb : public GeographicMapDraw
{
 public:
    inline GeographicMapWithPointsFromHidb(std::string aVirusType, double aPointSizeInPixels, double aPointDensity, std::string aOutlineColor, double aOutlineWidth)
        : GeographicMapDraw(aOutlineColor, Pixels{aOutlineWidth}), mVirusType{aVirusType}, mPointSize(aPointSizeInPixels), mDensity(aPointDensity) {}

    virtual void prepare(Surface& aSurface);

    void add_points_from_hidb_colored_by(const GeographicMapColoring& aColoring, const ColorOverride& aColorOverride, const std::vector<std::string>& aPriority, std::string aStartDate, std::string aEndDate);

    // inline void add_points_from_hidb_colored_by_continent(const GeographicMapColoring::TagToColor& aContinentColor, const std::map<std::string, std::string>& aColorOverride, const std::vector<std::string>& aPriority, std::string aStartDate, std::string aEndDate) { add_points_from_hidb_colored_by(ColoringByContinent(aContinentColor), ColorOverride(aColorOverride), aPriority, aStartDate, aEndDate); }
    // inline void add_points_from_hidb_colored_by_clade(const GeographicMapColoring::TagToColor& aCladeColor, const std::map<std::string, std::string>& aColorOverride, const std::vector<std::string>& aPriority, std::string aStartDate, std::string aEndDate) { add_points_from_hidb_colored_by(ColoringByClade(aCladeColor), ColorOverride(aColorOverride), aPriority, aStartDate, aEndDate); }
    // inline void add_points_from_hidb_colored_by_lineage(const GeographicMapColoring::TagToColor& aLineageColor, const std::map<std::string, std::string>& aColorOverride, const std::vector<std::string>& aPriority, std::string aStartDate, std::string aEndDate) { add_points_from_hidb_colored_by(ColoringByLineage(aLineageColor), ColorOverride(aColorOverride), aPriority, aStartDate, aEndDate); }
    // inline void add_points_from_hidb_colored_by_lineage_and_deletion_mutants(const GeographicMapColoring::TagToColor& aLineageColor, const std::map<std::string, std::string>& aColorOverride, const std::vector<std::string>& aPriority, std::string aStartDate, std::string aEndDate) { add_points_from_hidb_colored_by(ColoringByLineageAndDeletionMutants(aLineageColor), ColorOverride(aColorOverride), aPriority, aStartDate, aEndDate); }

    inline void add_points_from_hidb_colored_by_continent_old(const std::map<std::string, std::string>& aContinentColor, const std::map<std::string, std::string>& aColorOverride, const std::vector<std::string>& aPriority, std::string aStartDate, std::string aEndDate) { add_points_from_hidb_colored_by(ColoringByContinent(aContinentColor), ColorOverride(aColorOverride), aPriority, aStartDate, aEndDate); }
    inline void add_points_from_hidb_colored_by_clade_old(const std::map<std::string, std::string>& aCladeColor, const std::map<std::string, std::string>& aColorOverride, const std::vector<std::string>& aPriority, std::string aStartDate, std::string aEndDate) { add_points_from_hidb_colored_by(ColoringByClade(aCladeColor), ColorOverride(aColorOverride), aPriority, aStartDate, aEndDate); }
    inline void add_points_from_hidb_colored_by_lineage_old(const std::map<std::string, std::string>& aLineageColor, const std::map<std::string, std::string>& aColorOverride, const std::vector<std::string>& aPriority, std::string aStartDate, std::string aEndDate) { add_points_from_hidb_colored_by(ColoringByLineage(aLineageColor), ColorOverride(aColorOverride), aPriority, aStartDate, aEndDate); }
    inline void add_points_from_hidb_colored_by_lineage_and_deletion_mutants_old(const std::map<std::string, std::string>& aLineageColor, const std::map<std::string, std::string>& aColorOverride, const std::vector<std::string>& aPriority, std::string aStartDate, std::string aEndDate) { add_points_from_hidb_colored_by(ColoringByLineageAndDeletionMutants(aLineageColor), ColorOverride(aColorOverride), aPriority, aStartDate, aEndDate); }

 private:
    std::string mVirusType;
    Pixels mPointSize;
    double mDensity;

    class PointsAtLocationIterator;

    class PointsAtLocation : public std::map<std::pair<GeographicMapColoring::ColoringData, long>, size_t>
    {
     public:
        PointsAtLocationIterator iterator() const { return PointsAtLocationIterator(*this); }
    };

    class PointsAtLocationIterator
    {
     public:
        inline const auto& operator*() const { return mCurrent->first; }
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
        inline void add(std::string aLocation, long aPriority, GeographicMapColoring::ColoringData aColoringData) { ++(*this)[aLocation][{aColoringData, aPriority}]; }
          // static inline size_t number_of_points_at_location(const PointsAtLocation& colors) { return std::accumulate(colors.begin(), colors.end(), 0U, [](size_t sum, auto elt) -> size_t {return sum + elt.second; }); }
    };

    Points mPointsAtLocation;

}; // class GeographicMapWithPointsFromHidb

// ----------------------------------------------------------------------

class GeographicTimeSeriesBase
{
 public:
    inline GeographicTimeSeriesBase(std::string aVirusType, double aPointSizeInPixels, double aPointDensity, std::string aOutlineColor, double aOutlineWidth)
        : mMap(aVirusType, aPointSizeInPixels, aPointDensity, aOutlineColor, aOutlineWidth) {}
    virtual ~GeographicTimeSeriesBase();

    inline map_elements::Title& title() { return mMap.title(); }
    virtual void draw(std::string aFilenamePrefix, const GeographicMapColoring& aColoring, const ColorOverride& aColorOverride, double aImageWidth) const = 0;
    void draw(std::string aFilenamePrefix, TimeSeriesIterator& aBegin, const TimeSeriesIterator& aEnd, const std::vector<std::string>& aPriority, const GeographicMapColoring& aColoring, const ColorOverride& aColorOverride, double aImageWidth) const;

 private:
    GeographicMapWithPointsFromHidb mMap;

}; // class GeographicTimeSeriesBase

// ----------------------------------------------------------------------

template <typename TimeSeries> class GeographicTimeSeries : public GeographicTimeSeriesBase
{
 public:
    inline GeographicTimeSeries(std::string aVirusType, std::string aStart, std::string aEnd, const std::vector<std::string>& aPriority, double aPointSizeInPixels, double aPointDensity, std::string aOutlineColor, double aOutlineWidth)
        : GeographicTimeSeriesBase(aVirusType, aPointSizeInPixels, aPointDensity, aOutlineColor, aOutlineWidth), mTS(aStart, aEnd), mPriority(aPriority) {}

    inline void draw(std::string aFilenamePrefix, const GeographicMapColoring& aColoring, const ColorOverride& aColorOverride, double aImageWidth) const override
        {
            auto start = mTS.begin(), end = mTS.end();
            GeographicTimeSeriesBase::draw(aFilenamePrefix, start, end, mPriority, aColoring, aColorOverride, aImageWidth);
        }

 private:
    TimeSeries mTS;
    const std::vector<std::string> mPriority;

}; // class GeographicTimeSeries

using GeographicTimeSeriesMonthly = GeographicTimeSeries<MonthlyTimeSeries>;
using GeographicTimeSeriesYearly = GeographicTimeSeries<YearlyTimeSeries>;
using GeographicTimeSeriesWeekly = GeographicTimeSeries<WeeklyTimeSeries>;

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
