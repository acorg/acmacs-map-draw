#pragma once

#include <stdexcept>

#include "acmacs-base/rjson-forward.hh"
#include "acmacs-base/point-style.hh"
#include "acmacs-chart-2/point-index-list.hh"
#include "acmacs-map-draw/point-style-draw.hh"

// ----------------------------------------------------------------------

class ChartDraw;

// aMods is a list of:
//  - string with mod name, e.g. "all_grey"
//  - object: {"N": "mod-name", mod parameters}
// aModData is an object for mod parameters and non-standard mod lookup (see settings.cc):
//  {"mod-name": [<mods to apply>]}

void apply_mods(ChartDraw& aChartDraw, const rjson::value& aMods, const rjson::value& aModData, bool aIgnoreUnrecognized = false);

class unrecognized_mod : public std::runtime_error { public: using std::runtime_error::runtime_error; };

// ----------------------------------------------------------------------

class Mod
{
 public:
    Mod() {}
    Mod(const rjson::value& aArgs) : mArgs{aArgs} {}
    virtual ~Mod() { /* std::cerr << "~Mod " << args() << '\n'; */ }

    virtual void apply(ChartDraw& aChartDraw, const rjson::value& aModData) = 0;

 protected:
    const rjson::value& args() const { return mArgs; }

    acmacs::PointStyle style() const { return point_style_from_json(args()); }
    PointDrawingOrder drawing_order() const { return drawing_order_from_json(args()); }
    void add_labels(ChartDraw& aChartDraw, const acmacs::chart::PointIndexList& aIndices, size_t aBaseIndex, const rjson::value& aLabelData);
    void add_label(ChartDraw& aChartDraw, size_t aIndex, size_t aBaseIndex, const rjson::value& aLabelData);
    void add_legend(ChartDraw& aChartDraw, const acmacs::chart::PointIndexList& aIndices, const acmacs::PointStyle& aStyle, const rjson::value& aLegendData);
    void add_legend(ChartDraw& aChartDraw, const acmacs::chart::PointIndexList& aIndices, const acmacs::PointStyle& aStyle, std::string aLabel, const rjson::value& aLegendData);

 private:
    const rjson::value mArgs;  // not reference! "N" is probably wrong due to updating args in factory!

    friend inline std::ostream& operator << (std::ostream& out, const Mod& aMod) { return out << aMod.args(); }

}; // class Mod

// ----------------------------------------------------------------------

class ModAntigens : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& aModData) override;

}; // class ModAntigens

// ----------------------------------------------------------------------

class ModMoveBase : public Mod
{
 public:
    using Mod::Mod;

 protected:
    acmacs::PointCoordinates get_move_to(ChartDraw& aChartDraw, bool aVerbose) const;

}; // class ModMoveBase

// ----------------------------------------------------------------------

class ModMoveAntigens : public ModMoveBase
{
 public:
    using ModMoveBase::ModMoveBase;

    void apply(ChartDraw& aChartDraw, const rjson::value& aModData) override;

}; // class ModMoveAntigens

// ----------------------------------------------------------------------

class ModMoveAntigensStress : public ModMoveBase
{
 public:
    using ModMoveBase::ModMoveBase;

    void apply(ChartDraw& aChartDraw, const rjson::value& aModData) override;

}; // class ModMoveAntigens

// ----------------------------------------------------------------------

class ModMoveSera : public ModMoveBase
{
 public:
    using ModMoveBase::ModMoveBase;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override;

}; // class ModMoveSera

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
