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

    virtual acmacs::chart::Indexes select(const acmacs::chart::Chart& aChart, const acmacs::chart::Chart* aPreviousChart, const rjson::value& aSelector);
    inline acmacs::chart::Indexes select(const acmacs::chart::Chart& aChart, const rjson::value& aSelector) { return select(aChart, nullptr, aSelector); }
    virtual acmacs::chart::Indexes command(const acmacs::chart::Chart& aChart, const acmacs::chart::Chart* aPreviousChart, const rjson::object& aSelector) = 0;
    inline acmacs::chart::Indexes command(const acmacs::chart::Chart& aChart, const rjson::object& aSelector) { return select(aChart, nullptr, aSelector); }
    virtual void filter_name(const acmacs::chart::Chart& aChart, acmacs::chart::Indexes& indices, std::string aName) = 0;
    virtual void filter_full_name(const acmacs::chart::Chart& aChart, acmacs::chart::Indexes& indices, std::string aFullName) = 0;
    virtual void filter_rectangle(const acmacs::chart::Chart& aChart, acmacs::chart::Indexes& indices, const acmacs::chart::Projection& aProjection, const acmacs::Rectangle& aRectangle) = 0;
    virtual void filter_circle(const acmacs::chart::Chart& aChart, acmacs::chart::Indexes& indices, const acmacs::chart::Projection& aProjection, const acmacs::Circle& aCircle) = 0;

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

    acmacs::chart::Indexes command(const acmacs::chart::Chart& aChart, const acmacs::chart::Chart* aPreviousChart, const rjson::object& aSelector) override;
    void filter_sequenced(const acmacs::chart::Chart& aChart, acmacs::chart::Indexes& indices);
    void filter_not_sequenced(const acmacs::chart::Chart& aChart, acmacs::chart::Indexes& indices);
    std::map<std::string, size_t> clades(const acmacs::chart::Chart& aChart);
    void filter_clade(const acmacs::chart::Chart& aChart, acmacs::chart::Indexes& indices, std::string aClade);
    inline void filter_name(const acmacs::chart::Chart& aChart, acmacs::chart::Indexes& indices, std::string aName) override { filter_name_in(aChart.antigens(), indices, aName); }
    inline void filter_full_name(const acmacs::chart::Chart& aChart, acmacs::chart::Indexes& indices, std::string aFullName) override { filter_full_name_in(aChart.antigens(), indices, aFullName); }
    void filter_vaccine(const acmacs::chart::Chart& aChart, acmacs::chart::Indexes& indices, const VaccineMatchData& aMatchData);
    inline void filter_rectangle(const acmacs::chart::Chart& /*aChart*/, acmacs::chart::Indexes& indices, const acmacs::chart::Projection& aProjection, const acmacs::Rectangle& aRectangle) override { filter_rectangle_in(indices, 0, *aProjection.layout(), aProjection.transformation(), aRectangle); }
    virtual void filter_circle(const acmacs::chart::Chart& /*aChart*/, acmacs::chart::Indexes& indices, const acmacs::chart::Projection& aProjection, const acmacs::Circle& aCircle) override { filter_circle_in(indices, 0, *aProjection.layout(), aProjection.transformation(), aCircle); }

 private:
    const std::vector<seqdb::SeqdbEntrySeq>& seqdb_entries(const acmacs::chart::Chart& aChart);

};  // class SelectAntigens

// ----------------------------------------------------------------------

class SelectSera : public SelectAntigensSera
{
 public:
    using SelectAntigensSera::SelectAntigensSera;

    acmacs::chart::Indexes command(const acmacs::chart::Chart& aChart, const acmacs::chart::Chart* aPreviousChart, const rjson::object& aSelector) override;
    inline void filter_name(const acmacs::chart::Chart& aChart, acmacs::chart::Indexes& indices, std::string aName) override { filter_name_in(aChart.sera(), indices, aName); }
    inline void filter_full_name(const acmacs::chart::Chart& aChart, acmacs::chart::Indexes& indices, std::string aFullName) override { filter_full_name_in(aChart.antigens(), indices, aFullName); }
    inline void filter_rectangle(const acmacs::chart::Chart& aChart, acmacs::chart::Indexes& indices, const acmacs::chart::Projection& aProjection, const acmacs::Rectangle& aRectangle) override { filter_rectangle_in(indices, aChart.number_of_antigens(), *aProjection.layout(), aProjection.transformation(), aRectangle); }
    virtual void filter_circle(const acmacs::chart::Chart& aChart, acmacs::chart::Indexes& indices, const acmacs::chart::Projection& aProjection, const acmacs::Circle& aCircle) override { filter_circle_in(indices, aChart.number_of_antigens(), *aProjection.layout(), aProjection.transformation(), aCircle); }

};  // class SelectSera

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
