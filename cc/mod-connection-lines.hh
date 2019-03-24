#pragma once

#include "acmacs-map-draw/mod-applicator.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

class ModConnectionLines : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& aModData) override;

}; // class ModConnectionLines

// ----------------------------------------------------------------------

class ModColorByNumberOfConnectionLines : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& aModData) override;

}; // class ModColorByNumberOfConnectionLines

// ----------------------------------------------------------------------

class ModErrorLines : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& aModData) override;

}; // class ModErrorLines

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
