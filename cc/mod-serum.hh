#pragma once

#include "acmacs-chart-2/point-index-list.hh"
#include "acmacs-chart-2/chart.hh"
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
    acmacs::chart::PointIndexList select_mark_sera(ChartDraw& aChartDraw, bool aVerbose);
    size_t select_serum(ChartDraw& aChartDraw, bool aVerbose) const;
    void mark_serum(ChartDraw& aChartDraw, size_t serum_index);
    acmacs::chart::PointIndexList select_mark_antigens(ChartDraw& aChartDraw, size_t aSerumIndex, acmacs::chart::find_homologous find_homologous_options, bool aVerbose);
    acmacs::chart::PointIndexList select_antigens(ChartDraw& aChartDraw, size_t aSerumIndex, acmacs::chart::find_homologous find_homologous_options, bool aVerbose) const;
    acmacs::chart::PointIndexList select_homologous_antigens(ChartDraw& aChartDraw, size_t aSerumIndex, acmacs::chart::find_homologous find_homologous_options, bool aVerbose) const;

}; // class ModSerumHomologous

// ----------------------------------------------------------------------

class ModSerumCircle : public ModSerumHomologous
{
 public:
    using ModSerumHomologous::ModSerumHomologous;

    void apply(ChartDraw& aChartDraw, const rjson::value& aModData) override;

    enum class serum_circle_radius_type { theoretical, empirical };
    static serum_circle_radius_type radius_type_from_string(std::string radius_type_s);

    void make_serum_circle(ChartDraw& aChartDraw, size_t aSerumIndex, const acmacs::chart::PointIndexList& aAntigenIndices, serum_circle_radius_type radius_type, const rjson::value& circle_plot_spec, double fold, bool verbose) const;
    void make_serum_circle(ChartDraw& aChartDraw, size_t aSerumIndex, const acmacs::chart::PointIndexList& aAntigenIndices, const rjson::value& homologous_titer, serum_circle_radius_type radius_type, const rjson::value& circle_plot_spec, double fold, bool verbose) const;

  protected:
    void make_serum_circle(ChartDraw& aChartDraw, size_t aSerumIndex, const acmacs::chart::PointIndexList& antigen_indices, acmacs::chart::Titer aHomologousTiter, const rjson::value& empirical_plot_spec, const rjson::value& theoretical_plot_spec, const rjson::value& fallback_plot_spec, double fold, bool verbose) const;
    void make_serum_circle(ChartDraw& aChartDraw, size_t aSerumIndex, const acmacs::chart::PointIndexList& aAntigenIndices, const rjson::value& empirical_plot_spec, const rjson::value& theoretical_plot_spec, const rjson::value& fallback_plot_spec, double fold, bool verbose) const;
    void make_serum_circle(ChartDraw& aChartDraw, size_t aSerumIndex, Scaled aRadius, const rjson::value& circle_plot_spec) const;
    double calculate_radius(ChartDraw& aChartDraw, size_t aSerumIndex, const acmacs::chart::PointIndexList& aAntigenIndices, serum_circle_radius_type radius_type, double fold, bool aVerbose) const;
    double calculate_radius(ChartDraw& aChartDraw, size_t aSerumIndex, const acmacs::chart::PointIndexList& aAntigenIndices, acmacs::chart::Titer aHomologousTiter, serum_circle_radius_type radius_type, double fold, bool aVerbose) const;

}; // class ModSerumCircle

// ----------------------------------------------------------------------

class ModSerumCircles : public ModSerumCircle
{
 public:
    using ModSerumCircle::ModSerumCircle;

    void apply(ChartDraw& aChartDraw, const rjson::value& aModData) override;
};

// ======================================================================

class ModSerumCoverage : public ModSerumHomologous
{
 public:
    using ModSerumHomologous::ModSerumHomologous;

    void apply(ChartDraw& aChartDraw, const rjson::value& aModData) override;
    void apply(ChartDraw& aChartDraw, size_t serum_index, const acmacs::chart::PointIndexList& antigen_indices, const rjson::value& homologous_titer, double fold, const rjson::value& within_4fold, const rjson::value& outside_4fold, bool verbose);

}; // class ModSerumCoverage

// ----------------------------------------------------------------------

class ModSerumCoverageCircle : public ModSerumHomologous
{
 public:
    using ModSerumHomologous::ModSerumHomologous;

    void apply(ChartDraw& aChartDraw, const rjson::value& aModData) override;

}; // class ModSerumCoverage

// ----------------------------------------------------------------------

class ModSerumLine : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& aModData) override;

}; // class ModSerumCoverage

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
