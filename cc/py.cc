#include "acmacs-base/pybind11.hh"
#include "locationdb/locdb.hh"
#include "hidb/hidb.hh"
#include "seqdb/seqdb.hh"

#include "acmacs-chart/chart.hh"
#include "acmacs-chart/ace.hh"
#include "vaccines.hh"
#include "draw.hh"

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

// static inline PointStyle* point_style_kw(PointStyle& aStyle, py::args args, py::kwargs kwargs)
// {
//     PointStyle* obj = new (&aStyle) PointStyle(PointStyle::Empty);
//     std::cerr << "point_style_kw" << std::endl;
//     return obj;
// }

// static inline PointStyle* point_style_modify_kw(PointStyle* aStyle, py::args args, py::kwargs kwargs)
// {
//     std::cerr << "point_style_modify_kw" << std::endl;
//     return aStyle;
// }

// static inline PointStyle* new_point_style_kw(py::args args, py::kwargs kwargs)
// {
//     PointStyle* style = new PointStyle(PointStyle::Empty);
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
              // .def("find_hi_name", &seqdb::Seqdb::find_hi_name, py::arg("name"), py::return_value_policy::reference, py::doc("returns entry_seq found by hi name or None"))
            ;

      // ----------------------------------------------------------------------
      // Antigen, Serum
      // ----------------------------------------------------------------------

    py::class_<AntigenSerum>(m, "AntigenSerum")
            .def("full_name", &AntigenSerum::full_name)
            // .def("variant_id", &AntigenSerum::variant_id)
            .def("name", py::overload_cast<>(&AntigenSerum::name, py::const_))
            .def("lineage", py::overload_cast<>(&AntigenSerum::lineage, py::const_))
            .def("passage", py::overload_cast<>(&AntigenSerum::passage, py::const_))
            .def("reassortant", py::overload_cast<>(&AntigenSerum::reassortant, py::const_))
            .def("semantic", py::overload_cast<>(&AntigenSerum::semantic, py::const_))
            .def("annotations", [](const AntigenSerum &as) { py::list list; for (const auto& anno: as.annotations()) { list.append(py::str(anno)); } return list; }, py::doc("returns a copy of the annotation list, modifications to the returned list are not applied"))
            ;

    py::class_<Antigen, AntigenSerum>(m, "Antigen")
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
            .def("continents", [](const Antigens& antigens, const LocDb& aLocDb) { Antigens::ContinentData data; antigens.continents(data, aLocDb); return data; })
            ;

    py::class_<Sera>(m, "Sera")
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
            ;

    py::class_<Chart>(m, "Chart")
            .def("number_of_antigens", &Chart::number_of_antigens)
            .def("number_of_sera", &Chart::number_of_sera)
            .def("antigens", py::overload_cast<>(&Chart::antigens), py::return_value_policy::reference)
            .def("sera", py::overload_cast<>(&Chart::sera), py::return_value_policy::reference)
            .def("antigen", &Chart::antigen, py::arg("no"), py::return_value_policy::reference)
            .def("serum", &Chart::serum, py::arg("no"), py::return_value_policy::reference)
            .def("lineage", &Chart::lineage)
            // .def("table_id", &Chart::table_id)
            // .def("find_homologous_antigen_for_sera", &Chart::find_homologous_antigen_for_sera)
            .def("chart_info", py::overload_cast<>(&Chart::chart_info, py::const_), py::return_value_policy::reference)
        ;

    m.def("import_chart", &import_chart, py::arg("data"), py::doc("Imports chart from a buffer or file in the ace format."));
    m.def("export_chart", &export_chart, py::arg("filename"), py::arg("chart"), py::doc("Exports chart into a file in the ace format."));

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
            .def("report", &Vaccines::report)
            .def("egg", &Vaccines::egg, py::return_value_policy::reference)
            .def("cell", &Vaccines::cell, py::return_value_policy::reference)
            .def("reassortant", &Vaccines::reassortant, py::return_value_policy::reference)
            ;

    m.def("find_vaccines_in_chart", &find_vaccines_in_chart, py::arg("name"), py::arg("chart"), py::arg("hidb"));

      // ----------------------------------------------------------------------
      // Draw
      // ----------------------------------------------------------------------

    py::class_<Color>(m, "Color")
            .def(py::init<std::string>(), py::arg("color") = "black")
            ;

    py::class_<PointStyle> point_style(m, "PointStyle");
    py::enum_<enum PointStyle::Empty>(point_style, "PointStyle_Empty").value("Empty", PointStyle::Empty).export_values();
    point_style
            .def(py::init<enum PointStyle::Empty>(), py::arg("_") = PointStyle::Empty)
              // .def("__init__", &point_style_kw)
              // .def("modify", &point_style_modify_kw)
            .def("show", &PointStyle::show)
            .def("hide", &PointStyle::hide)
            .def("shape", &point_style_shape, py::arg("shape"))
            .def("fill", [](PointStyle& style, std::string color) -> PointStyle& { return style.fill(color); }, py::arg("fill"))
            .def("fill", &PointStyle::fill, py::arg("fill"))
            .def("outline", [](PointStyle& style, std::string color) -> PointStyle& { return style.outline(color); }, py::arg("outline"))
            .def("outline", &PointStyle::outline, py::arg("outline"))
            .def("size", [](PointStyle& style, double aSize) -> PointStyle& { return style.size(Pixels{aSize}); }, py::arg("size"))
            .def("outline_width", &PointStyle::outline_width, py::arg("outline_width"))
            .def("aspect", py::overload_cast<double>(&PointStyle::aspect), py::arg("aspect"))
            .def("rotation", py::overload_cast<double>(&PointStyle::rotation), py::arg("rotation"))
            .def("scale", &PointStyle::scale, py::arg("scale"))
            .def("scale_outline", &PointStyle::scale_outline, py::arg("scale"))
            ;
      // m.def("point_style", &new_point_style_kw);

    py::class_<DrawingOrder>(m, "DrawingOrder")
            .def("raise_", &DrawingOrder::raise, py::arg("point_no"))
            .def("lower", &DrawingOrder::lower, py::arg("point_no"))
            ;

    py::class_<ChartDraw>(m, "ChartDraw")
            .def(py::init<Chart&, size_t>(), py::arg("chart"), py::arg("projection_no") = 0)
            .def("prepare", &ChartDraw::prepare)
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
            .def("background_color", [](ChartDraw& cd, std::string color) { cd.background_color(color); }, py::arg("color"))
            .def("grid", [](ChartDraw& cd, std::string color, double line_width) { cd.grid(color, line_width); }, py::arg("color") = "grey80", py::arg("line_width") = 1.0)
            .def("border", [](ChartDraw& cd, std::string color, double line_width) { cd.border(color, line_width); }, py::arg("color") = "black", py::arg("line_width") = 1.0)
            .def("continent_map", [](ChartDraw& cd, std::vector<double> aOrigin, double aWidth) { cd.continent_map({aOrigin[0], aOrigin[1]}, Pixels{aWidth}); }, py::arg("origin"), py::arg("width"), py::doc("Origin and width are in pixels. Negative values in orinin mean from right/bottom of the surface"))
            ;

      // ----------------------------------------------------------------------

    return m.ptr();
}

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
