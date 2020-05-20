#pragma once

#include <string>
#include <map>

#include "acmacs-base/time-series.hh"
#include "acmacs-virus/virus-name.hh"
#include "hidb-5/hidb.hh"
#include "locationdb/locdb.hh"
#include "acmacs-map-draw/point-style-draw.hh"
#include "acmacs-map-draw/map-elements.hh"

// ----------------------------------------------------------------------

namespace acmacs::surface { class Surface; }
class GeographicMapColoring;    // private, defined in geographic-map.cc

// ----------------------------------------------------------------------

class LongLat
{
 public:
      // LongLat() = default;

    double longitude{0}, latitude{0};

}; // class LongLat

class GeographicMapPoint : public acmacs::PointStyle
{
 public:
    GeographicMapPoint() = default;
    GeographicMapPoint(const LongLat& aLongLat, long aPriority) : mLongLat(aLongLat), mPriority{aPriority} {}

    void draw(acmacs::surface::Surface& aSurface) const { draw_point(aSurface, *this, {mLongLat.longitude, mLongLat.latitude}); }

    bool operator<(const GeographicMapPoint& aNother) const { return mPriority < aNother.mPriority; }

 private:
    LongLat mLongLat;
    long mPriority{0};
};

// ----------------------------------------------------------------------

class GeographicMapPoints : public std::vector<GeographicMapPoint>
{
 public:
    void draw(acmacs::surface::Surface& aSurface) const;
    void sort() { std::sort(begin(), end()); }
};

// ----------------------------------------------------------------------

class GeographicMapDraw
{
 public:
    GeographicMapDraw(Color aOutline, Pixels aOutlineWidth) : mOutline(aOutline), mOutlineWidth(aOutlineWidth) {}
    GeographicMapDraw(const GeographicMapDraw&) = default;
    virtual ~GeographicMapDraw();

    virtual void prepare(acmacs::surface::Surface& aSurface);
    virtual void draw(std::string aFilename, double aImageWidth);

    void add_point(long aPriority, double aLat, double aLong, const acmacs::color::Modifier& aFill, Pixels aSize, const acmacs::color::Modifier& aOutline, Pixels aOutlineWidth = Pixels{0});
    map_elements::Title& title() { return mTitle; }

 protected:
    void sort_points() { mPoints.sort(); }

 private:
    Color mOutline;
    Pixels mOutlineWidth;
    GeographicMapPoints mPoints;
    map_elements::Title mTitle;

    virtual void draw(acmacs::surface::Surface& aOutlineSurface, acmacs::surface::Surface& aPointSurface) const;

}; // class GeographicMapDraw

// ----------------------------------------------------------------------

class GeographicMapColoring
{
 public:
    struct ColoringData
    {
        ColoringData() = default;
        ColoringData(Color aFill, Color aOutline = BLACK, double aOutlineWidth = 0) : fill{aFill}, outline{aOutline}, outline_width{aOutlineWidth} {}
        ColoringData(const acmacs::color::Modifier& aFill) : fill{aFill} {}
        // ColoringData(std::string aFill, std::string aOutline = "black", double aOutlineWidth = 0) : fill{aFill}, outline{aOutline}, outline_width{aOutlineWidth} {}
        ColoringData(std::string_view aFill, std::string_view aOutline = "black", double aOutlineWidth = 0) : fill{aFill}, outline{aOutline}, outline_width{aOutlineWidth} {}
        bool operator<(const ColoringData& aNother) const { return fill == aNother.fill ? (outline == aNother.outline ? outline_width < aNother.outline_width : outline < aNother.outline) : fill < aNother.fill; }
        ColoringData& operator=(std::string_view a_fill) { fill = acmacs::color::Modifier{a_fill}; return *this; }

        acmacs::color::Modifier fill; // no change by default
        acmacs::color::Modifier outline{BLACK};
        Pixels outline_width{0};
    };

    using TagToColor = std::map<std::string, ColoringData, std::less<>>;
    using TagColor = std::pair<std::string, ColoringData>;

    virtual ~GeographicMapColoring();

    virtual TagColor color(const hidb::Antigen& aAntigen) const = 0;
};

// ----------------------------------------------------------------------

