#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "acmacs-base/rjson-forward.hh"
#include "acmacs-base/timeit.hh"
#include "acmacs-base/size.hh"
#include "acmacs-base/debug.hh"
#include "seqdb-3/aa-at-pos.hh"
#include "acmacs-map-draw/select-filter.hh"

// ----------------------------------------------------------------------

// class LocDb;
class VaccineMatchData;

// ----------------------------------------------------------------------

// ----------------------------------------------------------------------

class SelectAntigensSera
{
  public:
    SelectAntigensSera(acmacs::verbose aVerbose = acmacs::verbose::no, size_t aReportNamesThreshold = 10)
        : mVerbose{aVerbose}, mReportNamesThreshold{aReportNamesThreshold}, mReportTime{report_time::no}
    {
    }
    virtual ~SelectAntigensSera();

    enum class transformed_t { no, yes };

    virtual acmacs::chart::Indexes select(const ChartSelectInterface& aChartSelectInterface, const rjson::value& aSelector);
    virtual acmacs::chart::Indexes command(const ChartSelectInterface& aChartSelectInterface, const rjson::value& aSelector) = 0;
    virtual void filter_name(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aName) = 0;
    virtual void filter_full_name(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aFullName) = 0;
    virtual void filter_location(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aLocation) = 0;
    virtual void filter_rectangle(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::Rectangle& aRectangle, transformed_t transformed,
                                  Rotation rotation) = 0;
    virtual void filter_circle(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::Circle& aCircle) = 0;
    virtual void filter_out_distinct(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes) = 0;

    // aTable is either table date or full table id (e.g. MELB:HI:turkey:20170216)
    virtual void filter_table(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aTable) = 0;

    // layer >= 0 - from beginning, <0 - from end
    virtual void filter_layer(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, int aLayer) = 0;

    virtual void filter_outline(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, Color outline) = 0;
    virtual void filter_outline_width(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, Pixels outline_width) = 0;

  protected:
    bool verbose() const { return mVerbose == acmacs::verbose::yes; }
    size_t report_names_threshold() const { return mReportNamesThreshold; }
    auto timer() { return mReportTime; }

    // const acmacs::seqdb::subset& seqdb_entries(const ChartSelectInterface& aChartSelectInterface);

  private:
    acmacs::verbose mVerbose;
    size_t mReportNamesThreshold;
    report_time mReportTime;

}; // class SelectAntigensSera

// ----------------------------------------------------------------------

class SelectAntigens : public SelectAntigensSera
{
  public:
    using SelectAntigensSera::SelectAntigensSera;

    void filter_sequenced(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes) { acmacs::map_draw::select::filter::sequenced(aChartSelectInterface, indexes); }
    void filter_not_sequenced(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes) { acmacs::map_draw::select::filter::not_sequenced(aChartSelectInterface, indexes); }
    void filter_clade(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aClade) { acmacs::map_draw::select::filter::clade(aChartSelectInterface, indexes, aClade); }
    void filter_amino_acid_at_pos(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, char amino_acid, acmacs::seqdb::pos1_t pos1, bool equal)  { acmacs::map_draw::select::filter::amino_acid_at_pos(aChartSelectInterface, indexes, amino_acid, pos1, equal); }
    void filter_amino_acid_at_pos(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::seqdb::amino_acid_at_pos1_eq_list_t& pos1_aa) { acmacs::map_draw::select::filter::amino_acid_at_pos(aChartSelectInterface, indexes, pos1_aa); }
    void filter_outlier(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, double aUnits) { acmacs::map_draw::select::filter::outlier(aChartSelectInterface, indexes, aUnits); }
    std::map<std::string_view, size_t> clades(const ChartSelectInterface& aChartSelectInterface) { return acmacs::map_draw::select::clades(aChartSelectInterface); }

    acmacs::chart::Indexes command(const ChartSelectInterface& aChartSelectInterface, const rjson::value& aSelector) override;

