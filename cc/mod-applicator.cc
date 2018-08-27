#include <string>
using namespace std::string_literals;

#include "acmacs-chart-2/serum-line.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/mod-applicator.hh"
#include "acmacs-map-draw/mod-serum.hh"
#include "acmacs-map-draw/mod-procrustes.hh"
#include "acmacs-map-draw/mod-amino-acids.hh"
#include "acmacs-map-draw/select.hh"
#include "acmacs-map-draw/point-style-draw.hh"

// ----------------------------------------------------------------------

using Mods = std::vector<std::unique_ptr<Mod>>;

inline std::ostream& operator << (std::ostream& out, const Mods& aMods)
{
    out << "[\n";
    for (const auto& mod: aMods)
        out << "  " << *mod << '\n';
    out << ']';
    return out;
}

// ----------------------------------------------------------------------

void Mod::add_label(ChartDraw& aChartDraw, size_t aIndex, size_t aBaseIndex, const rjson::value& aLabelData)
{
    if (aChartDraw.point_has_coordinates(aIndex + aBaseIndex)) {
        auto& label = aChartDraw.add_label(aIndex + aBaseIndex);
        try { label.color(Color(aLabelData["color"])); } catch (rjson::field_not_found&) {}
        try { label.size(aLabelData["size"]); } catch (rjson::field_not_found&) {}
        try { label.weight(aLabelData["weight"].str()); } catch (rjson::field_not_found&) {}
        try { label.slant(aLabelData["slant"].str()); } catch (rjson::field_not_found&) {}
        try { label.font_family(aLabelData["font_family"].str()); } catch (rjson::field_not_found&) {}

        try {
            const rjson::array& offset = aLabelData["offset"];
            label.offset({offset[0], offset[1]});
        }
        catch (std::exception&) {
        }

        try {
            label.display_name(aLabelData["display_name"].strv());
        }
        catch (rjson::field_not_found&) {
            try {
                const auto name_type = aLabelData["name_type"].strv();
                if (aBaseIndex == 0) { // antigen
                    auto antigen = aChartDraw.chart().antigen(aIndex);
                    std::string name;
                    if (name_type == "abbreviated")
                        name = antigen->abbreviated_name();
                    else if (name_type == "abbreviated_name_with_passage_type" || name_type == "abbreviated_with_passage_type")
                        name = antigen->abbreviated_name_with_passage_type();
                    else if (name_type == "abbreviated_location_with_passage_type")
                        name = antigen->abbreviated_location_with_passage_type();
                    else {
                        if (name_type != "full")
                            std::cerr << "WARNING: unrecognized \"name_type\" for label for antigen " << aIndex << '\n';
                        name = antigen->full_name();
                    }
                    label.display_name(name);
                }
                else {      // serum
                    auto serum = aChartDraw.chart().serum(aIndex);
                    if (name_type == "abbreviated")
                        label.display_name(serum->abbreviated_name());
                    else if (name_type == "abbreviated_name_with_serum_id")
                        label.display_name(serum->abbreviated_name_with_serum_id());
                    else {
                        if (name_type != "full")
                            std::cerr << "WARNING: unrecognized \"name_type\" for label for serum " << aIndex << '\n';
                        label.display_name(serum->full_name());
                    }
                }
            }
            catch (rjson::field_not_found&) {
            }
        }
    }

} // Mod::add_label

// ----------------------------------------------------------------------

void Mod::add_labels(ChartDraw& aChartDraw, const acmacs::chart::PointIndexList& aIndices, size_t aBaseIndex, const rjson::value& aLabelData)
{
    if (aLabelData.get_or_default("show", true)) {
        for (auto index: aIndices)
            add_label(aChartDraw, index, aBaseIndex, aLabelData);
    }
    else {
        for (auto index: aIndices)
            aChartDraw.remove_label(index);
    }

} // Mod::add_labels

// ----------------------------------------------------------------------

void Mod::add_legend(ChartDraw& aChartDraw, const acmacs::chart::PointIndexList& aIndices, const acmacs::PointStyle& aStyle, const rjson::value& aLegendData)
{
    const std::string label = aLegendData.get_or_default("label", "use \"label\" in \"legend\"");
    if (aLegendData.get_or_default("replace", false)) {
        // std::cerr << "DEBUG: add_legend replace " << label << '\n';
        // std::cerr << "DEBUG: " << aChartDraw.legend_point_label().lines() << '\n';
        if (aLegendData.get_or_default("count", false))
            aChartDraw.legend_point_label().remove_line(label + " (" + std::to_string(aIndices.size()) + ")");
        else
            aChartDraw.legend_point_label().remove_line(label);
    }
    add_legend(aChartDraw, aIndices, aStyle, label, aLegendData);

} // Mod::add_legend

