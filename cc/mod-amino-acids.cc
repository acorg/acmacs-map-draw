#include "acmacs-base/enumerate.hh"
#include "seqdb/seqdb.hh"
#include "acmacs-map-draw/mod-amino-acids.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

void ModAminoAcids::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = args().get_or_default("report", false);
    try {
        if (auto [pos_present, pos] = args().get_array_if("pos"); pos_present) {
            aa_pos(aChartDraw, pos, verbose);
        }
        else if (auto [groups_present, groups] = args().get_array_if("groups"); groups_present) {
            for (const auto& group: groups)
                aa_group(aChartDraw, group, verbose);
        }
        else {
            std::cerr << "No pos no groups" << '\n';
            throw std::bad_variant_access{};
        }
    }
    catch (std::bad_variant_access&) {
        throw unrecognized_mod{"expected either \"pos\":[] or \"groups\":[] mod: " + args().to_json() };
    }

} // ModAminoAcids::apply

// ----------------------------------------------------------------------

void ModAminoAcids::aa_pos(ChartDraw& aChartDraw, const rjson::array& aPos, bool aVerbose)
{
    const auto& seqdb = seqdb::get(do_report_time(aVerbose));
    const auto aa_indices = seqdb.aa_at_positions_for_antigens(*aChartDraw.chart().antigens(), {std::begin(aPos), std::end(aPos)}, aVerbose);
      // aa_indices is std::map<std::string, std::vector<size_t>>
    std::vector<std::string> aa_sorted(aa_indices.size()); // most frequent aa first
    std::transform(std::begin(aa_indices), std::end(aa_indices), std::begin(aa_sorted), [](const auto& entry) -> std::string { return entry.first; });
    std::sort(std::begin(aa_sorted), std::end(aa_sorted), [&aa_indices](const auto& n1, const auto& n2) -> bool { return aa_indices.find(n1)->second.size() > aa_indices.find(n2)->second.size(); });
    for (auto [index, aa]: acmacs::enumerate(aa_sorted)) {
        const auto& indices_for_aa = aa_indices.find(aa)->second;
        auto styl = style();
        styl.fill = fill_color(index, aa);
        aChartDraw.modify(indices_for_aa, styl, drawing_order());
        try { add_legend(aChartDraw, indices_for_aa, styl, aa, args()["legend"]); } catch (rjson::field_not_found&) {}
        if (aVerbose)
            std::cerr << "INFO: amino-acids at " << aPos << ": " << aa << ' ' << indices_for_aa.size() << '\n';
    }

    if (auto centroid = args().get_or_default("centroid", false); centroid) {
        auto layout = aChartDraw.projection().layout();
    }

} // ModAminoAcids::aa_pos

// ----------------------------------------------------------------------

void ModAminoAcids::aa_group(ChartDraw& aChartDraw, const rjson::object& aGroup, bool aVerbose)
{
    const rjson::array& pos_aa = aGroup["pos_aa"];
    std::vector<size_t> positions(pos_aa.size());
    std::transform(std::begin(pos_aa), std::end(pos_aa), std::begin(positions), [](const auto& src) { return std::stoul(src.str()); });
    std::string target_aas(pos_aa.size(), ' ');
    std::transform(std::begin(pos_aa), std::end(pos_aa), std::begin(target_aas), [](const auto& src) { return static_cast<std::string_view>(src).back(); });
    const auto& seqdb = seqdb::get(do_report_time(aVerbose));
    const auto aa_indices = seqdb.aa_at_positions_for_antigens(*aChartDraw.chart().antigens(), positions, aVerbose);
    if (const auto aap = aa_indices.find(target_aas); aap != aa_indices.end()) {
        auto styl = style();
        styl = point_style_from_json(aGroup);
        aChartDraw.modify(aap->second, styl, drawing_order());
        try { add_legend(aChartDraw, aap->second, styl, string::join(" ", std::begin(pos_aa), std::end(pos_aa)), args()["legend"]); } catch (rjson::field_not_found&) {}
        if (aVerbose)
            std::cerr << "INFO: amino-acids group " << pos_aa << ": " << ' ' << aap->second.size() << '\n';
    }
    else {
        std::cerr << "WARNING: no \"" << target_aas << "\" in " << aa_indices << '\n';
    }

} // ModAminoAcids::aa_group

// ----------------------------------------------------------------------

Color ModAminoAcids::fill_color(size_t aIndex, std::string aAA)
{
    if (aAA == "X") {
        ++mIndexDiff;
        return Color(args().get_or_default("X_color", "grey25"));
    }
    if (mColors.empty()) {
        if (auto [colors_present, colors] = args().get_array_if("colors"); colors_present) {
            mColors.resize(colors.size());
            std::transform(std::begin(colors), std::end(colors), mColors.begin(), [](const auto& src) -> std::string_view { return src; });
        }
        else {
            const auto ct = args().get_or_default("color_set", "ana");
            mColors = Color::distinct(ct == "google" ? Color::distinct_t::GoogleMaps : Color::distinct_t::Ana);
        }
    }
    const auto index = aIndex - mIndexDiff;
    if (index < mColors.size())
        return mColors[index];
    else
        throw unrecognized_mod{"too few distinct colors in mod: " + args().to_json()};

} // ModAminoAcids::fill_color

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
