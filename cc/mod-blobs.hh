#pragma once

#include "acmacs-map-draw/mod-applicator.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

class ModBlobs : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& aModData) override;

}; // class ModBlobs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
