#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "acmacs-base/rjson.hh"
#include "acmacs-base/timeit.hh"
#include "acmacs-draw/size.hh"
#include "seqdb/seqdb.hh"

// ----------------------------------------------------------------------

class Chart;
class LocDb;
namespace hidb { class HiDb; }
class VaccineMatchData;

// ----------------------------------------------------------------------

class SelectAntigensSera
{
 public:
    inline SelectAntigensSera(bool aVerbose = false) : mVerbose{aVerbose}, mReportTime{aVerbose ? report_time::Yes : report_time::No} {}
    virtual ~SelectAntigensSera();

    virtual std::vector<size_t> select(const Chart& aChart, const rjson::value& aSelector);
    virtual std::vector<size_t> command(const Chart& aChart, const rjson::object& aSelector) = 0;
    virtual void filter_name(const Chart& aChart, std::vector<size_t>& indices, std::string aName) = 0;
    virtual void filter_full_name(const Chart& aChart, std::vector<size_t>& indices, std::string aFullName) = 0;
    virtual void filter_rectangle(const Chart& aChart, std::vector<size_t>& indices, const ProjectionBase& aProjection, const Rectangle& aRectangle) = 0;
    virtual void filter_circle(const Chart& aChart, std::vector<size_t>& indices, const ProjectionBase& aProjection, const Circle& aCircle) = 0;

 protected:
    template <typename AgSr> inline void filter_name_in(const AgSr& aAgSr, std::vector<size_t>& indices, std::string aName)
        {
              // Timeit ti("filter_name_in " + aName + ": ", std::cerr, mVerbose ? report_time::Yes : report_time::No);
            std::vector<size_t> result(indices.size());
            const auto by_name = aAgSr.find_by_name(aName);
            const auto end = std::set_intersection(indices.begin(), indices.end(), by_name.begin(), by_name.end(), result.begin());
            indices.erase(std::copy(result.begin(), end, indices.begin()), indices.end());
        }

    template <typename AgSr> inline void filter_full_name_in(const AgSr& aAgSr, std::vector<size_t>& indices, std::string aFullName)
        {
            indices.erase(std::remove_if(indices.begin(), indices.end(), [&](auto index) { return aAgSr[index].full_name() != aFullName; }), indices.end());
        }

    inline void filter_rectangle_in(std::vector<size_t>& indices, size_t aIndexBase, const LayoutBase& aLayout, const Transformation& aTransformation, const Rectangle& aRectangle)
        {
            const auto rect_transformed = aRectangle.transform(aTransformation.inverse());
            auto not_in_rectangle = [&](auto index) -> bool { const auto& p = aLayout[index + aIndexBase]; return p.size() == 2 ? !rect_transformed.within(p[0], p[1]) : true; };
            indices.erase(std::remove_if(indices.begin(), indices.end(), not_in_rectangle), indices.end());
        }

    inline void filter_circle_in(std::vector<size_t>& indices, size_t aIndexBase, const LayoutBase& aLayout, const Transformation& aTransformation, const Circle& aCircle)
        {
            const auto circle_transformed = aCircle.transform(aTransformation.inverse());
            auto not_in_circle = [&](auto index) -> bool { const auto& p = aLayout[index + aIndexBase]; return p.size() == 2 ? !circle_transformed.within(p[0], p[1]) : true; };
            indices.erase(std::remove_if(indices.begin(), indices.end(), not_in_circle), indices.end());
        }

    inline bool verbose() const { return mVerbose; }
    inline auto timer() { return mReportTime; }

 private:
    bool mVerbose;
    report_time mReportTime;

}; // class SelectAntigensSera

// ----------------------------------------------------------------------

class SelectAntigens : public SelectAntigensSera
{
 public:
    using SelectAntigensSera::SelectAntigensSera;

    std::vector<size_t> command(const Chart& aChart, const rjson::object& aSelector) override;
    void filter_sequenced(const Chart& aChart, std::vector<size_t>& indices);
    void filter_not_sequenced(const Chart& aChart, std::vector<size_t>& indices);
    std::map<std::string, size_t> clades(const Chart& aChart);
    void filter_clade(const Chart& aChart, std::vector<size_t>& indices, std::string aClade);
    inline void filter_name(const Chart& aChart, std::vector<size_t>& indices, std::string aName) override { filter_name_in(aChart.antigens(), indices, aName); }
    inline void filter_full_name(const Chart& aChart, std::vector<size_t>& indices, std::string aFullName) override { filter_full_name_in(aChart.antigens(), indices, aFullName); }
    void filter_vaccine(const Chart& aChart, std::vector<size_t>& indices, const VaccineMatchData& aMatchData);
    inline void filter_rectangle(const Chart& /*aChart*/, std::vector<size_t>& indices, const ProjectionBase& aProjection, const Rectangle& aRectangle) override { filter_rectangle_in(indices, 0, aProjection.layout(), aProjection.transformation(), aRectangle); }
    virtual void filter_circle(const Chart& /*aChart*/, std::vector<size_t>& indices, const ProjectionBase& aProjection, const Circle& aCircle) override { filter_circle_in(indices, 0, aProjection.layout(), aProjection.transformation(), aCircle); }

 private:
    std::unique_ptr<std::vector<seqdb::SeqdbEntrySeq>> mSeqdbEntries;
    const Chart* mChartForSeqdbEntries = nullptr;

    const std::vector<seqdb::SeqdbEntrySeq>& seqdb_entries(const Chart& aChart);

};  // class SelectAntigens

// ----------------------------------------------------------------------

class SelectSera : public SelectAntigensSera
{
 public:
    using SelectAntigensSera::SelectAntigensSera;

    std::vector<size_t> command(const Chart& aChart, const rjson::object& aSelector) override;
    inline void filter_name(const Chart& aChart, std::vector<size_t>& indices, std::string aName) override { filter_name_in(aChart.sera(), indices, aName); }
    inline void filter_full_name(const Chart& aChart, std::vector<size_t>& indices, std::string aFullName) override { filter_full_name_in(aChart.antigens(), indices, aFullName); }
    inline void filter_rectangle(const Chart& aChart, std::vector<size_t>& indices, const ProjectionBase& aProjection, const Rectangle& aRectangle) override { filter_rectangle_in(indices, aChart.number_of_antigens(), aProjection.layout(), aProjection.transformation(), aRectangle); }
    virtual void filter_circle(const Chart& aChart, std::vector<size_t>& indices, const ProjectionBase& aProjection, const Circle& aCircle) override { filter_circle_in(indices, aChart.number_of_antigens(), aProjection.layout(), aProjection.transformation(), aCircle); }

};  // class SelectSera

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
