#pragma once

#include "acmacs-map-draw/mod-applicator.hh"

// ----------------------------------------------------------------------

class ModSera : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& aModData) override;

}; // class ModSera

// ----------------------------------------------------------------------

class ModSerumHomologous : public Mod
{
 public:
    using Mod::Mod;

 protected:
    size_t select_mark_serum(ChartDraw& aChartDraw, bool aVerbose);
    size_t select_serum(ChartDraw& aChartDraw, bool aVerbose) const;
    void mark_serum(ChartDraw& aChartDraw, size_t serum_index);
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
    enum class serum_circle_radius_type { theoretical, empirical };

    void make_serum_circle(ChartDraw& aChartDraw, size_t aSerumIndex, Scaled aRadius) const;
    double calculate_radius(ChartDraw& aChartDraw, size_t aSerumIndex, const std::vector<size_t>& aAntigenIndices, serum_circle_radius_type radius_type, bool aVerbose) const;

}; // class ModSerumCircle

// ----------------------------------------------------------------------

class ModSerumCoverage : public ModSerumHomologous
{
 public:
    using ModSerumHomologous::ModSerumHomologous;

    void apply(ChartDraw& aChartDraw, const rjson::value& aModData) override;
    void apply(ChartDraw& aChartDraw, const rjson::object& within_4fold, const rjson::object& outside_4fold, bool verbose);

}; // class ModSerumCoverage

// ----------------------------------------------------------------------

class ModSerumCoverageCircle : public ModSerumHomologous
{
 public:
    using ModSerumHomologous::ModSerumHomologous;

    void apply(ChartDraw& aChartDraw, const rjson::value& aModData) override;

}; // class ModSerumCoverage

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