// ----------------------------------------------------------------------

void Mod::add_legend(ChartDraw& aChartDraw, const acmacs::chart::PointIndexList& aIndices, const acmacs::PointStyle& aStyle, std::string aLabel, const rjson::value& aLegendData)
{
    if (aLegendData.get_or_default("show", true) && !aIndices.empty()) {
        if (aLegendData.get_or_default("type", "") == "continent_map") {
            const auto& offset = aLegendData.get_or_empty_array("offset");
            if (offset.size() != 2)
                aChartDraw.continent_map();
            else
                aChartDraw.continent_map({offset[0], offset[1]}, Pixels{aLegendData.get_or_default("size", 100.0)});
        }
        else {
            auto& legend = aChartDraw.legend_point_label();
            if (aLegendData.get_or_default("count", false))
                aLabel += " (" + std::to_string(aIndices.size()) + ")";
            legend.add_line(*aStyle.outline, *aStyle.fill, aLabel);
        }
    }

} // Mod::add_legend

// ----------------------------------------------------------------------

// #pragma GCC diagnostic push
// #ifdef __clang__
// #pragma GCC diagnostic ignored "-Wexit-time-destructors"
// #pragma GCC diagnostic ignored "-Wglobal-constructors"
// #endif
// static const rjson::object factory_empty_args;
// #pragma GCC diagnostic pop

static Mods factory(const rjson::value& aMod, const rjson::object& aSettingsMods, const rjson::object& aUpdate);

// ----------------------------------------------------------------------

void apply_mods(ChartDraw& aChartDraw, const rjson::array& aMods, const rjson::object& aModData, bool aIgnoreUnrecognized)
{
    const auto& mods_data_mod = aModData.get_or_empty_object("mods");
    for (const auto& mod_desc: aMods) {
        Timeit ti{"INFO: Applying " + mod_desc.to_json() + ": "};
        try {
            for (const auto& mod: factory(mod_desc, mods_data_mod, {})) {
                mod->apply(aChartDraw, aModData);
            }
        }
        catch (std::bad_variant_access&) {
            std::cerr << "ERROR: std::bad_variant_access: in handling mod: " << mod_desc << '\n';
            throw unrecognized_mod{mod_desc.str()};
        }
        catch (unrecognized_mod&) {
            if (aIgnoreUnrecognized)
                std::cerr << "WARNING: unrecognized mod: " << mod_desc << '\n';
            else
                throw;
        }
    }

} // apply_mods

// ----------------------------------------------------------------------

void ModAntigens::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = args().get_or_default("report", false);
    const auto report_names_threshold = args().get_or_default("report_names_threshold", 10U);
    try {
        const auto indices = SelectAntigens(verbose, report_names_threshold).select(aChartDraw, args()["select"]);
        const auto styl = style();
          // if (verbose)
          //     std::cerr << "DEBUG ModAntigens " << indices << ' ' << args() << ' ' << styl << '\n';
        aChartDraw.modify(indices, styl, drawing_order());
        try { add_labels(aChartDraw, indices, 0, args()["label"]); } catch (rjson::field_not_found&) {}
        try { add_legend(aChartDraw, indices, styl, args()["legend"]); } catch (rjson::field_not_found&) {}
    }
    catch (rjson::field_not_found&) {
        throw unrecognized_mod{"no \"select\" in \"antigens\" mod: " + args().to_json() };
    }

} // ModAntigens::apply

// ----------------------------------------------------------------------