    void filter_name(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aName) override
    {
        acmacs::map_draw::select::filter::name_in(aChartSelectInterface.chart().antigens(), indexes, aName);
    }
    void filter_full_name(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aFullName) override
    {
        acmacs::map_draw::select::filter::full_name_in(aChartSelectInterface.chart().antigens(), indexes, aFullName);
    }
    void filter_location(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aLocation) override
    {
        acmacs::map_draw::select::filter::location_in(aChartSelectInterface.chart().antigens(), indexes, aLocation);
    }
    void filter_rectangle(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::Rectangle& aRectangle, transformed_t transformed,
                          Rotation rotation) override
    {
        acmacs::map_draw::select::filter::rectangle_in(indexes, 0, transformed == transformed_t::yes ? *aChartSelectInterface.transformed_layout() : *aChartSelectInterface.layout(), aRectangle, rotation);
    }
    void filter_circle(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::Circle& aCircle) override
    {
        acmacs::map_draw::select::filter::circle_in(indexes, 0, *aChartSelectInterface.layout(), aCircle);
    }
    void filter_outline(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, Color outline) override { acmacs::map_draw::select::filter::outline_in(aChartSelectInterface, indexes, 0, outline); }
    void filter_outline_width(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, Pixels outline_width) override
    {
        acmacs::map_draw::select::filter::outline_width_in(aChartSelectInterface, indexes, 0, outline_width);
    }
    void filter_out_distinct(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes) override { acmacs::map_draw::select::filter::out_distinct_in(aChartSelectInterface.chart().antigens(), indexes); }

    void filter_vaccine(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const VaccineMatchData& aMatchData)
    {
        acmacs::map_draw::select::filter::vaccine(aChartSelectInterface, indexes, aChartSelectInterface.chart().info()->virus_type(acmacs::chart::Info::Compute::Yes), aMatchData);
    }
    void filter_layer(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, int aLayer) override;
    void filter_table(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aTable) override;

    void filter_relative_to_serum_line(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, double distance_min, double distance_max, int direction)
    {
        acmacs::map_draw::select::filter::relative_to_serum_line(aChartSelectInterface, indexes, distance_min, distance_max, direction);
    }

    void filter_titrated_against(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& antigen_indexes, const acmacs::chart::Indexes& serum_indexes)
    {
        acmacs::map_draw::select::filter::antigens_titrated_against(aChartSelectInterface, antigen_indexes, serum_indexes);
    }

}; // class SelectAntigens

// ----------------------------------------------------------------------

class SelectSera : public SelectAntigensSera
{
  public:
    using SelectAntigensSera::SelectAntigensSera;

    acmacs::chart::Indexes command(const ChartSelectInterface& aChartSelectInterface, const rjson::value& aSelector) override;

    void filter_name(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aName) override
    {
        acmacs::map_draw::select::filter::name_in(aChartSelectInterface.chart().sera(), indexes, aName);
    }
    void filter_full_name(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aFullName) override
    {
        acmacs::map_draw::select::filter::full_name_in(aChartSelectInterface.chart().sera(), indexes, aFullName);
    }
    void filter_location(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aLocation) override
    {
        acmacs::map_draw::select::filter::location_in(aChartSelectInterface.chart().sera(), indexes, aLocation);
    }
    void filter_rectangle(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::Rectangle& aRectangle, transformed_t transformed,
                          Rotation rotation) override
    {
        acmacs::map_draw::select::filter::rectangle_in(indexes, aChartSelectInterface.chart().number_of_antigens(),
                            transformed == transformed_t::yes ? *aChartSelectInterface.transformed_layout() : *aChartSelectInterface.layout(), aRectangle, rotation);
    }
    void filter_circle(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::Circle& aCircle) override
    {
        acmacs::map_draw::select::filter::circle_in(indexes, aChartSelectInterface.chart().number_of_antigens(), *aChartSelectInterface.layout(), aCircle);
    }
    void filter_outline(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, Color outline) override
    {
        acmacs::map_draw::select::filter::outline_in(aChartSelectInterface, indexes, aChartSelectInterface.chart().number_of_antigens(), outline);
    }
    void filter_outline_width(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, Pixels outline_width) override
    {
        acmacs::map_draw::select::filter::outline_width_in(aChartSelectInterface, indexes, aChartSelectInterface.chart().number_of_antigens(), outline_width);
    }
    void filter_layer(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, int aLayer) override;
    void filter_table(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aTable) override;
    void filter_titrated_against(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& serum_indexes, const acmacs::chart::Indexes& antigen_indexes)
    {
        acmacs::map_draw::select::filter::sera_titrated_against(aChartSelectInterface, antigen_indexes, serum_indexes);
    }


    void filter_clade(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aClade);
    void filter_amino_acid_at_pos(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::seqdb::amino_acid_at_pos1_eq_list_t& pos1_aa);
    void filter_out_distinct(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes) override { acmacs::map_draw::select::filter::out_distinct_in(aChartSelectInterface.chart().sera(), indexes); }
    void filter_date_range(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view from, std::string_view to);

}; // class SelectSera

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
