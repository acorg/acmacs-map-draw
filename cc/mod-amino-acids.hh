#pragma once

#include <vector>

#include "acmacs-map-draw/mod-applicator.hh"

// ----------------------------------------------------------------------

class ModAminoAcids : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& aModData) override;

 private:
    std::vector<Color> mColors; // colors to auto-assign (if "colors" not present)
    size_t mIndexDiff = 0;

    void aa_pos(ChartDraw& aChartDraw, const rjson::array& aPos, bool aVerbose);
    void aa_group(ChartDraw& aChartDraw, const rjson::object& aGroup, bool aVerbose);
    Color fill_color_default(size_t aIndex, std::string aAA);
    void make_color_for_aa(std::map<std::string, Color>& color_for_aa, const std::vector<std::string>& aa_sorted);
    
}; // class ModAminoAcids

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
