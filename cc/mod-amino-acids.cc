#include <tuple>

#include "acmacs-base/enumerate.hh"
#include "seqdb/seqdb.hh"
#include "acmacs-map-draw/mod-amino-acids.hh"
#include "acmacs-map-draw/draw.hh"

// ----------------------------------------------------------------------

void ModAminoAcids::apply(ChartDraw& aChartDraw, const rjson::v1::value& /*aModData*/)
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

void ModAminoAcids::aa_pos(ChartDraw& aChartDraw, const rjson::v1::array& aPos, bool aVerbose)
{
    const auto& seqdb = seqdb::get(seqdb::ignore_errors::no, do_report_time(aVerbose));
    const auto aa_indices = seqdb.aa_at_positions_for_antigens(*aChartDraw.chart().antigens(), {std::begin(aPos), std::end(aPos)}, aVerbose ? seqdb::report::yes : seqdb::report::no);
      // aa_indices is std::map<std::string, std::vector<size_t>>
    std::vector<std::string> aa_sorted(aa_indices.size()); // most frequent aa first
    std::transform(std::begin(aa_indices), std::end(aa_indices), std::begin(aa_sorted), [](const auto& entry) -> std::string { return entry.first; });
    std::sort(std::begin(aa_sorted), std::end(aa_sorted), [&aa_indices](const auto& n1, const auto& n2) -> bool { return aa_indices.find(n1)->second.size() > aa_indices.find(n2)->second.size(); });
    std::map<std::string, Color> color_for_aa;
    make_color_for_aa(color_for_aa, aa_sorted);
    for (auto [index, aa]: acmacs::enumerate(aa_sorted)) {
        const auto& indices_for_aa = aa_indices.find(aa)->second;
        auto styl = style();
        if (auto ca = color_for_aa.find(aa); ca == color_for_aa.end())
            throw unrecognized_mod{"cannot find color for AA: " + aa + ", \"colors\" in the settings is not complete?"};
        else
            styl.fill = ca->second;
        aChartDraw.modify(indices_for_aa, styl, drawing_order());
        try { add_legend(aChartDraw, indices_for_aa, styl, aa, args()["legend"]); } catch (rjson::v1::field_not_found&) {}
        if (aVerbose)
            std::cerr << "INFO: amino-acids at " << aPos << ": " << aa << ' ' << indices_for_aa.size() << '\n';
    }

    if (auto make_centroid = args().get_or_default("centroid", false); make_centroid) {
        auto layout = aChartDraw.projection().transformed_layout();
        for (auto [aa, indexes] : aa_indices) {
            if (indexes.size() > 1) {
                  // coordinates, distance to centroid of all points, distance to centroid of 90% closest points
                using element_t = std::tuple<acmacs::Coordinates, double, double>;
                using acmacs::Coordinates;
                std::vector<element_t> points(indexes.size());
                std::transform(indexes.begin(), indexes.end(), points.begin(), [layout](auto index) -> element_t { return {layout->get(index), -1, -1}; });
                Coordinates centroid = std::accumulate(points.begin(), points.end(), Coordinates(layout->number_of_dimensions(), 0.0), [](Coordinates& sum, const auto& point) { for (size_t dim = 0; dim < std::get<0>(point).size(); ++dim) sum[dim] += std::get<0>(point)[dim]; return sum; });
                centroid /= points.size();
                std::for_each(points.begin(), points.end(), [&centroid](auto& point) { std::get<1>(point) = std::get<0>(point).distance(centroid); });
                std::sort(points.begin(), points.end(), [](const auto& p1, const auto& p2) { return std::get<1>(p1) < std::get<1>(p2); });
                const auto radius = std::get<1>(points.back());
                  // std::cerr << "DEBUG: min dist:" << std::get<1>(points.front()) << " max:" << radius << '\n';

                const auto num_points_90 = static_cast<long>(points.size() * 0.9);
                Coordinates centroid_90 = std::accumulate(points.begin(), points.begin() + num_points_90, Coordinates(layout->number_of_dimensions(), 0.0), [](Coordinates& sum, const auto& point) { for (size_t dim = 0; dim < std::get<0>(point).size(); ++dim) sum[dim] += std::get<0>(point)[dim]; return sum; });
                centroid_90 /= num_points_90;
                std::for_each(points.begin(), points.begin() + num_points_90, [&centroid_90](auto& point) { std::get<2>(point) = std::get<0>(point).distance(centroid_90); });
                std::sort(points.begin(), points.begin() + num_points_90, [](const auto& p1, const auto& p2) { return std::get<2>(p1) < std::get<2>(p2); });
                const auto radius_90 = std::get<2>(*(points.begin() + num_points_90 - 1));

                  //aChartDraw.point(centroid, Pixels{10}).color(color_for_aa.find(aa)->second, GREEN);
                aChartDraw.circle(centroid, Scaled{radius * 2}).color(TRANSPARENT, color_for_aa.find(aa)->second);
                aChartDraw.circle(centroid, Scaled{radius_90 * 2}).color(TRANSPARENT, color_for_aa.find(aa)->second);

            }
        }
    }

} // ModAminoAcids::aa_pos

