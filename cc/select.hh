#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "acmacs-base/rjson-forward.hh"
#include "acmacs-base/timeit.hh"
#include "acmacs-base/size.hh"
#include "acmacs-base/debug.hh"
#include "acmacs-chart-2/chart.hh"
#include "seqdb-3/aa-at-pos.hh"
#include "acmacs-map-draw/chart-select-interface.hh"

// ----------------------------------------------------------------------

// class LocDb;
class VaccineMatchData;

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
    template <typename AgSr> void filter_name_in(AgSr aAgSr, acmacs::chart::Indexes& indexes, std::string_view aName)
    {
        // Timeit ti("filter_name_in " + aName + ": ", do_report_time(mVerbose));
        acmacs::chart::Indexes result(indexes->size());
        const auto by_name = aAgSr->find_by_name(aName);
        // std::cerr << "DEBUG: SelectAntigensSera::filter_name_in \"" << aName << "\": " << by_name << '\n';
        const auto end = std::set_intersection(indexes.begin(), indexes.end(), by_name.begin(), by_name.end(), result.begin());
        indexes.get().erase(std::copy(result.begin(), end, indexes.begin()), indexes.end());
    }

    template <typename AgSr> void filter_full_name_in(AgSr aAgSr, acmacs::chart::Indexes& indexes, std::string_view aFullName)
    {
        indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), [&](auto index) { return (*aAgSr)[index]->full_name() != aFullName; }), indexes.end());
    }

    template <typename AgSr> void filter_location_in(AgSr aAgSr, acmacs::chart::Indexes& indexes, std::string_view aLocation)
    {
        indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), [&](auto index) { return (*aAgSr)[index]->location() != aLocation; }), indexes.end());
    }

    template <typename AgSr> void filter_out_distinct_in(AgSr aAgSr, acmacs::chart::Indexes& indexes)
    {
        indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), [&](auto index) { return (*aAgSr)[index]->annotations().distinct(); }), indexes.end());
    }

    void filter_rectangle_in(acmacs::chart::Indexes& indexes, size_t aIndexBase, const acmacs::Layout& aLayout, const acmacs::Rectangle& aRectangle, Rotation rotation);

    void filter_circle_in(acmacs::chart::Indexes& indexes, size_t aIndexBase, const acmacs::Layout& aLayout, const acmacs::Circle& aCircle)
    {
        const auto not_in_circle = [&](auto index) -> bool {
            const auto& p = aLayout[index + aIndexBase];
            return p.number_of_dimensions() == acmacs::number_of_dimensions_t{2} ? !aCircle.within(p) : true;
        };
        indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), not_in_circle), indexes.end());
    }

    void filter_outline_in(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, size_t aIndexBase, Color outline)
    {
        const auto& all_styles = aChartSelectInterface.plot_spec().all_styles();
        const auto other_outline = [&all_styles,outline,aIndexBase](auto index) -> bool {
            return all_styles[index + aIndexBase].outline != outline;
        };
        indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), other_outline), indexes.end());
    }

    void filter_outline_width_in(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, size_t aIndexBase, Pixels outline_width)
    {
        const auto& all_styles = aChartSelectInterface.plot_spec().all_styles();
        const auto other_outline_width = [&all_styles,outline_width,aIndexBase](auto index) -> bool {
            return all_styles[index + aIndexBase].outline_width != outline_width;
        };
        indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), other_outline_width), indexes.end());
    }

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

    acmacs::chart::Indexes command(const ChartSelectInterface& aChartSelectInterface, const rjson::value& aSelector) override;
    void filter_sequenced(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes);
    void filter_not_sequenced(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes);
    std::map<std::string_view, size_t> clades(const ChartSelectInterface& aChartSelectInterface);
    void filter_clade(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aClade);
    void filter_amino_acid_at_pos(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, char amino_acid, acmacs::seqdb::pos1_t pos1, bool equal);
    void filter_amino_acid_at_pos(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::seqdb::amino_acid_at_pos1_eq_list_t& pos1_aa);
    void filter_outlier(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, double aUnits);
    void filter_name(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aName) override
    {
        filter_name_in(aChartSelectInterface.chart().antigens(), indexes, aName);
    }
    void filter_full_name(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aFullName) override
    {
        filter_full_name_in(aChartSelectInterface.chart().antigens(), indexes, aFullName);
    }
    void filter_location(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aLocation) override
    {
        filter_location_in(aChartSelectInterface.chart().antigens(), indexes, aLocation);
    }
    void filter_vaccine(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const VaccineMatchData& aMatchData);
    void filter_rectangle(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::Rectangle& aRectangle, transformed_t transformed,
                          Rotation rotation) override
    {
        filter_rectangle_in(indexes, 0, transformed == transformed_t::yes ? *aChartSelectInterface.transformed_layout() : *aChartSelectInterface.layout(), aRectangle, rotation);
    }
    void filter_circle(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::Circle& aCircle) override
    {
        filter_circle_in(indexes, 0, *aChartSelectInterface.layout(), aCircle);
    }
    void filter_relative_to_serum_line(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, double distance_min, double distance_max, int direction);
    void filter_table(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aTable) override;
    void filter_layer(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, int aLayer) override;
    void filter_out_distinct(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes) override { filter_out_distinct_in(aChartSelectInterface.chart().antigens(), indexes); }
    void filter_titrated_against(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& antigen_indexes, const acmacs::chart::Indexes& serum_indexes);

    void filter_outline(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, Color outline) override { filter_outline_in(aChartSelectInterface, indexes, 0, outline); }
    void filter_outline_width(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, Pixels outline_width) override
    {
        filter_outline_width_in(aChartSelectInterface, indexes, 0, outline_width);
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
        filter_name_in(aChartSelectInterface.chart().sera(), indexes, aName);
    }
    void filter_full_name(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aFullName) override
    {
        filter_full_name_in(aChartSelectInterface.chart().sera(), indexes, aFullName);
    }
    void filter_location(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aLocation) override
    {
        filter_location_in(aChartSelectInterface.chart().sera(), indexes, aLocation);
    }
    void filter_rectangle(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::Rectangle& aRectangle, transformed_t transformed,
                          Rotation rotation) override
    {
        filter_rectangle_in(indexes, aChartSelectInterface.chart().number_of_antigens(),
                            transformed == transformed_t::yes ? *aChartSelectInterface.transformed_layout() : *aChartSelectInterface.layout(), aRectangle, rotation);
    }
    void filter_circle(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::Circle& aCircle) override
    {
        filter_circle_in(indexes, aChartSelectInterface.chart().number_of_antigens(), *aChartSelectInterface.layout(), aCircle);
    }
    void filter_table(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aTable) override;
    void filter_layer(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, int aLayer) override;
    void filter_clade(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aClade);
    void filter_amino_acid_at_pos(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::seqdb::amino_acid_at_pos1_eq_list_t& pos1_aa);
    void filter_out_distinct(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes) override { filter_out_distinct_in(aChartSelectInterface.chart().sera(), indexes); }
    void filter_titrated_against(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& serum_indexes, const acmacs::chart::Indexes& antigen_indexes);
    void filter_date_range(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view from, std::string_view to);

    void filter_outline(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, Color outline) override
    {
        filter_outline_in(aChartSelectInterface, indexes, aChartSelectInterface.chart().number_of_antigens(), outline);
    }
    void filter_outline_width(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, Pixels outline_width) override
    {
        filter_outline_width_in(aChartSelectInterface, indexes, aChartSelectInterface.chart().number_of_antigens(), outline_width);
    }

}; // class SelectSera

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
