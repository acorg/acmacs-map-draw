#pragma once

#include "acmacs-virus/type-subtype.hh"
#include "acmacs-chart-2/chart.hh"
#include "hidb-5/hidb.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

class VaccineMatchData;         // vaccines.hh

// ----------------------------------------------------------------------

namespace acmacs::map_draw::select::filter
{
    template <typename AgSr> void name_in(const AgSr& aAgSr, acmacs::chart::Indexes& indexes, std::string_view aName);
    extern template void name_in(const acmacs::chart::Antigens& aAgSr, acmacs::chart::Indexes& indexes, std::string_view aName);
    extern template void name_in(const acmacs::chart::Sera& aAgSr, acmacs::chart::Indexes& indexes, std::string_view aName);

    template <typename AgSr> void full_name_in(const AgSr& aAgSr, acmacs::chart::Indexes& indexes, std::string_view aFullName)
    {
        indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), [&](auto index) { return aAgSr[index]->full_name() != aFullName; }), indexes.end());
    }

    template <typename AgSr> void location_in(const AgSr& aAgSr, acmacs::chart::Indexes& indexes, std::string_view aLocation)
    {
        indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), [&](auto index) { return aAgSr[index]->location() != aLocation; }), indexes.end());
    }

    template <typename AgSr> void out_distinct_in(const AgSr& aAgSr, acmacs::chart::Indexes& indexes)
    {
        indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), [&](auto index) { return aAgSr[index]->annotations().distinct(); }), indexes.end());
    }

    inline void circle_in(acmacs::chart::Indexes& indexes, size_t aIndexBase, const acmacs::Layout& aLayout, const acmacs::Circle& aCircle)
    {
        const auto not_in_circle = [&](auto index) -> bool {
            const auto& p = aLayout[index + aIndexBase];
            return p.number_of_dimensions() == acmacs::number_of_dimensions_t{2} ? !aCircle.within(p) : true;
        };
        indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), not_in_circle), indexes.end());
    }

    inline void fill_in(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, size_t aIndexBase, Color fill)
    {
        const auto& all_styles = aChartSelectInterface.plot_spec().all_styles();
        const auto other_fill = [&all_styles, fill, aIndexBase](auto index) -> bool { return all_styles[index + aIndexBase].fill() != fill; };
        indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), other_fill), indexes.end());
    }

    inline void outline_in(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, size_t aIndexBase, Color outline)
    {
        const auto& all_styles = aChartSelectInterface.plot_spec().all_styles();
        const auto other_outline = [&all_styles, outline, aIndexBase](auto index) -> bool { return all_styles[index + aIndexBase].outline() != outline; };
        indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), other_outline), indexes.end());
    }

    inline void outline_width_in(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, size_t aIndexBase, Pixels outline_width)
    {
        const auto& all_styles = aChartSelectInterface.plot_spec().all_styles();
        const auto other_outline_width = [&all_styles, outline_width, aIndexBase](auto index) -> bool { return all_styles[index + aIndexBase].outline_width() != outline_width; };
        indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), other_outline_width), indexes.end());
    }

    inline void shown_in(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, size_t aIndexBase = 0)
    {
        const auto& all_styles = aChartSelectInterface.plot_spec().all_styles();
        const auto not_shown = [&all_styles, aIndexBase](auto index) -> bool { return !all_styles[index + aIndexBase].shown(); };
        indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), not_shown), indexes.end());
    }

    inline void not_shown_in(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, size_t aIndexBase = 0)
    {
        const auto& all_styles = aChartSelectInterface.plot_spec().all_styles();
        const auto shown = [&all_styles, aIndexBase](auto index) -> bool { return all_styles[index + aIndexBase].shown(); };
        indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), shown), indexes.end());
    }

    inline void with_label_in(const ChartDraw& chart_draw, acmacs::chart::Indexes& indexes, size_t aIndexBase, bool with_label)
    {
        const auto& labels = chart_draw.labels();
        const auto is_remove = [&labels, with_label, aIndexBase](auto index) -> bool { return labels.exists(index + aIndexBase) != with_label; };
        indexes.get().erase(std::remove_if(indexes.begin(), indexes.end(), is_remove), indexes.end());
    }

    void rectangle_in(acmacs::chart::Indexes& indexes, size_t aIndexBase, const acmacs::Layout& aLayout, const acmacs::Rectangle& aRectangle, Rotation rotation);

    void sequenced(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes);
    void not_sequenced(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes);
    void clade(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, std::string_view aClade);
    void amino_acid_at_pos(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, char amino_acid, acmacs::seqdb::pos1_t pos1, bool equal);
    void amino_acid_at_pos(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::seqdb::amino_acid_at_pos1_eq_list_t& pos1_aa);
    void amino_acid_at_pos(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::seqdb::amino_acid_at_pos1_eq_t& pos1_aa);
    void outlier(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, double aUnits);

    void vaccine(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, const acmacs::virus::type_subtype_t& virus_type, const VaccineMatchData& aMatchData);

    template <typename AgSr> void table_ag_sr(const AgSr& aAgSr, acmacs::chart::Indexes& indexes, std::string_view aTable, std::shared_ptr<hidb::Tables> aHidbTables);
    extern template void table_ag_sr(const std::vector<std::shared_ptr<hidb::Antigen>>& aAgSr, acmacs::chart::Indexes& indexes, std::string_view aTable, std::shared_ptr<hidb::Tables> aHidbTables);
    extern template void table_ag_sr(const std::vector<std::shared_ptr<hidb::Serum>>& aAgSr, acmacs::chart::Indexes& indexes, std::string_view aTable, std::shared_ptr<hidb::Tables> aHidbTables);

    enum ag_sr_ { antigens, sera };

    void layer(const acmacs::chart::Chart& chart, acmacs::chart::Indexes& indexes, int aLayer, ag_sr_ ag_sr);

    void relative_to_serum_line(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& indexes, double distance_min, double distance_max, int direction);
    void antigens_titrated_against(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& antigen_indexes, const acmacs::chart::Indexes& serum_indexes);
    void antigens_not_titrated_against(const ChartSelectInterface& aChartSelectInterface, acmacs::chart::Indexes& antigen_indexes, const acmacs::chart::Indexes& serum_indexes);
    void sera_titrated_against(const ChartSelectInterface& aChartSelectInterface, const acmacs::chart::Indexes& antigen_indexes, acmacs::chart::Indexes& serum_indexes);
    void sera_not_titrated_against(const ChartSelectInterface& aChartSelectInterface, const acmacs::chart::Indexes& antigen_indexes, acmacs::chart::Indexes& serum_indexes);

    void serum_id_in(const acmacs::chart::Sera& sera, acmacs::chart::Indexes& indexes, std::string_view serum_id);

    template <typename AgSrPerIndex> void most_used(const AgSrPerIndex& ag_sr_per_index, acmacs::chart::Indexes& indexes, size_t number_of_most_used);
    template <typename AgSrPerIndex> void most_used_for_name(const AgSrPerIndex& ag_sr_per_index, acmacs::chart::Indexes& indexes, size_t number_of_most_used);

    extern template void most_used(const std::vector<std::shared_ptr<hidb::Antigen>>& ag_sr_per_index, acmacs::chart::Indexes& indexes, size_t number_of_most_used);
    extern template void most_used(const std::vector<std::shared_ptr<hidb::Serum>>& ag_sr_per_index, acmacs::chart::Indexes& indexes, size_t number_of_most_used);
    extern template void most_used_for_name(const std::vector<std::shared_ptr<hidb::Antigen>>& ag_sr_per_index, acmacs::chart::Indexes& indexes, size_t number_of_most_used);
    extern template void most_used_for_name(const std::vector<std::shared_ptr<hidb::Serum>>& ag_sr_per_index, acmacs::chart::Indexes& indexes, size_t number_of_most_used);

} // namespace acmacs::map_draw::select::filter

// ----------------------------------------------------------------------

namespace acmacs::map_draw::select
{
    std::map<std::string_view, size_t> clades(const ChartSelectInterface& aChartSelectInterface);

}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