class ColoringByContinent : public GeographicMapColoring
{
 public:
    ColoringByContinent(const std::map<std::string, std::string>& aContinentColor) : mColors{aContinentColor.begin(), aContinentColor.end()} {}
    ColoringByContinent(const TagToColor& aContinentColor) : mColors{aContinentColor.begin(), aContinentColor.end()} {}

    TagColor color(const hidb::Antigen& aAntigen) const override;

 private:
    TagToColor mColors;

}; // class ColoringByContinent

// ----------------------------------------------------------------------

class ColoringByClade : public GeographicMapColoring
{
 public:
    ColoringByClade(const std::map<std::string, std::string>& aCladeColor) : mColors{aCladeColor.begin(), aCladeColor.end()} {}
    ColoringByClade(const TagToColor& aCladeColor) : mColors{aCladeColor.begin(), aCladeColor.end()} {}

    TagColor color(const hidb::Antigen& aAntigen) const override;

 private:
    TagToColor mColors;

}; // class ColoringByClade

// ----------------------------------------------------------------------

class ColoringByLineage : public GeographicMapColoring
{
 public:
    ColoringByLineage(const std::map<std::string, std::string>& aLineageColor) : mColors{aLineageColor.begin(), aLineageColor.end()} {}
    ColoringByLineage(const TagToColor& aLineageColor) : mColors{aLineageColor.begin(), aLineageColor.end()} {}

    TagColor color(const hidb::Antigen& aAntigen) const override
        {
            try {
                const std::string lineage{aAntigen.lineage().to_string()};
                return {lineage, mColors.at(lineage)};
            }
            catch (...) {
                return {"UNKNOWN", ColoringData{GREY50}};
            }
        }

 private:
    TagToColor mColors;

}; // class ColoringByLineage

// ----------------------------------------------------------------------

class ColoringByLineageAndDeletionMutants : public GeographicMapColoring
{
 public:
    ColoringByLineageAndDeletionMutants(const std::map<std::string, std::string>& aLineageColor, std::string aDeletionMutantColor = std::string{})
        : mColors(aLineageColor.begin(), aLineageColor.end()), mDeletionMutantColor{aDeletionMutantColor} {}
    ColoringByLineageAndDeletionMutants(const TagToColor& aLineageColor, std::string aDeletionMutantColor = std::string{})
        : mColors(aLineageColor.begin(), aLineageColor.end()), mDeletionMutantColor{aDeletionMutantColor} {}

    TagColor color(const hidb::Antigen& aAntigen) const override;

 private:
    TagToColor mColors;
    std::string mDeletionMutantColor;

}; // class ColoringByLineage

// ----------------------------------------------------------------------

class ColoringByAminoAcid : public GeographicMapColoring
{
 public:
    ColoringByAminoAcid(const rjson::value& settings) : settings_(settings) {}

    TagColor color(const hidb::Antigen& aAntigen) const override;

 private:
    rjson::value settings_;

}; // class ColoringByLineage

// ----------------------------------------------------------------------

class ColorOverride : public GeographicMapColoring
{
 public:
    ColorOverride() = default;
    ColorOverride(const std::map<std::string, std::string>& aNameColor)
        {
            for (auto [name, color]: aNameColor) {
                if (!name.empty() && name[0] != '?' && !color.empty() && color[0] != '?')
                    mColors.emplace(name, color);
            }
        }

    TagColor color(const hidb::Antigen& aAntigen) const override
        {
            try {
                return TagColor{aAntigen.name(), mColors.at(*aAntigen.name())};
            }
            catch (...) {
                return TagColor{"UNKNOWN", ColoringData{}};
            }
        }

 private:
    std::map<std::string, Color> mColors;

}; // class ColorOverride

// ----------------------------------------------------------------------

