#pragma once

#include "acmacs-map-draw/mod-applicator.hh"

// ----------------------------------------------------------------------

class ModSerumHomologous : public Mod
{
 public:
    using Mod::Mod;

 protected:
    size_t select_mark_serum(ChartDraw& aChartDraw, bool aVerbose);
    size_t select_serum(ChartDraw& aChartDraw, bool aVerbose) const;
    std::vector<size_t> select_mark_antigens(ChartDraw& aChartDraw, size_t aSerumIndex, bool aVerbose);
    std::vector<size_t> select_antigens(ChartDraw& aChartDraw, size_t aSerumIndex, bool aVerbose) const;
    std::vector<size_t> select_homologous_antigens(ChartDraw& aChartDraw, size_t aSerumIndex, bool aVerbose) const;

}; // class ModSerumHomologous

// ----------------------------------------------------------------------

class ModSerumCircle : public ModSerumHomologous
{
 public:
    using ModSerumHomologous::ModSerumHomologous;

    void apply(ChartDraw& aChartDraw, const rjson::value& aModData) override;

 protected:
    void make_serum_circle(ChartDraw& aChartDraw, size_t aSerumIndex, Scaled aRadius) const;
    double calculate_radius(ChartDraw& aChartDraw, size_t aSerumIndex, const std::vector<size_t>& aAntigenIndices, bool aVerbose) const;

}; // class ModSerumCircle

// ----------------------------------------------------------------------

class ModSerumCoverage : public ModSerumHomologous
{
 public:
    using ModSerumHomologous::ModSerumHomologous;

    void apply(ChartDraw& aChartDraw, const rjson::value& aModData) override;

}; // class ModSerumCoverage

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
