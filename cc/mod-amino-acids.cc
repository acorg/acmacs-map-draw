#include <tuple>

#include "acmacs-base/enumerate.hh"
#include "acmacs-base/fmt.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/filesystem.hh"
#include "acmacs-base/quicklook.hh"
#include "acmacs-base/color-distinct.hh"
#include "seqdb-3/compare.hh"
#include "acmacs-map-draw/mod-amino-acids.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/report-antigens.hh"
#include "acmacs-map-draw/select.hh"

// ----------------------------------------------------------------------

void ModAminoAcids::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = rjson::get_or(args(), "report", false);
    const auto report_names_threshold = rjson::get_or(args(), "report_names_threshold", 10UL);
    if (const auto& pos = args()["pos"]; !pos.is_null()) {
        aa_pos(aChartDraw, pos, verbose, report_names_threshold);
    }
    else if (const auto& groups = args()["groups"]; !groups.is_null()) {
        rjson::for_each(groups, [this,&aChartDraw,verbose,report_names_threshold](const rjson::value& group) { aa_group(aChartDraw, group, verbose, report_names_threshold); });
    }
    else {
        std::cerr << "No pos no groups" << '\n';
        throw unrecognized_mod{fmt::format("expected either \"pos\":[] or \"groups\":[] mod: {}", args())};
    }

} // ModAminoAcids::apply

// ----------------------------------------------------------------------

void ModAminoAcids::aa_pos(ChartDraw& aChartDraw, const rjson::value& aPos, bool aVerbose, size_t report_names_threshold)
{
    std::vector<size_t> positions;
    rjson::copy(aPos, positions);
    const auto aa_indices = aChartDraw.aa_at_pos1_for_antigens(positions);
    // aa_indices is std::map<std::string, std::vector<size_t>>
    std::vector<std::string> aa_sorted(aa_indices.size()); // most frequent aa first
    std::transform(std::begin(aa_indices), std::end(aa_indices), std::begin(aa_sorted), [](const auto& entry) -> std::string { return entry.first; });
    std::sort(std::begin(aa_sorted), std::end(aa_sorted), [&aa_indices](const auto& n1, const auto& n2) -> bool { return aa_indices.find(n1)->second.size() > aa_indices.find(n2)->second.size(); });
    std::map<std::string, Color> color_for_aa;
    make_color_for_aa(color_for_aa, aa_sorted);
    for (auto [index, aa] : acmacs::enumerate(aa_sorted)) {
        const auto& indices_for_aa = aa_indices.find(aa)->second;
        auto styl = style();
        if (auto ca = color_for_aa.find(aa); ca == color_for_aa.end())
            throw unrecognized_mod{"cannot find color for AA: " + aa + ", \"colors\" in the settings is not complete?"};
        else
            styl.fill = ca->second;
        styl.modified_fill = true;
        aChartDraw.modify(acmacs::chart::Indexes{indices_for_aa}, styl, drawing_order());
        if (const auto& legend = args()["legend"]; !legend.is_null())
            add_legend(aChartDraw, acmacs::chart::Indexes{indices_for_aa}, styl, aa, legend);
        if (aVerbose) {
            fmt::print(stderr, "INFO: amino-acids at {}: {} {}\n", aPos, aa, indices_for_aa.size());
            report_antigens(std::begin(indices_for_aa), std::end(indices_for_aa), aChartDraw, report_names_threshold);
        }
    }

    if (auto make_centroid = rjson::get_or(args(), "centroid", false); make_centroid) {
        auto layout = aChartDraw.projection().transformed_layout();
        for (auto [aa, indexes] : aa_indices) {
            if (indexes.size() > 1) {
                const auto sum_vectors = [](acmacs::PointCoordinates sum, const auto& point) {
                    for (auto dim : acmacs::range(std::get<0>(point).number_of_dimensions()))
                        sum[dim] += std::get<0>(point)[dim];
                    return sum;
                };

                // coordinates, distance to centroid of all points, distance to centroid of 90% closest points
                using element_t = std::tuple<acmacs::PointCoordinates, double, double>;
                std::vector<element_t> points(indexes.size(), {acmacs::PointCoordinates(layout->number_of_dimensions()), 0.0, 0.0});
                std::transform(indexes.begin(), indexes.end(), points.begin(), [layout](auto index) -> element_t { return {layout->at(index), -1, -1}; });
                acmacs::PointCoordinates centroid = std::accumulate(points.begin(), points.end(), acmacs::PointCoordinates(layout->number_of_dimensions(), 0.0), sum_vectors);
                centroid /= static_cast<double>(points.size());
                std::for_each(points.begin(), points.end(), [&centroid](auto& point) { std::get<1>(point) = acmacs::distance(std::get<0>(point), centroid); });
                std::sort(points.begin(), points.end(), [](const auto& p1, const auto& p2) { return std::get<1>(p1) < std::get<1>(p2); });
                const auto radius = std::get<1>(points.back());
                // std::cerr << "DEBUG: min dist:" << std::get<1>(points.front()) << " max:" << radius << '\n';

                const auto num_points_90 = static_cast<long>(static_cast<double>(points.size()) * 0.9);
                acmacs::PointCoordinates centroid_90 = std::accumulate(points.begin(), points.begin() + num_points_90, acmacs::PointCoordinates(layout->number_of_dimensions(), 0.0), sum_vectors);
                centroid_90 /= static_cast<double>(num_points_90);
                std::for_each(points.begin(), points.begin() + num_points_90, [&centroid_90](auto& point) { std::get<2>(point) = acmacs::distance(std::get<0>(point), centroid_90); });
                std::sort(points.begin(), points.begin() + num_points_90, [](const auto& p1, const auto& p2) { return std::get<2>(p1) < std::get<2>(p2); });
                const auto radius_90 = std::get<2>(*(points.begin() + num_points_90 - 1));

                // aChartDraw.point(centroid, Pixels{10}).color(color_for_aa.find(aa)->second, GREEN);
                aChartDraw.circle(centroid, Scaled{radius * 2}).color(TRANSPARENT, color_for_aa.find(aa)->second);
                aChartDraw.circle(centroid, Scaled{radius_90 * 2}).color(TRANSPARENT, color_for_aa.find(aa)->second);
            }
        }
    }

} // ModAminoAcids::aa_pos

