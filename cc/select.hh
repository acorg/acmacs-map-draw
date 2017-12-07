#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "acmacs-base/rjson.hh"
#include "acmacs-base/timeit.hh"
#include "acmacs-base/size.hh"
#include "acmacs-chart-2/chart.hh"
#include "seqdb/seqdb.hh"
#include "acmacs-map-draw/chart-select-interface.hh"

// ----------------------------------------------------------------------

class LocDb;
namespace hidb { class HiDb; }
class VaccineMatchData;

// ----------------------------------------------------------------------

class SelectAntigensSera
{
 public:
    inline SelectAntigensSera(bool aVerbose = false, size_t aReportNamesThreshold = 10)
        : mVerbose{aVerbose}, mReportNamesThreshold{aReportNamesThreshold}, mReportTime{aVerbose ? report_time::Yes : report_time::No} {}
    virtual ~SelectAntigensSera();

    virtual acmacs::chart::Indexes select(const ChartSelectInterface& aChartSelectInterface, const rjson::value& aSelector);
    virtual acmacs::chart::Indexes command(const ChartSelectInterface& aChartSelectInterface, const rjson::object& aSelector) = 0;
    virtual void filter_name(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indices, std::string aName) = 0;
    virtual void filter_full_name(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indices, std::string aFullName) = 0;
    virtual void filter_rectangle(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indices, const acmacs::Rectangle& aRectangle) = 0;
    virtual void filter_circle(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indices, const acmacs::Circle& aCircle) = 0;

 protected:
    template <typename AgSr> inline void filter_name_in(AgSr aAgSr, acmacs::chart::Indexes& indices, std::string aName)
        {
              // Timeit ti("filter_name_in " + aName + ": ", mVerbose ? report_time::Yes : report_time::No);
            acmacs::chart::Indexes result(indices.size());
            const auto by_name = aAgSr->find_by_name(aName);
            const auto end = std::set_intersection(indices.begin(), indices.end(), by_name.begin(), by_name.end(), result.begin());
            indices.erase(std::copy(result.begin(), end, indices.begin()), indices.end());
        }

    template <typename AgSr> inline void filter_full_name_in(AgSr aAgSr, acmacs::chart::Indexes& indices, std::string aFullName)
        {
            indices.erase(std::remove_if(indices.begin(), indices.end(), [&](auto index) { return (*aAgSr)[index]->full_name() != aFullName; }), indices.end());
        }

    inline void filter_rectangle_in(acmacs::chart::Indexes& indices, size_t aIndexBase, const acmacs::chart::Layout& aLayout, const acmacs::Transformation& aTransformation, const acmacs::Rectangle& aRectangle)
        {
            const auto rect_transformed = aRectangle.transform(aTransformation.inverse());
            auto not_in_rectangle = [&](auto index) -> bool { const auto& p = aLayout[index + aIndexBase]; return p.size() == 2 ? !rect_transformed.within(p[0], p[1]) : true; };
            indices.erase(std::remove_if(indices.begin(), indices.end(), not_in_rectangle), indices.end());
        }

    inline void filter_circle_in(acmacs::chart::Indexes& indices, size_t aIndexBase, const acmacs::chart::Layout& aLayout, const acmacs::Transformation& aTransformation, const acmacs::Circle& aCircle)
        {
            const auto circle_transformed = aCircle.transform(aTransformation.inverse());
            auto not_in_circle = [&](auto index) -> bool { const auto& p = aLayout[index + aIndexBase]; return p.size() == 2 ? !circle_transformed.within(p[0], p[1]) : true; };
            indices.erase(std::remove_if(indices.begin(), indices.end(), not_in_circle), indices.end());
        }

    inline bool verbose() const { return mVerbose; }
    inline size_t report_names_threshold() const { return mReportNamesThreshold; }
    inline auto timer() { return mReportTime; }

 private:
    bool mVerbose;
    size_t mReportNamesThreshold;
    report_time mReportTime;

}; // class SelectAntigensSera

// ----------------------------------------------------------------------

class SelectAntigens : public SelectAntigensSera
{
 public:
    using SelectAntigensSera::SelectAntigensSera;

    acmacs::chart::Indexes command(const ChartSelectInterface& aChartSelectInterface, const rjson::object& aSelector) override;
    void filter_sequenced(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indices);
    void filter_not_sequenced(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indices);
    std::map<std::string, size_t> clades(const ChartSelectInterface& aChartSelectInterface);
    void filter_clade(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indices, std::string aClade);
    void filter_outlier(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indices, double aUnits);
    inline void filter_name(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indices, std::string aName) override { filter_name_in(aChartSelectInterface.chart().antigens(), indices, aName); }
    inline void filter_full_name(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indices, std::string aFullName) override { filter_full_name_in(aChartSelectInterface.chart().antigens(), indices, aFullName); }
    void filter_vaccine(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indices, const VaccineMatchData& aMatchData);
    inline void filter_rectangle(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indices, const acmacs::Rectangle& aRectangle) override { filter_rectangle_in(indices, 0, *aChartSelectInterface.layout(), aChartSelectInterface.transformation(), aRectangle); }
    virtual void filter_circle(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indices, const acmacs::Circle& aCircle) override { filter_circle_in(indices, 0, *aChartSelectInterface.layout(), aChartSelectInterface.transformation(), aCircle); }

 private:
    const std::vector<seqdb::SeqdbEntrySeq>& seqdb_entries(const ChartSelectInterface& aChartSelectInterface);

};  // class SelectAntigens

// ----------------------------------------------------------------------

class SelectSera : public SelectAntigensSera
{
 public:
    using SelectAntigensSera::SelectAntigensSera;

    acmacs::chart::Indexes command(const ChartSelectInterface& aChartSelectInterface, const rjson::object& aSelector) override;
    inline void filter_name(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indices, std::string aName) override { filter_name_in(aChartSelectInterface.chart().sera(), indices, aName); }
    inline void filter_full_name(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indices, std::string aFullName) override { filter_full_name_in(aChartSelectInterface.chart().antigens(), indices, aFullName); }
    inline void filter_rectangle(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indices, const acmacs::Rectangle& aRectangle) override { filter_rectangle_in(indices, aChartSelectInterface.chart().number_of_antigens(), *aChartSelectInterface.layout(), aChartSelectInterface.transformation(), aRectangle); }
    virtual void filter_circle(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indices, const acmacs::Circle& aCircle) override { filter_circle_in(indices, aChartSelectInterface.chart().number_of_antigens(), *aChartSelectInterface.layout(), aChartSelectInterface.transformation(), aCircle); }

};  // class SelectSera

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
