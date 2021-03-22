#include "acmacs-chart-2/selected-antigens-sera.hh"
#include "acmacs-map-draw/mapi-procrustes.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/map-elements-v2.hh"

// ----------------------------------------------------------------------

constexpr const std::string_view sProcrustesArrowElementKeyword{"procrustes-arrow"};

// ----------------------------------------------------------------------

acmacs::mapi::v1::distances_t acmacs::mapi::v1::procrustes_arrows(ChartDraw& chart_draw, const acmacs::chart::Projection& secondary_projection, const acmacs::chart::CommonAntigensSera& common, const acmacs::chart::ProcrustesData& procrustes_data, const ArrowPlotSpec& arrow_plot_spec)
{
    auto secondary_layout = procrustes_data.apply(*secondary_projection.layout());
    auto primary_layout = chart_draw.chart(0).modified_projection().transformed_layout();
    const std::vector<acmacs::chart::CommonAntigensSera::common_t> common_points = common.points(acmacs::chart::CommonAntigensSera::subset::all);
    distances_t distances; // point_no in primary chart and its arrow distance
    for (size_t point_no = 0; point_no < common_points.size(); ++point_no) {
        const auto primary_coords = primary_layout->at(common_points[point_no].primary), secondary_coords = secondary_layout->at(common_points[point_no].secondary);
        const auto distance{acmacs::distance(primary_coords, secondary_coords)};
        distances.emplace_back(common_points[point_no].primary, distance);
        if (distance > arrow_plot_spec.threshold) {
            auto& path = chart_draw.map_elements().add<map_elements::v2::Path>(sProcrustesArrowElementKeyword);
            path.outline(arrow_plot_spec.outline);
            path.outline_width(arrow_plot_spec.line_width);
            path.data().close = false;
            path.data().vertices.emplace_back(map_elements::v2::Coordinates::not_transformed{primary_coords});
            path.data().vertices.emplace_back(map_elements::v2::Coordinates::not_transformed{secondary_coords});
            auto& arrow = path.arrows().emplace_back();
            arrow.at(1);
            arrow.fill(arrow_plot_spec.arrow_fill);
            arrow.outline(arrow_plot_spec.arrow_outline);
            arrow.width(arrow_plot_spec.arrow_width);
            arrow.outline_width(arrow_plot_spec.arrow_outline_width);
        }
    }
    return distances;

} // acmacs::mapi::v1::procrustes_arrows

// ----------------------------------------------------------------------

std::pair<acmacs::mapi::v1::distances_t, acmacs::chart::ProcrustesData> acmacs::mapi::v1::procrustes_arrows(ChartDraw& chart_draw, const acmacs::chart::Projection& secondary_projection, const acmacs::chart::CommonAntigensSera& common, acmacs::chart::procrustes_scaling_t scaling, const ArrowPlotSpec& arrow_plot_spec)
{
    auto procrustes_data = acmacs::chart::procrustes(chart_draw.chart(0).modified_projection(), secondary_projection, common.points(acmacs::chart::CommonAntigensSera::subset::all), scaling);
    return {procrustes_arrows(chart_draw, secondary_projection, common, procrustes_data, arrow_plot_spec), procrustes_data};

} // acmacs::mapi::v1::procrustes_arrows

// ----------------------------------------------------------------------

namespace acmacs::mapi::inline v1
{
    static inline void make_line(map_elements::v2::Path& path, map_elements::v2::Coordinates&& p1, map_elements::v2::Coordinates&& p2, const acmacs::color::Modifier& outline, Pixels outline_width)
    {
        path.data().close = false;
        path.data().vertices.emplace_back(std::move(p1));
        path.data().vertices.emplace_back(std::move(p2));
        path.outline(outline);
        path.outline_width(outline_width);

    } // make_line

} // namespace acmacs::mapi::inline v1

// ----------------------------------------------------------------------

void acmacs::mapi::v1::connection_lines(ChartDraw& chart_draw, const acmacs::chart::SelectedAntigensModify& antigens, const acmacs::chart::SelectedSeraModify& sera, const ConnectionLinePlotSpec& plot_spec)
{
    const auto number_of_antigens = chart_draw.chart().number_of_antigens();
    auto layout = chart_draw.chart(0).modified_layout();
    std::vector<std::pair<size_t, size_t>> lines_to_draw;
    auto titers = chart_draw.chart().titers();
    for (const auto ag_no : ranges::views::filter(antigens.indexes, [&layout](size_t index) { return layout->point_has_coordinates(index); })) {
        for (const auto sr_no : ranges::views::filter(sera.indexes, [&layout, number_of_antigens](size_t index) { return layout->point_has_coordinates(index + number_of_antigens); })) {
            if (const auto titer = titers->titer(ag_no, sr_no); !titer.is_dont_care()) {
                make_line(chart_draw.map_elements().add<map_elements::v2::Path>(), map_elements::v2::Coordinates::points{ag_no}, map_elements::v2::Coordinates::points{sr_no + number_of_antigens},
                            plot_spec.color, plot_spec.line_width);
                lines_to_draw.emplace_back(ag_no, sr_no);
            }
        }
    }
//     if (getenv("report"sv).to<bool>())
    AD_INFO("connection lines: ({}) {}", lines_to_draw.size(), lines_to_draw);

} // acmacs::mapi::v1::connection_lines

// ----------------------------------------------------------------------

void acmacs::mapi::v1::error_lines(ChartDraw& chart_draw, const acmacs::chart::SelectedAntigensModify& antigens, const acmacs::chart::SelectedSeraModify& sera, const ErrorLinePlotSpec& plot_spec)
{

} // acmacs::mapi::v1::error_lines

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