// ----------------------------------------------------------------------

void ModAminoAcids::make_color_for_aa(std::map<std::string, Color>& color_for_aa, const std::vector<std::string>& aa_sorted)
{
    if (const auto& colors = args()["colors"]; !colors.is_null()) {
        std::vector<std::string> aa_without_colors;
        for (const auto& aa : aa_sorted) {
            if (const auto& color = colors[aa]; !color.is_null()) {
                color_for_aa[aa] = Color(color.to<std::string_view>());
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

void ModAminoAcids::aa_group(ChartDraw& aChartDraw, const rjson::value& aGroup, bool aVerbose, size_t report_names_threshold)
{
    const auto& pos_aa = aGroup["pos_aa"];
    std::vector<size_t> positions(pos_aa.size());
    rjson::transform(pos_aa, std::begin(positions), [](const rjson::value& src) -> size_t { return std::stoul(src.to<std::string>()); });
    std::string target_aas(pos_aa.size(), ' ');
    rjson::transform(pos_aa, std::begin(target_aas), [](const rjson::value& src) { return src.to<std::string_view>().back(); });
    const auto aa_indices = aChartDraw.aa_at_pos1_for_antigens(positions);
    if (const auto aap = aa_indices.find(target_aas); aap != aa_indices.end()) {
        auto styl = style();
        styl = point_style_from_json(aGroup);
        aChartDraw.modify(acmacs::chart::Indexes{aap->second}, styl, drawing_order());
        if (const auto& legend = args()["legend"]; !legend.is_null())
            add_legend(aChartDraw, acmacs::chart::Indexes{aap->second}, styl, acmacs::string::join(acmacs::string::join_space, positions), legend);
        if (aVerbose) {
            fmt::print(stderr, "INFO: amino-acids group {}: {}\n", pos_aa, aap->second.size());
            report_antigens(std::begin(aap->second), std::end(aap->second), aChartDraw, report_names_threshold);
        }
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
        return Color(rjson::get_or(args(), "X_color", "grey25"));
    }
    if (mColors.empty()) {
        const auto ct = rjson::get_or(args(), "color_set", "ana");
        mColors = acmacs::color::distinct(ct == "google" ? acmacs::color::distinct_t::GoogleMaps : acmacs::color::distinct_t::Ana);
    }
    const auto index = aIndex - mIndexDiff;
    if (index < mColors.size())
        return mColors[index];
    else
        return mColors.back();
    // throw unrecognized_mod{fmt::format("too few distinct colors in mod ({}): {}", mColors.size(), args())};

} // ModAminoAcids::fill_color_default

// ----------------------------------------------------------------------

void ModCompareSequences::apply(ChartDraw& aChartDraw, const rjson::value& aModData)
{
    acmacs::chart::Indexes indexes1, indexes2;
    if (const auto& select1 = args()["select1"]; !select1.is_null())
        indexes1 = SelectAntigens(acmacs::verbose::no, 10).select(aChartDraw, select1);
    else
        throw unrecognized_mod{fmt::format("no select1 in mod: {}", args())};
    if (const auto& select2 = args()["select2"]; !select2.is_null())
        indexes2 = SelectAntigens(acmacs::verbose::no, 10).select(aChartDraw, select2);
    else
        throw unrecognized_mod{fmt::format("no select2 in mod: {}", args())};

    const auto& matched = aChartDraw.match_seqdb();
    auto set1 = matched.filter_by_indexes(indexes1);
    auto set2 = matched.filter_by_indexes(indexes2);

    if (const auto& format = args()["format"]; !format.is_null() && format.to<std::string_view>() == "html") {
        std::string filename{"-"};
        if (const auto& output1 = args()["output"]; !output1.is_null()) {
            filename = output1.to<std::string_view>();
        }
        else if (const auto& output2 = aModData["output_pdf"]; !output2.is_null()) {
            filename = fs::path(output2.to<std::string_view>()).replace_extension(".html");
        }
        else {
            filename = "/d/a.html";
        }
        acmacs::file::write(filename, acmacs::seqdb::compare_report_html(filename, set1, set2));
        if (const auto& open = args()["open"]; open.is_null() || open.to<bool>())
            acmacs::open_or_quicklook(true, false, filename, 0, 0);
    }
    else
        fmt::print("{}\n\n", acmacs::seqdb::compare_report_text(set1, set2));

} // ModCompareSequences::apply

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