acmacs::Coordinates ModMoveBase::get_move_to(ChartDraw& aChartDraw, bool aVerbose) const
{
    acmacs::Coordinates move_to;
    if (auto [to_present, to] = args().get_array_if("to"); to_present) {
        move_to = acmacs::Coordinates{to[0], to[1]};
    }
    else if (auto [to_antigen_present, to_antigen] = args().get_object_if("to_antigen"); to_antigen_present) {
        const auto antigens = SelectAntigens(aVerbose).select(aChartDraw, to_antigen);
        if (antigens.size() != 1)
            throw unrecognized_mod{"\"to_antigen\" does not select single antigen, mod: " + args().to_json()};
        move_to = aChartDraw.layout()->get(antigens.front());
    }
    else if (auto [to_serum_present, to_serum] = args().get_object_if("to_serum"); to_serum_present) {
        const auto sera = SelectSera(aVerbose).select(aChartDraw, to_serum);
        if (sera.size() != 1)
            throw unrecognized_mod{"\"to_serum\" does not select single serum, mod: " + args().to_json()};
        move_to = aChartDraw.layout()->get(sera.front() + aChartDraw.number_of_antigens());
    }
    else
        throw unrecognized_mod{"neither of \"to\", \"to_antigen\", \"to__serum\" provided in mod: " + args().to_json()};
    return move_to;

} // ModMoveBase::get_move_to

// ----------------------------------------------------------------------

void ModMoveAntigens::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = args().get_or_default("report", false);
    try {
        auto& projection = aChartDraw.projection();
        if (auto flip_scale = args().get_or_default("flip_over_serum_line", std::numeric_limits<double>::max()); flip_scale < (std::numeric_limits<double>::max() / 2)) {
            const acmacs::chart::SerumLine serum_line(projection);
            auto layout = aChartDraw.layout();
            for (auto index : SelectAntigens(verbose).select(aChartDraw, args()["select"])) {
                const auto flipped = serum_line.line().flip_over(layout->get(index), flip_scale);
                projection.move_point(index, flipped);
            }
        }
        else {
            const auto move_to = get_move_to(aChartDraw, verbose);
            for (auto index : SelectAntigens(verbose).select(aChartDraw, args()["select"])) {
                projection.move_point(index, move_to);
            }
        }
    }
    catch (rjson::field_not_found&) {
        throw unrecognized_mod{"no \"select\" in \"move_antigens\" mod: " + args().to_json()};
    }

} // ModMoveAntigens::apply

// ----------------------------------------------------------------------

void ModMoveSera::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = args().get_or_default("report", false);
    try {
        const auto move_to = get_move_to(aChartDraw, verbose);
        auto& projection = aChartDraw.projection();
        for (auto index: SelectSera(verbose).select(aChartDraw, args()["select"])) {
            projection.move_point(index + aChartDraw.number_of_antigens(), move_to);
        }
    }
    catch (rjson::field_not_found&) {
        throw unrecognized_mod{"no \"select\" in \"move_sera\" mod: " + args().to_json() };
    }

} // ModMoveSera::apply

// ----------------------------------------------------------------------

class ModRotate : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            try {
                if (auto [present, degrees_v] = args().get_value_if("degrees"); present) {
                    const double pi_180 = std::acos(-1) / 180.0;
                    aChartDraw.rotate(static_cast<double>(degrees_v) * pi_180);
                }
                else if (auto [present, radians_v] = args().get_value_if("radians"); present) {
                    aChartDraw.rotate(radians_v);
                }
                else {
                    throw rjson::field_not_found{};
                }
            }
            catch (rjson::field_not_found&) {
                throw unrecognized_mod{"mod: " + args().to_json()};
            }
        }

}; // class ModRotate

// ----------------------------------------------------------------------

class ModFlip : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            try {
                if (auto [present, direction_v] = args().get_value_if("direction"); present) {
                    const auto direction = direction_v.strv();
                    if (direction == "ew")
                        aChartDraw.flip(0, 1);
                    else if (direction == "ns")
                        aChartDraw.flip(1, 0);
                    else
                        throw rjson::field_not_found{};
                }
                else {
                    throw rjson::field_not_found{};
                }
            }
            catch (rjson::field_not_found&) {
                throw unrecognized_mod{"mod: " + args().to_json()};
            }
        }

}; // class ModFlip

// ----------------------------------------------------------------------

class ModViewport : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            try {
                if (auto [present_abs, abs] = args().get_array_if("abs"); present_abs) {
                    if (abs.size() != 3)
                        throw unrecognized_mod{"\"abs\" must be array of 3 floats. mod: " + args().to_json()};
                    aChartDraw.viewport({abs[0], abs[1]}, abs[2]);
                }
                else if (auto [present_rel, rel] = args().get_array_if("rel"); present_rel) {
                    if (rel.size() != 3)
                        throw unrecognized_mod{"\"rel\" must be array of 3 floats. mod: " + args().to_json()};
                    aChartDraw.calculate_viewport(false);
                    const auto& orig_viewport = aChartDraw.viewport();
                    const auto new_size = static_cast<double>(rel[2]) + orig_viewport.size.width;
                    if (new_size < 1)
                        throw unrecognized_mod{"invalid size difference in \"rel\". mod: " + args().to_json()};
                    aChartDraw.viewport(orig_viewport.origin + acmacs::Location2D{static_cast<double>(rel[0]), static_cast<double>(rel[1])}, new_size);
                }
                else {
                    throw rjson::field_not_found{};
                }
            }
            catch (rjson::field_not_found&) {
                throw unrecognized_mod{"mod: " + args().to_json()};
            }
        }

}; // class ModViewport

