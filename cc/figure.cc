#include "acmacs-map-draw/figure.hh"

// ----------------------------------------------------------------------

acmacs::mapi::v1::Figure::Figure(const FigureRaw& figure_raw, const ChartDraw& chart_draw)
    : vertices(figure_raw.vertices.size(), PointCoordinates{PointCoordinates::nan2D}), close{figure_raw.close}
{
    std::transform(std::begin(figure_raw.vertices), std::end(figure_raw.vertices), std::begin(vertices),
                   [&chart_draw](const map_elements::v2::Coordinates& src) { return src.get_transformed(chart_draw); });

} // acmacs::mapi::v1::Figure::Figure

// ----------------------------------------------------------------------

// http://geomalgorithms.com/a03-_inclusion.html
// returns winding number, i.e. 0 if point is outside polygon defined by path, non-zero otherwise
int acmacs::mapi::v1::Figure::winding_number(const acmacs::PointCoordinates& point, const std::vector<acmacs::PointCoordinates>& path)
{
    // >0 for point left of the line through p0 and p1
    // =0 for point on the line
    // <0 for point right of the line
    const auto is_left = [&point](auto p0, auto p1) -> double { return ((p1->x() - p0->x()) * (point.y() - p0->y()) - (point.x() - p0->x()) * (p1->y() - p0->y())); };

    int wn{0};
    auto path_end = std::prev(path.end(), path.front() == path.back() ? 1 : 0);
    for (auto vi = path.begin(); vi != path_end; ++vi) {
        auto vi_next = std::next(vi);
        if (vi_next == path_end)
            vi_next = path.begin();
        if (vi->y() <= point.y()) {
            if (vi_next->y() > point.y() && is_left(vi, vi_next) > 0)
                ++wn;
        }
        else {
            if (vi_next->y() <= point.y() && is_left(vi, vi_next) < 0)
                --wn;
        }
    }
    return wn;

} // acmacs::mapi::v1::Figure::winding_number

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
