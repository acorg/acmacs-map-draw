#include "acmacs-base/pybind11.hh"
#include "locationdb/locdb.hh"
#include "acmacs-chart/chart.hh"
#include "acmacs-chart/ace.hh"
#include "acmacs-chart/lispmds.hh"
#include "hidb/hidb.hh"
#include "seqdb/seqdb.hh"

#include "draw.hh"
#include "geographic-map.hh"
#include "time-series.hh"
#include "vaccines.hh"

// ----------------------------------------------------------------------

static inline std::string get_point_style_shape(PointStyle& aStyle)
{
    std::string shape;
    switch (aStyle.shape()) {
      case PointStyle::Shape::Circle:
          shape = "circle";
          break;
      case PointStyle::Shape::Box:
          shape = "box";
          break;
      case PointStyle::Shape::Triangle:
          shape = "triangle";
          break;
      case PointStyle::Shape::NoChange:
          break;
    }
    return shape;
}

// ----------------------------------------------------------------------

// static inline PointStyleDraw* point_style_kw(PointStyleDraw& aStyle, py::args args, py::kwargs kwargs)
// {
//     PointStyleDraw* obj = new (&aStyle) PointStyleDraw(PointStyle::Empty);
//     std::cerr << "point_style_kw" << std::endl;
//     return obj;
// }

// static inline PointStyleDraw* point_style_modify_kw(PointStyleDraw* aStyle, py::args args, py::kwargs kwargs)
// {
//     std::cerr << "point_style_modify_kw" << std::endl;
//     return aStyle;
// }

// static inline PointStyleDraw* new_point_style_kw(py::args args, py::kwargs kwargs)
// {
//     PointStyleDraw* style = new PointStyleDraw(PointStyle::Empty);
//     std::cerr << "new_point_style_kw" << std::endl;
//     return style;
// }

// ----------------------------------------------------------------------

template <typename Iter> class PyTimeSeriesIterator
{
 public:
    inline PyTimeSeriesIterator(Iter&& begin, Iter&& end) : mStarted(false), mCurrent(begin), mEnd(end) {}
    inline PyTimeSeriesIterator<Iter>& iter() { return *this; }
    inline Iter& next()
        {
            if (mStarted && mCurrent != mEnd)
                ++mCurrent;
            else
                mStarted = true;
            if (mCurrent == mEnd)
                throw py::stop_iteration();
            return mCurrent;
        }

 private:
    bool mStarted;
    Iter mCurrent, mEnd;
};

template <typename Iter> inline auto make_py_ts_iter(Iter&& begin, Iter&& end) { return PyTimeSeriesIterator<Iter>(std::move(begin), std::move(end)); }

// ----------------------------------------------------------------------