// ----------------------------------------------------------------------

class ModBackground : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            try {
                aChartDraw.background_color(Color(args()["color"]));
            }
            catch (rjson::field_not_found&) {
                throw unrecognized_mod{"mod: " + args().to_json()};
            }
        }

}; // class ModBackground

// ----------------------------------------------------------------------

class ModBorder : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            try {
                aChartDraw.border(Color(args().get_or_default("color", "black")), args().get_or_default("line_width", 1.0));
            }
            catch (rjson::field_not_found&) {
                throw unrecognized_mod{"mod: " + args().to_json()};
            }
        }

}; // class ModBorder

// ----------------------------------------------------------------------

class ModGrid : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            try {
                aChartDraw.grid(Color(args().get_or_default("color", "grey80")), args().get_or_default("line_width", 1.0));
            }
            catch (rjson::field_not_found&) {
                throw unrecognized_mod{"mod: " + args().to_json()};
            }
        }

}; // class ModGrid

// ----------------------------------------------------------------------

class ModPointScale : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            try {
                aChartDraw.scale_points(args().get_or_default("scale", 1.0), args().get_or_default("outline_scale", 1.0));
            }
            catch (rjson::field_not_found&) {
                throw unrecognized_mod{"mod: " + args().to_json()};
            }
        }

}; // class ModPointScale

// ----------------------------------------------------------------------

class ModTitle : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            try {
                auto& title = aChartDraw.title();
                if (args().get_or_default("show", true)) {
                    title.show(true);
                    if (auto [display_name_present, display_name] = args().get_array_if("display_name"); display_name_present) {
                        for (auto& line: display_name)
                            title.add_line(line.str());
                    }
                    else {
                        title.add_line(aChartDraw.chart().make_name(aChartDraw.projection_no()));
                    }
                    if (auto [offset_present, offset] = args().get_array_if("offset"); offset_present)
                        title.offset({offset[0], offset[1]});
                    if (auto [padding_present, padding] = args().get_value_if("padding"); padding_present)
                        title.padding(padding);
                    if (auto [text_size_present, text_size] = args().get_value_if("text_size"); text_size_present)
                        title.text_size(text_size);
                    if (auto [text_color_present, text_color] = args().get_value_if("text_color"); text_color_present)
                        title.text_color(Color(text_color));
                    if (auto [background_present, background] = args().get_value_if("background"); background_present)
                        title.background(Color(background));
                    if (auto [border_color_present, border_color] = args().get_value_if("border_color"); border_color_present)
                        title.border_color(Color(border_color));
                    if (auto [border_width_present, border_width] = args().get_value_if("border_width"); border_width_present)
                        title.border_width(border_width);
                    if (auto [font_weight_present, font_weight] = args().get_value_if("font_weight"); font_weight_present)
                        title.weight(font_weight.str());
                    if (auto [font_slant_present, font_slant] = args().get_value_if("font_slant"); font_slant_present)
                        title.slant(font_slant.str());
                    if (auto [font_family_present, font_family] = args().get_value_if("font_family"); font_family_present)
                        title.font_family(font_family.str());
                }
                else {
                    title.show(false);
                }
            }
            catch (rjson::field_not_found&) {
                throw unrecognized_mod{"mod: " + args().to_json()};
            }
        }

}; // class ModTitle

// ----------------------------------------------------------------------

