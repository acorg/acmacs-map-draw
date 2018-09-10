#pragma once

#include "acmacs-map-draw/mod-applicator.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

class ModProcrustesArrows : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::v1::value& aModData) override;

}; // class ModProcrustesArrows

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