// ----------------------------------------------------------------------

void ModAminoAcids::make_color_for_aa(std::map<std::string, Color>& color_for_aa, const std::vector<std::string>& aa_sorted)
{
    if (auto [colors_present, colors] = args().get_object_if("colors"); colors_present) {
        std::vector<std::string> aa_without_colors;
        for (const auto& aa : aa_sorted) {
            if (auto [color_present, color] = colors.get_value_if(aa); color_present) {
                color_for_aa[aa] = Color(color);
            }
            else
                aa_without_colors.push_back(aa);
        }
        if (!aa_without_colors.empty())
            throw unrecognized_mod{"the following AAs has no colors defined in settings \"colors\": " + acmacs::to_string(aa_without_colors)};
    }
    else {
        for (auto [index, aa]: acmacs::enumerate(aa_sorted)) {
            color_for_aa[aa] = fill_color_default(index, aa);
        }
    }

} // ModAminoAcids::make_color_for_aa

// ----------------------------------------------------------------------

void ModAminoAcids::aa_group(ChartDraw& aChartDraw, const rjson::v1::object& aGroup, bool aVerbose)
{
    const rjson::v1::array& pos_aa = aGroup["pos_aa"];
    std::vector<size_t> positions(pos_aa.size());
    std::transform(std::begin(pos_aa), std::end(pos_aa), std::begin(positions), [](const auto& src) { return std::stoul(src.str()); });
    std::string target_aas(pos_aa.size(), ' ');
    std::transform(std::begin(pos_aa), std::end(pos_aa), std::begin(target_aas), [](const auto& src) { return static_cast<std::string_view>(src).back(); });
    const auto& seqdb = seqdb::get(seqdb::ignore_errors::no, do_report_time(aVerbose));
    const auto aa_indices = seqdb.aa_at_positions_for_antigens(*aChartDraw.chart().antigens(), positions, aVerbose ? seqdb::report::yes : seqdb::report::no);
    if (const auto aap = aa_indices.find(target_aas); aap != aa_indices.end()) {
        auto styl = style();
        styl = point_style_from_json(aGroup);
        aChartDraw.modify(aap->second, styl, drawing_order());
        try { add_legend(aChartDraw, aap->second, styl, string::join(" ", std::begin(pos_aa), std::end(pos_aa)), args()["legend"]); } catch (rjson::v1::field_not_found&) {}
        if (aVerbose)
            std::cerr << "INFO: amino-acids group " << pos_aa << ": " << ' ' << aap->second.size() << '\n';
    }
    else {
        std::cerr << "WARNING: no \"" << target_aas << "\" in " << aa_indices << '\n';
    }

} // ModAminoAcids::aa_group

// ----------------------------------------------------------------------

Color ModAminoAcids::fill_color_default(size_t aIndex, std::string aAA)
{
    if (aAA == "X") {
        ++mIndexDiff;
        return Color(args().get_or_default("X_color", "grey25"));
    }
    if (mColors.empty()) {
        const auto ct = args().get_or_default("color_set", "ana");
        mColors = Color::distinct(ct == "google" ? Color::distinct_t::GoogleMaps : Color::distinct_t::Ana);
    }
    const auto index = aIndex - mIndexDiff;
    if (index < mColors.size())
        return mColors[index];
    else
        throw unrecognized_mod{"too few distinct colors in mod: " + args().to_json()};

} // ModAminoAcids::fill_color_default

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