PYBIND11_MODULE(acmacs_map_draw_backend, m)
{
    m.doc() = "Acmacs map draw plugin";

      // ----------------------------------------------------------------------
      // acmacs-base/time-series
      // ----------------------------------------------------------------------

    py::class_<TimeSeriesIterator>(m, "TimeSeriesIterator")
            .def("numeric_name", &TimeSeriesIterator::numeric_name)
            .def("text_name", &TimeSeriesIterator::text_name)
            .def("first_date", &TimeSeriesIterator::first_date)
            .def("after_last_date", &TimeSeriesIterator::after_last_date)
            ;

    py::class_<MonthlyTimeSeries::Iterator, TimeSeriesIterator>(m, "MonthlyTimeSeries_Iterator");
    py::class_<YearlyTimeSeries::Iterator, TimeSeriesIterator>(m, "YearlyTimeSeries_Iterator");
    py::class_<WeeklyTimeSeries::Iterator, TimeSeriesIterator>(m, "WeeklyTimeSeries_Iterator");

    py::class_<PyTimeSeriesIterator<MonthlyTimeSeries::Iterator>>(m, "PyTimeSeriesIterator_MonthlyTimeSeries")
            .def("__iter__", &PyTimeSeriesIterator<MonthlyTimeSeries::Iterator>::iter)
            .def("__next__", &PyTimeSeriesIterator<MonthlyTimeSeries::Iterator>::next)
            ;

    py::class_<PyTimeSeriesIterator<YearlyTimeSeries::Iterator>>(m, "PyTimeSeriesIterator_YearlyTimeSeries")
            .def("__iter__", &PyTimeSeriesIterator<YearlyTimeSeries::Iterator>::iter)
            .def("__next__", &PyTimeSeriesIterator<YearlyTimeSeries::Iterator>::next)
            ;

    py::class_<PyTimeSeriesIterator<WeeklyTimeSeries::Iterator>>(m, "PyTimeSeriesIterator_WeeklyTimeSeries")
            .def("__iter__", &PyTimeSeriesIterator<WeeklyTimeSeries::Iterator>::iter)
            .def("__next__", &PyTimeSeriesIterator<WeeklyTimeSeries::Iterator>::next)
            ;

    py::class_<MonthlyTimeSeries>(m, "MonthlyTimeSeries")
            .def(py::init<std::string, std::string>(), py::arg("start"), py::arg("end"))
              // .def("__iter__", [](MonthlyTimeSeries& v) { return py::make_iterator(v.begin(), v.end()); }, py::keep_alive<0, 1>()) /* Keep MonthlyTimeSeries alive while iterator is used */
            .def("__iter__", [](MonthlyTimeSeries& v) { return make_py_ts_iter(v.begin(), v.end()); }, py::keep_alive<0, 1>()) /* Keep MonthlyTimeSeries alive while iterator is used */
            ;

    py::class_<YearlyTimeSeries>(m, "YearlyTimeSeries")
            .def(py::init<std::string, std::string>(), py::arg("start"), py::arg("end"))
              // .def("__iter__", [](YearlyTimeSeries& v) { return py::make_iterator(v.begin(), v.end()); }, py::keep_alive<0, 1>()) /* Keep YearlyTimeSeries alive while iterator is used */
            .def("__iter__", [](YearlyTimeSeries& v) { return make_py_ts_iter(v.begin(), v.end()); }, py::keep_alive<0, 1>()) /* Keep YearlyTimeSeries alive while iterator is used */
            ;

    py::class_<WeeklyTimeSeries>(m, "WeeklyTimeSeries")
            .def(py::init<std::string, std::string>(), py::arg("start"), py::arg("end"))
              // .def("__iter__", [](WeeklyTimeSeries& v) { return py::make_iterator(v.begin(), v.end()); }, py::keep_alive<0, 1>()) /* Keep WeeklyTimeSeries alive while iterator is used */
            .def("__iter__", [](WeeklyTimeSeries& v) { return make_py_ts_iter(v.begin(), v.end()); }, py::keep_alive<0, 1>()) /* Keep WeeklyTimeSeries alive while iterator is used */
            ;

      // ----------------------------------------------------------------------
      // Draw
      // ----------------------------------------------------------------------

    py::class_<Color>(m, "Color")
            .def(py::init<std::string>(), py::arg("color") = "black")
            .def("__str__", &Color::to_string)
            .def("to_string", &Color::to_string)
            .def("to_hex_string", &Color::to_string)
            .def("light", &Color::light)
            ;

    m.def("distinct_colors", &Color::distinct_colors);

    py::class_<PointStyle> point_style(m, "PointStyle");
    py::enum_<enum PointStyle::Empty>(point_style, "PointStyle_Empty").value("Empty", PointStyle::Empty).export_values();
    point_style
            .def(py::init<enum PointStyle::Empty>(), py::arg("_") = PointStyle::Empty)
              // .def("__init__", &point_style_kw)
              // .def("modify", &point_style_modify_kw)
            .def("show", [](PointStyle& style, bool show) -> PointStyle& { return style.show(show ? PointStyle::Shown::Shown : PointStyle::Shown::Hidden); }, py::arg("show") = true)
            .def("hide", &PointStyle::hide)
            .def("shape", py::overload_cast<std::string>(&PointStyle::shape), py::arg("shape"))
            .def("shape", &get_point_style_shape)
            .def("fill", [](PointStyle& style, std::string color) -> PointStyle& { return style.fill(color); }, py::arg("fill"))
            .def("fill", [](PointStyle& style, std::string color, double light) -> PointStyle& { Color c{color}; c.light(light); return style.fill(c); }, py::arg("fill"), py::arg("light"))
            .def("fill", py::overload_cast<>(&PointStyle::fill, py::const_))
            .def("outline", [](PointStyle& style, std::string color) -> PointStyle& { return style.outline(color); }, py::arg("outline"))
            .def("outline", [](PointStyle& style, std::string color, double light) -> PointStyle& { Color c{color}; c.light(light); return style.outline(c); }, py::arg("outline"), py::arg("light"))
            .def("outline", py::overload_cast<>(&PointStyle::outline, py::const_))
            .def("size", [](PointStyle& style, double aSize) -> PointStyle& { return style.size(Pixels{aSize}); }, py::arg("size"))
            .def("outline_width", [](PointStyle& style, double aOutlineWidth) -> PointStyle& { return style.outline_width(Pixels{aOutlineWidth}); }, py::arg("outline_width"))
            .def("outline_width", py::overload_cast<>(&PointStyle::outline_width, py::const_))
            .def("aspect", py::overload_cast<double>(&PointStyle::aspect), py::arg("aspect"))
            .def("rotation", py::overload_cast<double>(&PointStyle::rotation), py::arg("rotation"))
            .def("scale", &PointStyle::scale, py::arg("scale"))
            .def("scale_outline", &PointStyle::scale_outline, py::arg("scale"))
            ;

    py::class_<DrawingOrder>(m, "DrawingOrder")
            .def("raise_", &DrawingOrder::raise, py::arg("point_no"))
            .def("lower", &DrawingOrder::lower, py::arg("point_no"))
            ;

    py::class_<LegendPointLabel>(m, "LegendPointLabel")
            .def("add_line", [](LegendPointLabel& legend, std::string outline, std::string fill, std::string label) { legend.add_line(outline, fill, label); }, py::arg("outline"), py::arg("fill"), py::arg("label"))
            .def("add_line", [](LegendPointLabel& legend, std::string outline, std::string fill, std::string label, double light) { Color f{fill}, o{outline}; f.light(light); o.light(light); legend.add_line(o, f, label); }, py::arg("outline"), py::arg("fill"), py::arg("label"), py::arg("light"))
            .def("label_size", &LegendPointLabel::label_size, py::arg("label_size"))
            .def("point_size", &LegendPointLabel::point_size, py::arg("point_size"))
            .def("background", [](LegendPointLabel& legend, std::string aBackground) { legend.background(aBackground); }, py::arg("background"))
            .def("border_color", [](LegendPointLabel& legend, std::string aBorderColor) { legend.border_color(aBorderColor); }, py::arg("border_color"))
            .def("border_width", &LegendPointLabel::border_width, py::arg("border_width"))
            ;

    py::class_<Title>(m, "Title")
            .def("remove_all_lines", &Title::remove_all_lines)
            .def("add_line", [](Title& aTitle, std::string aLine) -> Title& { return aTitle.add_line(aLine); }, py::arg("text"))
            .def("add_line", [](Title& aTitle, const std::vector<std::string>& aLines) -> Title& { for (const auto& line: aLines) { aTitle.add_line(line); } return aTitle; }, py::arg("lines"))
            .def("show", py::overload_cast<bool>(&Title::show), py::arg("show"))
            .def("offset", py::overload_cast<double, double>(&Title::offset), py::arg("x"), py::arg("y"))
            .def("padding", &Title::padding, py::arg("padding"))
            .def("text_size", &Title::text_size, py::arg("text_size"))
            .def("text_color", [](Title& aTitle, std::string aColor) -> Title& { return aTitle.text_color(aColor); }, py::arg("color"))
            .def("background", [](Title& aTitle, std::string aBackground) -> Title& { return aTitle.background(aBackground); }, py::arg("color"))
            .def("border_color", [](Title& aTitle, std::string aBorderColor) -> Title& { return aTitle.border_color(aBorderColor); }, py::arg("color"))
            .def("border_width", &Title::border_width, py::arg("width"))
            .def("weight", &Title::weight, py::arg("weight"))
            .def("slant", &Title::slant, py::arg("slant"))
            .def("font_family", &Title::font_family, py::arg("font_family"))
            ;

    py::class_<Label>(m, "Label")
            .def("offset", &Label::offset, py::arg("x"), py::arg("y"), py::return_value_policy::reference)
            .def("offset", [](Label& aLabel, const std::vector<double>& aOffset) -> Label& { return aLabel.offset(aOffset[0], aOffset[1]); }, py::arg("offset"), py::return_value_policy::reference)
            .def("display_name", &Label::display_name, py::arg("name"), py::return_value_policy::reference)
            .def("color", [](Label& aLabel, std::string aColor) -> Label& { return aLabel.color(aColor); }, py::arg("color"), py::return_value_policy::reference)
            .def("size", &Label::size, py::arg("size"), py::return_value_policy::reference)
            .def("weight", &Label::weight, py::arg("weight"), py::return_value_policy::reference)
            .def("slant", &Label::slant, py::arg("slant"), py::return_value_policy::reference)
            .def("font_family", &Label::font_family, py::arg("font_family"), py::return_value_policy::reference)
            ;

    py::class_<SerumCircle>(m, "SerumCircle")
            .def("fill", [](SerumCircle& sc, std::string color) { sc.fill(color); }, py::arg("color") = "pink")
            .def("outline", [](SerumCircle& sc, std::string color, double line_width) { sc.outline(color, line_width); }, py::arg("color") = "pink", py::arg("line_width") = 1.0)
            .def("radius_line", [](SerumCircle& sc, std::string color, double line_width) { sc.radius_line(color, line_width); }, py::arg("color") = "pink", py::arg("line_width") = 1.0)
            .def("angles", &SerumCircle::angles, py::arg("start") = 0.0, py::arg("end") = 0.0)
            .def("radius_line_no_dash", &SerumCircle::radius_line_no_dash)
            .def("radius_line_dash1", &SerumCircle::radius_line_dash1)
            .def("radius_line_dash2", &SerumCircle::radius_line_dash2)
            ;

    py::class_<Arrow>(m, "Arrow")
            .def("from_to", [](Arrow& aArrow, double x1, double y1, double x2, double y2) { aArrow.from_to({x1, y1}, {x2, y2}); }, py::arg("x1"), py::arg("y1"), py::arg("x2"), py::arg("y2"))
            .def("color", [](Arrow& aArrow, std::string line_color, std::string arrow_head_color) { aArrow.color(line_color, arrow_head_color); }, py::arg("line_color"), py::arg("arrow_head_color"))
            .def("arrow_head_filled", &Arrow::arrow_head_filled, py::arg("filled"))
            .def("line_width", &Arrow::line_width, py::arg("width"))
            .def("arrow_width", &Arrow::arrow_width, py::arg("width"))
            ;

    py::class_<Point>(m, "Point")
            .def("center", [](Point& aPoint, double x, double y) { aPoint.center({x, y}); }, py::arg("x"), py::arg("y"))
            .def("size", [](Point& aPoint, double size) { aPoint.size(Pixels{size}); }, py::arg("size"))
            .def("color", [](Point& aPoint, std::string fill_color, std::string outline_color) { aPoint.color(fill_color, outline_color); }, py::arg("fill_color"), py::arg("outline_color"))
            .def("outline_width", &Point::outline_width, py::arg("outline_width"))
            ;

    py::class_<ChartDraw>(m, "ChartDraw")
            .def(py::init<Chart&, size_t>(), py::arg("chart"), py::arg("projection_no") = 0)
            .def("prepare", &ChartDraw::prepare)
            .def("point_styles", &ChartDraw::point_styles)
            .def("point_styles_base", &ChartDraw::point_styles_base)
            .def("calculate_viewport", &ChartDraw::calculate_viewport)
            .def("draw", py::overload_cast<std::string, double>(&ChartDraw::draw, py::const_), py::arg("filename"), py::arg("size"))
            .def("hide_all_except", &ChartDraw::hide_all_except, py::arg("not_hide"))
            .def("mark_egg_antigens", &ChartDraw::mark_egg_antigens)
            .def("mark_reassortant_antigens", &ChartDraw::mark_reassortant_antigens)
            .def("all_grey", &ChartDraw::mark_all_grey, py::arg("color") = Color("grey80"))
            .def("scale_points", &ChartDraw::scale_points, py::arg("scale"), py::arg("outline_scale") = 1.0, py::doc("outline_scale=0 means use point scale for outline too"))
            .def("modify", [](ChartDraw& cd, size_t aIndex, const PointStyle& aStyle, bool aRaise, bool aLower) { cd.modify(aIndex, aStyle, aRaise ? ChartDraw::Raise : (aLower ? ChartDraw::Lower : ChartDraw::NoOrderChange)); }, py::arg("index"), py::arg("style"), py::arg("raise_") = false, py::arg("lower") = false)
            .def("modify_serum", [](ChartDraw& cd, size_t aIndex, const PointStyle& aStyle, bool aRaise, bool aLower) { cd.modify_serum(aIndex, aStyle, aRaise ? ChartDraw::Raise : (aLower ? ChartDraw::Lower : ChartDraw::NoOrderChange)); }, py::arg("index"), py::arg("style"), py::arg("raise_") = false, py::arg("lower") = false)
            .def("modify", [](ChartDraw& cd, const std::vector<size_t>& indices, const PointStyle& style, bool aRaise, bool aLower) { cd.modify(indices.begin(), indices.end(), style, aRaise ? ChartDraw::Raise : (aLower ? ChartDraw::Lower : ChartDraw::NoOrderChange)); }, py::arg("indices"), py::arg("style"), py::arg("raise_") = false, py::arg("lower") = false)
            .def("modify_sera", [](ChartDraw& cd, const std::vector<size_t>& indices, const PointStyle& style, bool aRaise, bool aLower) { cd.modify_sera(indices.begin(), indices.end(), style, aRaise ? ChartDraw::Raise : (aLower ? ChartDraw::Lower : ChartDraw::NoOrderChange)); }, py::arg("indices"), py::arg("style"), py::arg("raise_") = false, py::arg("lower") = false)
            .def("drawing_order", &ChartDraw::drawing_order, py::return_value_policy::reference)
            .def("rotate", &ChartDraw::rotate, py::arg("angle"))
            .def("flip", &ChartDraw::flip, py::arg("x"), py::arg("y"))
            .def("flip_ns", [](ChartDraw& cd) { cd.flip(1, 0); })
            .def("flip_ew", [](ChartDraw& cd) { cd.flip(0, 1); })
            .def("transformation", &ChartDraw::transformation, py::return_value_policy::reference)
            .def("viewport", py::overload_cast<double, double, double>(&ChartDraw::viewport), py::arg("x"), py::arg("y"), py::arg("size"))
            .def("viewport", [](ChartDraw& cd, const std::vector<double>& a) { cd.viewport(a[0], a[1], a[2]); }, py::arg("viewport"))
            .def("background_color", [](ChartDraw& cd, std::string color) { cd.background_color(color); }, py::arg("color"))
            .def("grid", [](ChartDraw& cd, std::string color, double line_width) { cd.grid(color, line_width); }, py::arg("color") = "grey80", py::arg("line_width") = 1.0)
            .def("border", [](ChartDraw& cd, std::string color, double line_width) { cd.border(color, line_width); }, py::arg("color") = "black", py::arg("line_width") = 1.0)
            .def("continent_map", [](ChartDraw& cd, std::vector<double> aOrigin, double aWidth) { cd.continent_map({aOrigin[0], aOrigin[1]}, Pixels{aWidth}); }, py::arg("origin"), py::arg("width"), py::doc("Origin and width are in pixels. Negative values in orinin mean from right/bottom of the surface"))
            .def("legend", [](ChartDraw& cd, std::vector<double> aOrigin) -> LegendPointLabel& { return cd.legend({aOrigin[0], aOrigin[1]}); }, py::arg("origin"), py::return_value_policy::reference, py::doc("Origin is in pixels. Negative values in orinin mean from right/bottom of the surface"))
            .def("legend", [](ChartDraw& cd) -> LegendPointLabel& { return cd.legend(); }, py::return_value_policy::reference)
            .def("remove_legend", &ChartDraw::remove_legend)
            .def("title", [](ChartDraw& cd, std::vector<double> aOrigin) -> Title& { return cd.title({aOrigin[0], aOrigin[1]}); }, py::arg("origin"), py::return_value_policy::reference, py::doc("Origin is in pixels. Negative values in orinin mean from right/bottom of the surface"))
            .def("title", [](ChartDraw& cd) -> Title& { return cd.title(); }, py::return_value_policy::reference)
            .def("label", py::overload_cast<size_t>(&ChartDraw::add_label), py::arg("index"), py::return_value_policy::reference)
            .def("remove_label", py::overload_cast<size_t>(&ChartDraw::remove_label), py::arg("index"))
            .def("serum_circle", [](ChartDraw& cd, size_t aSerumNo, double aRadius) -> SerumCircle& { return cd.serum_circle(aSerumNo, Scaled(aRadius)); }, py::arg("serum_no"), py::arg("radius"), py::return_value_policy::reference)
            .def("arrow", [](ChartDraw& cd, double x1, double y1, double x2, double y2) -> Arrow& { return cd.arrow({x1, y1}, {x2, y2}); }, py::arg("x1"), py::arg("y1"), py::arg("x2"), py::arg("y2"), py::return_value_policy::reference)
            .def("point", [](ChartDraw& cd, double x, double y, double size) -> Point& { return cd.point({x, y}, Pixels{size}); }, py::arg("x"), py::arg("y"), py::arg("size"), py::return_value_policy::reference)
            ;

      // ----------------------------------------------------------------------

    py::class_<GeographicMapDraw>(m, "GeographicMapDraw")
              // .def(py::init<Color, Pixels>())
            .def("draw", static_cast<void (GeographicMapDraw::*)(std::string, double)>(&GeographicMapDraw::draw), py::arg("filename"), py::arg("image_width"))
            .def("title", &GeographicMapDraw::title, py::return_value_policy::reference)
            ;

    py::class_<GeographicMapWithPointsFromHidb, GeographicMapDraw>(m, "GeographicMapWithPointsFromHidb")
            .def(py::init<const hidb::HiDb&, const LocDb&, double, double, std::string, double>(), py::arg("hidb"), py::arg("locdb"), py::arg("point_size_in_pixels") = 4.0, py::arg("point_density") = 0.8, py::arg("outline_color") = "grey63", py::arg("outline_width") = 0.5)
            .def("add_points_from_hidb_colored_by_continent", &GeographicMapWithPointsFromHidb::add_points_from_hidb_colored_by_continent, py::arg("continent_color"), py::arg("color_override"), py::arg("start_date"), py::arg("end_date"))
            .def("add_points_from_hidb_colored_by_clade", &GeographicMapWithPointsFromHidb::add_points_from_hidb_colored_by_clade, py::arg("clade_color"), py::arg("color_override"), py::arg("seqdb"), py::arg("start_date"), py::arg("end_date"))
            .def("add_points_from_hidb_colored_by_lineage", &GeographicMapWithPointsFromHidb::add_points_from_hidb_colored_by_lineage, py::arg("lineage_color"), py::arg("color_override"), py::arg("start_date"), py::arg("end_date"))
            ;

    py::class_<GeographicTimeSeriesBase>(m, "GeographicTimeSeries")
            .def("title", &GeographicTimeSeriesBase::title, py::return_value_policy::reference)
            .def("draw_colored_by_continent", [](GeographicTimeSeriesBase& aTS, std::string aPrefix, const std::map<std::string, std::string>& aContinentColor, const std::map<std::string, std::string>& aColorOverride, double aImageWidth) { aTS.draw(aPrefix, ColoringByContinent(aContinentColor, aTS.locdb()), aColorOverride, aImageWidth); }, py::arg("filename_prefix"), py::arg("continent_color"), py::arg("color_override"), py::arg("image_width"))
            .def("draw_colored_by_clade", [](GeographicTimeSeriesBase& aTS, std::string aPrefix, const std::map<std::string, std::string>& aCladeColor, const std::map<std::string, std::string>& aColorOverride, const seqdb::Seqdb& aSeqdb, double aImageWidth) { aTS.draw(aPrefix, ColoringByClade(aCladeColor, aSeqdb), aColorOverride, aImageWidth); }, py::arg("filename_prefix"), py::arg("clade_color"), py::arg("color_override"), py::arg("seqdb"), py::arg("image_width"))
            .def("draw_colored_by_lineage", [](GeographicTimeSeriesBase& aTS, std::string aPrefix, const std::map<std::string, std::string>& aLineageColor, const std::map<std::string, std::string>& aColorOverride, double aImageWidth) { aTS.draw(aPrefix, ColoringByLineage(aLineageColor), aColorOverride, aImageWidth); }, py::arg("filename_prefix"), py::arg("lineage_color"), py::arg("color_override"), py::arg("image_width"))
            .def("draw_colored_by_lineage_and_deletion_mutants", [](GeographicTimeSeriesBase& aTS, std::string aPrefix, const std::map<std::string, std::string>& aLineageColor, std::string aDeletionMutantColor, const std::map<std::string, std::string>& aColorOverride, const seqdb::Seqdb& aSeqdb, double aImageWidth) { aTS.draw(aPrefix, ColoringByLineageAndDeletionMutants(aLineageColor, aDeletionMutantColor, aSeqdb), aColorOverride, aImageWidth); }, py::arg("filename_prefix"), py::arg("lineage_color"), py::arg("deletion_mutant_color"), py::arg("color_override"), py::arg("seqdb"), py::arg("image_width"))
            ;

    m.def("geographic_time_series_monthly", [](std::string aStart, std::string aEnd, const hidb::HiDb& aHiDb, const LocDb& aLocDb, double aPointSize, double aPointDensity, std::string aOutlineColor, double aOutlineWidth) -> GeographicTimeSeriesBase* { return new GeographicTimeSeriesMonthly(aStart, aEnd, aHiDb, aLocDb, aPointSize, aPointDensity, aOutlineColor, aOutlineWidth); }, py::arg("start_date"), py::arg("end_date"), py::arg("hidb"), py::arg("locdb"), py::arg("point_size_in_pixels") = 4.0, py::arg("point_density") = 0.8, py::arg("outline_color") = "grey63", py::arg("outline_width") = 0.5);
    m.def("geographic_time_series_yearly", [](std::string aStart, std::string aEnd, const hidb::HiDb& aHiDb, const LocDb& aLocDb, double aPointSize, double aPointDensity, std::string aOutlineColor, double aOutlineWidth) -> GeographicTimeSeriesBase* { return new GeographicTimeSeriesYearly(aStart, aEnd, aHiDb, aLocDb, aPointSize, aPointDensity, aOutlineColor, aOutlineWidth); }, py::arg("start_date"), py::arg("end_date"), py::arg("hidb"), py::arg("locdb"), py::arg("point_size_in_pixels") = 4.0, py::arg("point_density") = 0.8, py::arg("outline_color") = "grey63", py::arg("outline_width") = 0.5);
    m.def("geographic_time_series_weekly", [](std::string aStart, std::string aEnd, const hidb::HiDb& aHiDb, const LocDb& aLocDb, double aPointSize, double aPointDensity, std::string aOutlineColor, double aOutlineWidth) -> GeographicTimeSeriesBase* { return new GeographicTimeSeriesWeekly(aStart, aEnd, aHiDb, aLocDb, aPointSize, aPointDensity, aOutlineColor, aOutlineWidth); }, py::arg("start_date"), py::arg("end_date"), py::arg("hidb"), py::arg("locdb"), py::arg("point_size_in_pixels") = 4.0, py::arg("point_density") = 0.8, py::arg("outline_color") = "grey63", py::arg("outline_width") = 0.5);

      // ----------------------------------------------------------------------
      // Vaccines
      // ----------------------------------------------------------------------

    py::class_<Vaccines>(m, "Vaccines")
            .def(py::init<const Chart&, const hidb::HiDb&>(), py::arg("chart"), py::arg("hidb"))
            .def("report_all", &Vaccines::report_all, py::arg("indent") = 0)
            .def("report", &Vaccines::report, py::arg("indent") = 0)
            .def("match", &Vaccines::match, py::arg("name") = "", py::arg("type") = "", py::arg("passage_type") = "")
            .def("plot", &Vaccines::plot, py::arg("chart_draw"))
            .def("indices", &Vaccines::indices)
            ;

    py::class_<VaccineMatcher>(m, "VaccineMatcher")
            .def("no", &VaccineMatcher::no, py::arg("no"))
            .def("show", &VaccineMatcher::show, py::arg("show"))
            .def("shape", &VaccineMatcher::shape, py::arg("shape"))
            .def("size", &VaccineMatcher::size, py::arg("size"))
            .def("fill", py::overload_cast<std::string>(&VaccineMatcher::fill), py::arg("color"))
            .def("outline", py::overload_cast<std::string>(&VaccineMatcher::outline), py::arg("color"))
            .def("outline_width", &VaccineMatcher::outline_width, py::arg("outline_width"))
            .def("aspect", &VaccineMatcher::aspect, py::arg("aspect"))
            .def("rotation", &VaccineMatcher::rotation, py::arg("rotation"))
            .def("label", &VaccineMatcher::label, py::arg("chart_draw"), py::arg("locdb"))
            ;

    py::class_<VaccineMatcherLabel>(m, "VaccineMatcherLabel")
            .def("display_name", &VaccineMatcherLabel::display_name, py::arg("name"))
            .def("color", py::overload_cast<std::string>(&VaccineMatcherLabel::color), py::arg("color"))
            .def("size", &VaccineMatcherLabel::size, py::arg("size"))
            .def("weight", &VaccineMatcherLabel::weight, py::arg("weight"))
            .def("slant", &VaccineMatcherLabel::slant, py::arg("slant"))
            .def("font_family", &VaccineMatcherLabel::font_family, py::arg("font_family"))
            .def("offset", &VaccineMatcherLabel::offset, py::arg("x"), py::arg("y"))
            .def("offset", [](VaccineMatcherLabel& label, const std::vector<double>& offset) { label.offset(offset[0], offset[1]); }, py::arg("offset"))
            .def("name_type", &VaccineMatcherLabel::name_type, py::arg("name_type"))
            ;

      // ----------------------------------------------------------------------

    m.def("test_time_series", &test_time_series);
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
