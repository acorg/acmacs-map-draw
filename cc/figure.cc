#include "acmacs-map-draw/figure.hh"

// ----------------------------------------------------------------------

// http://geomalgorithms.com/a03-_inclusion.html
// returns winding number, i.e. 0 if point is outside polygon defined by path, non-zero otherwise
int acmacs::mapi::v1::Figure::winding_number(const acmacs::PointCoordinates& point, const vertices_t& path, const ChartDraw& chart_draw)
{
    // >0 for point left of the line through p0 and p1
    // =0 for point on the line
    // <0 for point right of the line
    const auto is_left = [&point](auto p0, auto p1) -> double { return ((p1.x() - p0.x()) * (point.y() - p0.y()) - (point.x() - p0.x()) * (p1.y() - p0.y())); };

    int wn{0};
    auto path_end = std::prev(path.end(), path.front() == path.back() ? 1 : 0);
    for (auto vi = path.begin(); vi != path_end; ++vi) {
        auto vi_next = std::next(vi);
        if (vi_next == path_end)
            vi_next = path.begin();
        const auto vi_transformed = vi->get_transformed(chart_draw);
        const auto vi_next_transformed = vi_next->get_transformed(chart_draw);
        if (vi_transformed.y() <= point.y()) {
            if (vi_next_transformed.y() > point.y() && is_left(vi_transformed, vi_next_transformed) > 0)
                ++wn;
        }
        else {
            if (vi_next_transformed.y() <= point.y() && is_left(vi_transformed, vi_next_transformed) < 0)
                --wn;
        }
    }
    return wn;

} // acmacs::mapi::v1::Figure::winding_number

// ----------------------------------------------------------------------



// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