class ModLegend : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            try {
                if (args().get_or_default("show", true)) {
                    auto& legend = aChartDraw.legend_point_label();
                    if (auto [data_present, data] = args().get_array_if("data"); data_present) {
                        for (const rjson::object& line_data: data)
                            legend.add_line(Color(line_data.get_or_default("outline", "black")), Color(line_data.get_or_default("fill", "pink")), line_data.get_or_default("display_name", "* no display_name *"));
                    }
                    if (auto [offset_present, offset] = args().get_array_if("offset"); offset_present)
                        legend.offset({offset[0], offset[1]});
                    if (auto [label_size_present, label_size] = args().get_value_if("label_size"); label_size_present)
                        legend.label_size(label_size);
                    if (auto [point_size_present, point_size] = args().get_value_if("point_size"); point_size_present)
                        legend.point_size(point_size);
                    if (auto [background_present, background] = args().get_value_if("background"); background_present)
                        legend.background(Color(background));
                    if (auto [border_color_present, border_color] = args().get_value_if("border_color"); border_color_present)
                        legend.border_color(Color(border_color));
                    if (auto [border_width_present, border_width] = args().get_value_if("border_width"); border_width_present)
                        legend.border_width(border_width);
                }
                else {
                    aChartDraw.remove_legend();
                }
            }
            catch (rjson::field_not_found&) {
                throw unrecognized_mod{"mod: " + args().to_json()};
            }
        }

}; // class ModLegend

// ----------------------------------------------------------------------

class ModLine : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            const auto begins = begins_ends(aChartDraw, "from");
            const auto ends = begins_ends(aChartDraw, "to");
            for(const auto& begin: begins) {
                for(const auto& end: ends) {
                    auto& line = aChartDraw.line(begin, end);
                    line.color(Color(args().get_or_default("color", "black")));
                    line.line_width(args().get_or_default("width", 1.0));
                }
            }
        }

 protected:
    std::vector<acmacs::Location2D> begins_ends(ChartDraw& aChartDraw, std::string aPrefix) const
        {
            std::vector<acmacs::Location2D> result;
            const auto verbose = args().get_or_default("report", false);
            if (auto [from_present, from] = args().get_array_if(aPrefix); from_present) {
                result.push_back({from[0], from[1]});
            }
            else if (auto [from_antigen_present, from_antigen] = args().get_object_if(aPrefix + "_antigen"); from_antigen_present) {
                auto layout = aChartDraw.layout();
                for (auto index: SelectAntigens(verbose).select(aChartDraw, from_antigen)) {
                    const auto coord = layout->get(index);
                    result.push_back({coord[0], coord[1]});
                }
            }
            else if (auto [from_serum_present, from_serum] = args().get_object_if(aPrefix + "_serum"); from_serum_present) {
                auto layout = aChartDraw.layout();
                for (auto index: SelectSera(verbose).select(aChartDraw, from_serum)) {
                    const auto coord = layout->get(index + aChartDraw.number_of_antigens());
                    result.push_back({coord[0], coord[1]});
                }
            }
            else
                throw unrecognized_mod{"neither \"" + aPrefix + "\" nor \"" + aPrefix + "_antigen\" nor \"" + aPrefix + "_serum\" provided in mod: " + args().to_json()};
            return result;
        }

}; // class ModLine

// ----------------------------------------------------------------------

class ModArrow : public ModLine
{
 public:
    using ModLine::ModLine;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            const auto begins = begins_ends(aChartDraw, "from");
            const auto ends = begins_ends(aChartDraw, "to");
            for(const auto& begin: begins) {
                for(const auto& end: ends) {
                    auto& arrow = aChartDraw.arrow(begin, end);
                    const auto color(args().get_or_default("color", "black"));
                    arrow.color(Color(color), Color(args().get_or_default("head_color", color)));
                    arrow.line_width(args().get_or_default("width", 1.0));
                    arrow.arrow_head_filled(args().get_or_default("head_filled", true));
                    arrow.arrow_width(args().get_or_default("arrow_width", 5.0));
                }
            }
        }

}; // class ModArrow

// ----------------------------------------------------------------------

class ModRectangle : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            const rjson::array& c1 = args().one_of("c1", "corner1");
            const rjson::array& c2 = args().one_of("c2", "corner2");
            auto& rectangle = aChartDraw.rectangle({c1[0], c1[1]}, {c2[0], c2[1]});
            rectangle.filled(args().get_or_default("filled", false));
            rectangle.color(Color(args().get_or_default("color", "#80FF00FF")));
            rectangle.line_width(args().get_or_default("line_width", 1));
        }

}; // class ModRectangle

// ----------------------------------------------------------------------

