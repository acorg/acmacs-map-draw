#include "acmacs-base/pybind11.hh"
#include "locationdb/locdb.hh"
#include "acmacs-chart/chart.hh"
#include "acmacs-chart/ace.hh"
#include "acmacs-chart/lispmds.hh"
#include "hidb/hidb.hh"
#include "hidb/vaccines.hh"
#include "seqdb/seqdb.hh"

#include "draw.hh"
#include "geographic-map.hh"

// ----------------------------------------------------------------------

static inline PointStyle& point_style_shape(PointStyle& aStyle, std::string aShape)
{
    if (aShape == "circle")
        return aStyle.shape(PointStyle::Shape::Circle);
    if (aShape == "box")
        return aStyle.shape(PointStyle::Shape::Box);
    if (aShape == "triangle")
        return aStyle.shape(PointStyle::Shape::Triangle);
    throw std::runtime_error("Unrecognized point style shape: " + aShape);
}

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

PYBIND11_PLUGIN(acmacs_map_draw_backend)
{
    py::module m("acmacs_map_draw_backend", "Acmacs map draw plugin");

      // ----------------------------------------------------------------------
      // HiDb
      // ----------------------------------------------------------------------

    py::register_exception<hidb::HiDb::NotFound>(m, "hidb_NotFound");

    py::class_<hidb::PerTable>(m, "hidb_PerTable")
            .def("table_id", py::overload_cast<>(&hidb::PerTable::table_id, py::const_))
            ;

    py::class_<hidb::AntigenData>(m, "hidb_AntigenSerumData_Antigen")
            .def("number_of_tables", &hidb::AntigenData::number_of_tables)
            .def("most_recent_table", &hidb::AntigenData::most_recent_table)
            .def("date", &hidb::AntigenData::date)
            ;

    py::class_<hidb::SerumData>(m, "hidb_AntigenSerumData_Serum")
            .def("number_of_tables", &hidb::SerumData::number_of_tables)
            .def("most_recent_table", &hidb::SerumData::most_recent_table)
            ;

    py::class_<hidb::HiDb>(m, "HiDb")
            .def("find_homologous_antigens_for_sera_of_chart", &hidb::HiDb::find_homologous_antigens_for_sera_of_chart, py::arg("chart"))
            ;

    py::class_<hidb::HiDbSet>(m, "HiDbSet")
            .def(py::init<std::string>(), py::arg("hidb_dir"))
            .def("get", &hidb::HiDbSet::get, py::arg("virus_type"), py::return_value_policy::reference)
            ;

      // ----------------------------------------------------------------------
      // SeqDb
      // ----------------------------------------------------------------------

    py::class_<seqdb::SeqdbSeq>(m, "SeqdbSeq")
            .def("clades", py::overload_cast<>(&seqdb::SeqdbSeq::clades))
            .def("amino_acid_at", &seqdb::SeqdbSeq::amino_acid_at, py::arg("pos"), py::doc("pos starts from 1"))
            ;

    py::class_<seqdb::SeqdbEntry>(m, "SeqdbEntry")
            ;

    py::class_<seqdb::SeqdbEntrySeq>(m, "SeqdbEntrySeq")
            .def_property_readonly("entry", py::overload_cast<>(&seqdb::SeqdbEntrySeq::entry), py::return_value_policy::reference)
            .def_property_readonly("seq", py::overload_cast<>(&seqdb::SeqdbEntrySeq::seq), py::return_value_policy::reference)
            .def("__bool__", &seqdb::SeqdbEntrySeq::operator bool)
            ;

    py::class_<seqdb::Seqdb>(m, "Seqdb")
            .def(py::init<>())
            .def("load", &seqdb::Seqdb::load, py::arg("filename") = std::string(), py::doc("reads seqdb from file containing json"))
            .def("build_hi_name_index", &seqdb::Seqdb::build_hi_name_index)
            .def("match_antigens", [](const seqdb::Seqdb& aSeqdb, const Antigens& aAntigens, bool aVerbose) { std::vector<seqdb::SeqdbEntrySeq> r; aSeqdb.match(aAntigens, r, aVerbose); return r; }, py::arg("antigens"), py::arg("verbose"))
            .def("aa_at_positions_for_antigens", [](const seqdb::Seqdb& aSeqdb, const Antigens& aAntigens, const std::vector<size_t>& aPositions, bool aVerbose) {
                    std::map<std::string, std::vector<size_t>> r; aSeqdb.aa_at_positions_for_antigens(aAntigens, aPositions, r, aVerbose); return r; }, py::arg("antigens"), py::arg("positions"), py::arg("verbose"))
              // .def("find_hi_name", &seqdb::Seqdb::find_hi_name, py::arg("name"), py::return_value_policy::reference, py::doc("returns entry_seq found by hi name or None"))
            ;

      // ----------------------------------------------------------------------
      // Antigen, Serum
      // ----------------------------------------------------------------------

    py::class_<AntigenSerum>(m, "AntigenSerum")
            .def("full_name", &AntigenSerum::full_name)
            .def("abbreviated_name", &AntigenSerum::abbreviated_name, py::arg("locdb"), py::doc("includes passage, reassortant, annotations"))
            .def("name_abbreviated", &AntigenSerum::name_abbreviated, py::arg("locdb"), py::doc("just name without passage, reassortant, annotations"))
            .def("location_abbreviated", &AntigenSerum::location_abbreviated, py::arg("locdb"))
            .def("name", py::overload_cast<>(&AntigenSerum::name, py::const_))
            .def("lineage", py::overload_cast<>(&AntigenSerum::lineage, py::const_))
            .def("passage", py::overload_cast<>(&AntigenSerum::passage, py::const_))
            .def("reassortant", py::overload_cast<>(&AntigenSerum::reassortant, py::const_))
            .def("semantic", py::overload_cast<>(&AntigenSerum::semantic, py::const_))
            .def("annotations", [](const AntigenSerum &as) { py::list list; for (const auto& anno: as.annotations()) { list.append(py::str(anno)); } return list; }, py::doc("returns a copy of the annotation list, modifications to the returned list are not applied"))
            ;

    py::class_<Antigen, AntigenSerum>(m, "Antigen")
            .def("abbreviated_name_with_passage_type", &Antigen::abbreviated_name_with_passage_type, py::arg("locdb"))
            .def("date", py::overload_cast<>(&Antigen::date, py::const_))
            .def("lab_id", [](const Antigen &a) { py::list list; for (const auto& li: a.lab_id()) { list.append(py::str(li)); } return list; }, py::doc("returns a copy of the lab_id list, modifications to the returned list are not applied"))
            ;

    py::class_<Serum, AntigenSerum>(m, "Serum")
            .def("serum_id", py::overload_cast<>(&Serum::serum_id, py::const_))
            .def("serum_species", py::overload_cast<>(&Serum::serum_species, py::const_))
            .def("homologous", py::overload_cast<>(&Serum::homologous, py::const_))
            ;

    py::class_<LocDb>(m, "LocDb")
            .def(py::init<>())
            .def("import_from", &LocDb::importFrom, py::arg("filename"))
            ;

    py::class_<Antigens>(m, "Antigens")
            .def("__getitem__", [](Antigens& antigens, size_t index) -> Antigen& { return antigens[index]; }, py::return_value_policy::reference)
            .def("continents", [](const Antigens& antigens, const LocDb& aLocDb) { Antigens::ContinentData data; antigens.continents(data, aLocDb); return data; })
            .def("countries", [](const Antigens& antigens, const LocDb& aLocDb) { Antigens::CountryData data; antigens.countries(data, aLocDb); return data; })
            .def("country", [](const Antigens& antigens, std::string aCountry, const LocDb& aLocDb) { std::vector<size_t> indices; antigens.country(aCountry, indices, aLocDb); return indices; })
            .def("find_by_name_matching", [](const Antigens& antigens, std::string aName) { std::vector<size_t> indices; antigens.find_by_name_matching(aName, indices); return indices; })
            .def("reference_indices", [](const Antigens& antigens) { std::vector<size_t> indices; antigens.reference_indices(indices); return indices; })
            .def("test_indices", [](const Antigens& antigens) { std::vector<size_t> indices; antigens.test_indices(indices); return indices; })
            .def("date_range_indices", [](const Antigens& antigens, std::string first_date, std::string after_last_date) { std::vector<size_t> indices; antigens.date_range_indices(first_date, after_last_date, indices); return indices; }, py::arg("first") = std::string(), py::arg("after_last") = std::string())
            .def("egg_indices", [](const Antigens& antigens) { std::vector<size_t> indices; antigens.egg_indices(indices); return indices; })
            .def("cell_indices", [](const Antigens& antigens) { std::vector<size_t> indices; antigens.cell_indices(indices); return indices; })
            .def("reassortant_indices", [](const Antigens& antigens) { std::vector<size_t> indices; antigens.reassortant_indices(indices); return indices; })
            ;

    py::class_<Sera>(m, "Sera")
            .def("__getitem__", [](Sera& sera, size_t index) -> Serum& { return sera[index]; }, py::return_value_policy::reference)
            .def("find_by_name_matching", [](const Sera& sera, std::string aName) { std::vector<size_t> indices; sera.find_by_name_matching(aName, indices); return indices; })
            ;

      // ----------------------------------------------------------------------
      // Chart
      // ----------------------------------------------------------------------

    py::class_<ChartInfo>(m, "ChartInfo")
            .def("virus", py::overload_cast<>(&ChartInfo::virus, py::const_))
            .def("virus_type", py::overload_cast<>(&ChartInfo::virus_type, py::const_))
            .def("assay", py::overload_cast<>(&ChartInfo::assay, py::const_))
            .def("date", py::overload_cast<>(&ChartInfo::date, py::const_))
            .def("lab", py::overload_cast<>(&ChartInfo::lab, py::const_))
            .def("rbc", py::overload_cast<>(&ChartInfo::rbc, py::const_))
            .def("name", py::overload_cast<>(&ChartInfo::name, py::const_))
            .def("subset", py::overload_cast<>(&ChartInfo::subset, py::const_))
            .def("type", &ChartInfo::type_as_string)
            .def("make_name", &ChartInfo::make_name)
            ;

    py::class_<Chart>(m, "Chart")
            .def("number_of_antigens", &Chart::number_of_antigens)
            .def("number_of_sera", &Chart::number_of_sera)
            .def("antigens", py::overload_cast<>(&Chart::antigens), py::return_value_policy::reference)
            .def("sera", py::overload_cast<>(&Chart::sera), py::return_value_policy::reference)
            .def("antigen", &Chart::antigen, py::arg("no"), py::return_value_policy::reference)
            .def("serum", &Chart::serum, py::arg("no"), py::return_value_policy::reference)
            .def("lineage", &Chart::lineage)
            .def("make_name", &Chart::make_name)
            .def("chart_info", py::overload_cast<>(&Chart::chart_info, py::const_), py::return_value_policy::reference)
            .def("serum_circle_radius", &Chart::serum_circle_radius, py::arg("antigen_no"), py::arg("serum_no"), py::arg("projection_no") = 0, py::arg("verbose") = false)
            .def("serum_coverage", [](const Chart& aChart, size_t aAntigenNo, size_t aSerumNo) -> std::vector<std::vector<size_t>> { std::vector<size_t> within, outside; aChart.serum_coverage(aAntigenNo, aSerumNo, within, outside); return {within, outside}; } , py::arg("antigen_no"), py::arg("serum_no"))
        ;

    py::class_<Transformation>(m, "Transformation")
            ;

    m.def("import_chart", &import_chart, py::arg("data"), py::doc("Imports chart from a buffer or file in the ace format."));
    m.def("export_chart", &export_chart, py::arg("filename"), py::arg("chart"), py::doc("Exports chart into a file in the ace format."));
    m.def("export_chart_lispmds", py::overload_cast<std::string, const Chart&, const std::vector<PointStyle>&, const Transformation&>(&export_chart_lispmds), py::arg("filename"), py::arg("chart"), py::arg("point_styles"), py::arg("transformation"), py::doc("Exports chart into a file in the lispmds save format."));

      // ----------------------------------------------------------------------
      // Vaccines (hidb)
      // ----------------------------------------------------------------------

    py::class_<Vaccines::HomologousSerum>(m, "Vaccines_HomologousSerum")
            .def_readonly("serum", &Vaccines::HomologousSerum::serum)
            .def_readonly("serum_index", &Vaccines::HomologousSerum::serum_index)
            .def_readonly("most_recent_table", &Vaccines::HomologousSerum::most_recent_table_date)
            .def("number_of_tables", &Vaccines::HomologousSerum::number_of_tables)
            ;

    py::class_<Vaccines::Entry>(m, "Vaccines_Entry")
            .def_readonly("antigen", &Vaccines::Entry::antigen)
            .def_readonly("antigen_index", &Vaccines::Entry::antigen_index)
            .def_readonly("homologous_sera", &Vaccines::Entry::homologous_sera, py::return_value_policy::reference)
            ;

    py::class_<Vaccines>(m, "Vaccines")
            .def("report", &Vaccines::report, py::arg("indent") = 0)
            .def("egg", &Vaccines::egg, py::arg("no") = 0, py::return_value_policy::reference)
            .def("cell", &Vaccines::cell, py::arg("no") = 0, py::return_value_policy::reference)
            .def("reassortant", &Vaccines::reassortant, py::arg("no") = 0, py::return_value_policy::reference)
            .def("number_of_eggs", &Vaccines::number_of_eggs)
            .def("number_of_cells", &Vaccines::number_of_cells)
            .def("number_of_reassortants", &Vaccines::number_of_reassortants)
            ;

    m.def("find_vaccines_in_chart", &find_vaccines_in_chart, py::arg("name"), py::arg("chart"), py::arg("hidb"));

      // ----------------------------------------------------------------------
      // Draw
      // ----------------------------------------------------------------------

    py::class_<Color>(m, "Color")
            .def(py::init<std::string>(), py::arg("color") = "black")
            .def("__str__", &Color::to_string)
            .def("to_string", &Color::to_string)
            .def("to_hex_string", &Color::to_string)
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
            .def("shape", &point_style_shape, py::arg("shape"))
            .def("shape", &get_point_style_shape)
            .def("fill", [](PointStyle& style, std::string color) -> PointStyle& { return style.fill(color); }, py::arg("fill"))
            .def("fill", py::overload_cast<>(&PointStyle::fill, py::const_))
            .def("outline", [](PointStyle& style, std::string color) -> PointStyle& { return style.outline(color); }, py::arg("outline"))
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
            .def("label_size", &LegendPointLabel::label_size, py::arg("label_size"))
            .def("point_size", &LegendPointLabel::point_size, py::arg("point_size"))
            .def("background", [](LegendPointLabel& legend, std::string aBackground) { legend.background(aBackground); }, py::arg("background"))
            .def("border_color", [](LegendPointLabel& legend, std::string aBorderColor) { legend.border_color(aBorderColor); }, py::arg("border_color"))
            .def("border_width", &LegendPointLabel::border_width, py::arg("border_width"))
            ;

    py::class_<Title>(m, "Title")
            .def("add_line", &Title::add_line, py::arg("text"))
            .def("text_size", &Title::text_size, py::arg("text_size"))
            .def("background", [](Title& legend, std::string aBackground) { legend.background(aBackground); }, py::arg("background"))
            .def("border_color", [](Title& legend, std::string aBorderColor) { legend.border_color(aBorderColor); }, py::arg("border_color"))
            .def("border_width", &Title::border_width, py::arg("border_width"))
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

    py::class_<ChartDraw>(m, "ChartDraw")
            .def(py::init<Chart&, size_t>(), py::arg("chart"), py::arg("projection_no") = 0)
            .def("prepare", &ChartDraw::prepare)
            .def("point_styles", &ChartDraw::point_styles)
            .def("point_styles_base", &ChartDraw::point_styles_base)
            .def("draw", py::overload_cast<std::string, double>(&ChartDraw::draw), py::arg("filename"), py::arg("size"))
            .def("mark_egg_antigens", &ChartDraw::mark_egg_antigens)
            .def("mark_reassortant_antigens", &ChartDraw::mark_reassortant_antigens)
            .def("all_grey", &ChartDraw::mark_all_grey, py::arg("color") = Color("grey80"))
            .def("scale_points", &ChartDraw::scale_points, py::arg("scale"), py::arg("outline_scale") = 1.0, py::doc("outline_scale=0 means use point scale for outline too"))
            .def("modify_point_by_index", &ChartDraw::modify_point_by_index, py::arg("index"), py::arg("style"), py::arg("raise_") = false, py::arg("lower") = false)
            .def("modify_points_by_indices", &ChartDraw::modify_points_by_indices, py::arg("indices"), py::arg("style"), py::arg("raise_") = false, py::arg("lower") = false)
            .def("drawing_order", &ChartDraw::drawing_order, py::return_value_policy::reference)
            .def("rotate", &ChartDraw::rotate, py::arg("angle"))
            .def("flip", &ChartDraw::flip, py::arg("x"), py::arg("y"))
            .def("flip_ns", [](ChartDraw& cd) { cd.flip(1, 0); })
            .def("flip_ew", [](ChartDraw& cd) { cd.flip(0, 1); })
            .def("transformation", &ChartDraw::transformation, py::return_value_policy::reference)
            .def("viewport", &ChartDraw::viewport, py::arg("x"), py::arg("y"), py::arg("size"))
            .def("viewport", [](ChartDraw& cd, const std::vector<double>& a) { cd.viewport(a[0], a[1], a[2]); }, py::arg("viewport"))
            .def("background_color", [](ChartDraw& cd, std::string color) { cd.background_color(color); }, py::arg("color"))
            .def("grid", [](ChartDraw& cd, std::string color, double line_width) { cd.grid(color, line_width); }, py::arg("color") = "grey80", py::arg("line_width") = 1.0)
            .def("border", [](ChartDraw& cd, std::string color, double line_width) { cd.border(color, line_width); }, py::arg("color") = "black", py::arg("line_width") = 1.0)
            .def("continent_map", [](ChartDraw& cd, std::vector<double> aOrigin, double aWidth) { cd.continent_map({aOrigin[0], aOrigin[1]}, Pixels{aWidth}); }, py::arg("origin"), py::arg("width"), py::doc("Origin and width are in pixels. Negative values in orinin mean from right/bottom of the surface"))
            .def("legend", [](ChartDraw& cd, std::vector<double> aOrigin) -> LegendPointLabel& { return cd.legend({aOrigin[0], aOrigin[1]}); }, py::arg("origin"), py::return_value_policy::reference, py::doc("Origin is in pixels. Negative values in orinin mean from right/bottom of the surface"))
            .def("legend", [](ChartDraw& cd) -> LegendPointLabel& { return cd.legend(); }, py::return_value_policy::reference)
            .def("title", [](ChartDraw& cd, std::vector<double> aOrigin) -> Title& { return cd.title({aOrigin[0], aOrigin[1]}); }, py::arg("origin"), py::return_value_policy::reference, py::doc("Origin is in pixels. Negative values in orinin mean from right/bottom of the surface"))
            .def("title", [](ChartDraw& cd) -> Title& { return cd.title(); }, py::return_value_policy::reference)
            .def("label", py::overload_cast<size_t>(&ChartDraw::add_label), py::arg("index"), py::return_value_policy::reference)
            .def("serum_circle", [](ChartDraw& cd, size_t aSerumNo, double aRadius) -> SerumCircle& { return cd.serum_circle(aSerumNo, Scaled(aRadius)); }, py::arg("serum_no"), py::arg("radius"), py::return_value_policy::reference)
            ;

      // ----------------------------------------------------------------------

    py::class_<GeographicMapDraw>(m, "GeographicMapDraw")
            .def(py::init<>())
            .def("draw", static_cast<void (GeographicMapDraw::*)(std::string)>(&GeographicMapDraw::draw), py::arg("filename"))
            ;

    py::class_<GeographicMapWithPointsFromHidb, GeographicMapDraw>(m, "GeographicMapWithPointsFromHidb")
            .def(py::init<const hidb::HiDb&, const LocDb&>(), py::arg("hidb"), py::arg("locdb"))
            .def("add_points_from_hidb", &GeographicMapWithPointsFromHidb::add_points_from_hidb, py::arg("start_date"), py::arg("end_date"))
            ;

      // ----------------------------------------------------------------------

    return m.ptr();
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