class GeographicMapWithPointsFromHidb : public GeographicMapDraw
{
 public:
    // GeographicMapWithPointsFromHidb(std::string aVirusType, double aPointSizeInPixels, double aPointDensity, std::string aOutlineColor, double aOutlineWidth)
    //     : GeographicMapDraw(Color(aOutlineColor), Pixels{aOutlineWidth}), mVirusType{aVirusType}, mPointSize(aPointSizeInPixels), mDensity(aPointDensity) {}
    GeographicMapWithPointsFromHidb(std::string aVirusType, double aPointSizeInPixels, double aPointDensity, Color aOutlineColor, double aOutlineWidth)
        : GeographicMapDraw(aOutlineColor, Pixels{aOutlineWidth}), mVirusType{aVirusType}, mPointSize(aPointSizeInPixels), mDensity(aPointDensity) {}

    virtual void prepare(acmacs::surface::Surface& aSurface);

    void add_points_from_hidb_colored_by(const GeographicMapColoring& aColoring, const ColorOverride& aColorOverride, const std::vector<std::string>& aPriority, std::string_view aStartDate, std::string_view aEndDate);

    // void add_points_from_hidb_colored_by_continent(const GeographicMapColoring::TagToColor& aContinentColor, const std::map<std::string, std::string>& aColorOverride, const std::vector<std::string>& aPriority, std::string aStartDate, std::string aEndDate) { add_points_from_hidb_colored_by(ColoringByContinent(aContinentColor), ColorOverride(aColorOverride), aPriority, aStartDate, aEndDate); }
    // void add_points_from_hidb_colored_by_clade(const GeographicMapColoring::TagToColor& aCladeColor, const std::map<std::string, std::string>& aColorOverride, const std::vector<std::string>& aPriority, std::string aStartDate, std::string aEndDate) { add_points_from_hidb_colored_by(ColoringByClade(aCladeColor), ColorOverride(aColorOverride), aPriority, aStartDate, aEndDate); }
    // void add_points_from_hidb_colored_by_lineage(const GeographicMapColoring::TagToColor& aLineageColor, const std::map<std::string, std::string>& aColorOverride, const std::vector<std::string>& aPriority, std::string aStartDate, std::string aEndDate) { add_points_from_hidb_colored_by(ColoringByLineage(aLineageColor), ColorOverride(aColorOverride), aPriority, aStartDate, aEndDate); }
    // void add_points_from_hidb_colored_by_lineage_and_deletion_mutants(const GeographicMapColoring::TagToColor& aLineageColor, const std::map<std::string, std::string>& aColorOverride, const std::vector<std::string>& aPriority, std::string aStartDate, std::string aEndDate) { add_points_from_hidb_colored_by(ColoringByLineageAndDeletionMutants(aLineageColor), ColorOverride(aColorOverride), aPriority, aStartDate, aEndDate); }

    void add_points_from_hidb_colored_by_continent_old(const std::map<std::string, std::string>& aContinentColor, const std::map<std::string, std::string>& aColorOverride, const std::vector<std::string>& aPriority, std::string aStartDate, std::string aEndDate) { add_points_from_hidb_colored_by(ColoringByContinent(aContinentColor), ColorOverride(aColorOverride), aPriority, aStartDate, aEndDate); }
    void add_points_from_hidb_colored_by_clade_old(const std::map<std::string, std::string>& aCladeColor, const std::map<std::string, std::string>& aColorOverride, const std::vector<std::string>& aPriority, std::string aStartDate, std::string aEndDate) { add_points_from_hidb_colored_by(ColoringByClade(aCladeColor), ColorOverride(aColorOverride), aPriority, aStartDate, aEndDate); }
    void add_points_from_hidb_colored_by_lineage_old(const std::map<std::string, std::string>& aLineageColor, const std::map<std::string, std::string>& aColorOverride, const std::vector<std::string>& aPriority, std::string aStartDate, std::string aEndDate) { add_points_from_hidb_colored_by(ColoringByLineage(aLineageColor), ColorOverride(aColorOverride), aPriority, aStartDate, aEndDate); }
    void add_points_from_hidb_colored_by_lineage_and_deletion_mutants_old(const std::map<std::string, std::string>& aLineageColor, const std::map<std::string, std::string>& aColorOverride, const std::vector<std::string>& aPriority, std::string aStartDate, std::string aEndDate) { add_points_from_hidb_colored_by(ColoringByLineageAndDeletionMutants(aLineageColor), ColorOverride(aColorOverride), aPriority, aStartDate, aEndDate); }

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
        const auto& operator*() const { return mCurrent->first; }
        void operator++() { if (mCurrent != mEnd) { ++mCurrentCount; if (mCurrentCount >= mCurrent->second) { ++mCurrent; mCurrentCount = 0; } } }
        size_t left() const { return std::accumulate(mCurrent, mEnd, 0U, [](size_t sum, auto elt) -> size_t {return sum + elt.second; }) - mCurrentCount; }
        operator bool() const { return mCurrent != mEnd; }