class ModCircle : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            const rjson::array& center = args()["center"];
            auto& circle = aChartDraw.circle({center[0], center[1]}, Scaled{args().get_or_default("size", 1.0)});
            circle.color(Color(args().get_or_default("fill", "transparent")), Color(args().get_or_default("outline", "#80FF00FF")));
            circle.outline_width(args().get_or_default("outline_width", 1.0));
            circle.aspect(Aspect{args().get_or_default("aspect", 1.0)});
            circle.rotation(Rotation{args().get_or_default("rotation", 0.0)});
        }

}; // class ModCircle

// ----------------------------------------------------------------------

Mods factory(const rjson::value& aMod, const rjson::object& aSettingsMods, const rjson::object& aUpdate)
{
    std::string name;
    rjson::object args;
    if (auto ptr_obj = std::get_if<rjson::object>(&aMod)) {
        name = ptr_obj->get_or_default("N", "");
        if (name.empty()) {
            if (ptr_obj->exists("?N"))
                name = "?"; // no "N" but "?N" present, avoid warning about commented out mode
        }
        args.update(*ptr_obj);
    }
    else if (auto ptr_str = std::get_if<rjson::string>(&aMod)) {
        name = ptr_str->str();
    }
    args.update(aUpdate);

    Mods result;

    auto get_referenced_mod = [&aSettingsMods](std::string aName) -> const rjson::array& {
        try {
            if (!aName.empty() && aName[0] == '*')
                return aSettingsMods[aName.substr(1)];
            else
                return aSettingsMods[aName];
        }
        catch (rjson::field_not_found&) {
            if (!aName.empty() && aName[0] == '*') // optional mod
                return rjson::sEmptyArray;
            throw unrecognized_mod{"mod not found: " + aName};
        }
        catch (std::bad_variant_access&) {
            throw unrecognized_mod{"[\"mods\"][\"" + aName + "\"] is not an array:\n\n" + aSettingsMods.to_json_pp(2, rjson::json_pp_emacs_indent::no) + "\n\n"};
        }
    };

    if (name == "antigens") {
        result.emplace_back(new ModAntigens(args));
    }
    else if (name == "sera") {
        result.emplace_back(new ModSera(args));
    }
    else if (name == "move_antigens") {
        result.emplace_back(new ModMoveAntigens(args));
    }
    else if (name == "move_sera") {
        result.emplace_back(new ModMoveSera(args));
    }
    else if (name == "amino-acids") {
        result.emplace_back(new ModAminoAcids(args));
    }
    else if (name == "rotate") {
        result.emplace_back(new ModRotate(args));
    }
    else if (name == "flip") {
        result.emplace_back(new ModFlip(args));
    }
    else if (name == "viewport") {
        result.emplace_back(new ModViewport(args));
    }
    else if (name == "title") {
        result.emplace_back(new ModTitle(args));
    }
    else if (name == "legend") {
        result.emplace_back(new ModLegend(args));
    }
    else if (name == "background") {
        result.emplace_back(new ModBackground(args));
    }
    else if (name == "border") {
        result.emplace_back(new ModBorder(args));
    }
    else if (name == "grid") {
        result.emplace_back(new ModGrid(args));
    }
    else if (name == "point_scale") {
        result.emplace_back(new ModPointScale(args));
    }
    else if (name == "line") {
        result.emplace_back(new ModLine(args));
    }
    else if (name == "arrow") {
        result.emplace_back(new ModArrow(args));
    }
    else if (name == "rectangle") {
        result.emplace_back(new ModRectangle(args));
    }
    else if (name == "circle") {
        result.emplace_back(new ModCircle(args));
    }
    else if (name == "serum_circle") {
        result.emplace_back(new ModSerumCircle(args));
    }
    else if (name == "serum_coverage") {
        result.emplace_back(new ModSerumCoverage(args));
    }
    else if (name == "serum_coverage_circle") {
        result.emplace_back(new ModSerumCoverageCircle(args));
    }
    else if (name == "serum_line") {
        result.emplace_back(new ModSerumLine(args));
    }
    else if (name == "procrustes_arrows") {
        result.emplace_back(new ModProcrustesArrows(args));
    }
    else if (name == "comment") {
        // comment mod silently ignored
    }
    else if (name.empty()) {
        std::cerr << "WARNING: mod ignored (no \"N\"): " << args << '\n';
    }
    else if (name.front() == '?' || name.back() == '?') {
        // commented out
    }
    else {
        for (const auto& submod_desc : get_referenced_mod(name)) {
            for (auto&& submod : factory(submod_desc, aSettingsMods, args)) {
                result.push_back(std::move(submod));
            }
        }
    }

    return result;

} // factory

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