     private:
        PointsAtLocation::const_iterator mCurrent;
        PointsAtLocation::const_iterator mEnd;
        size_t mCurrentCount;

        friend class PointsAtLocation;
        PointsAtLocationIterator(const PointsAtLocation& aSource) : mCurrent(aSource.begin()), mEnd(aSource.end()), mCurrentCount(0) {}
    };

    class Points : public std::map<std::string, PointsAtLocation>
    {
          // location-name to color to number-of-points
     public:
        void add(std::string aLocation, long aPriority, GeographicMapColoring::ColoringData aColoringData) { ++(*this)[aLocation][{aColoringData, aPriority}]; }
          // static size_t number_of_points_at_location(const PointsAtLocation& colors) { return std::accumulate(colors.begin(), colors.end(), 0U, [](size_t sum, auto elt) -> size_t {return sum + elt.second; }); }
    };

    Points mPointsAtLocation;

}; // class GeographicMapWithPointsFromHidb

// ----------------------------------------------------------------------

class GeographicTimeSeries
{
 public:
    GeographicTimeSeries(const acmacs::time_series::parameters& params, std::string aVirusType, const std::vector<std::string>& priority, double aPointSizeInPixels, double aPointDensity, Color aOutlineColor, double aOutlineWidth)
        : map_(aVirusType, aPointSizeInPixels, aPointDensity, aOutlineColor, aOutlineWidth), time_series_{acmacs::time_series::make(params)}, priority_{priority} {}
    virtual ~GeographicTimeSeries() = default;

    map_elements::Title& title() { return map_.title(); }
    void draw(std::string_view aFilenamePrefix, const GeographicMapColoring& aColoring, const ColorOverride& aColorOverride, double aImageWidth) const;

 private:
    GeographicMapWithPointsFromHidb map_;
    acmacs::time_series::series time_series_;
    const std::vector<std::string> priority_;

}; // class GeographicTimeSeries

// ----------------------------------------------------------------------

class GeographicTimeSeriesMonthly : public GeographicTimeSeries
{
  public:
    GeographicTimeSeriesMonthly(std::string aVirusType, std::string_view aStart, std::string_view aEnd, const std::vector<std::string>& aPriority, double aPointSizeInPixels, double aPointDensity, Color aOutlineColor, double aOutlineWidth)
        : GeographicTimeSeries({date::from_string(aStart), date::from_string(aEnd), acmacs::time_series::interval::month}, aVirusType, aPriority, aPointSizeInPixels, aPointDensity, aOutlineColor, aOutlineWidth) {}
};

class GeographicTimeSeriesYearly : public GeographicTimeSeries
{
  public:
    GeographicTimeSeriesYearly(std::string aVirusType, std::string_view aStart, std::string_view aEnd, const std::vector<std::string>& aPriority, double aPointSizeInPixels, double aPointDensity, Color aOutlineColor, double aOutlineWidth)
        : GeographicTimeSeries({date::from_string(aStart), date::from_string(aEnd), acmacs::time_series::interval::year}, aVirusType, aPriority, aPointSizeInPixels, aPointDensity, aOutlineColor, aOutlineWidth) {}
};

class GeographicTimeSeriesWeekly : public GeographicTimeSeries
{
  public:
    GeographicTimeSeriesWeekly(std::string aVirusType, std::string_view aStart, std::string_view aEnd, const std::vector<std::string>& aPriority, double aPointSizeInPixels, double aPointDensity, Color aOutlineColor, double aOutlineWidth)
        : GeographicTimeSeries({date::from_string(aStart), date::from_string(aEnd), acmacs::time_series::interval::week}, aVirusType, aPriority, aPointSizeInPixels, aPointDensity, aOutlineColor, aOutlineWidth) {}
};

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
