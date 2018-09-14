#include <string>
//using namespace std::string_literals;

#include "acmacs-base/rjson.hh"
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
        if (const auto& val = aLabelData["color"]; !val.is_null())
            label.color(Color(static_cast<std::string_view>(val)));
        if (const auto& val = aLabelData["size"]; !val.is_null())
            label.size(val);
        if (const auto& val = aLabelData["weight"]; !val.is_null())
            label.weight(val);
        if (const auto& val = aLabelData["slant"]; !val.is_null())
            label.slant(val);
        if (const auto& val = aLabelData["font_family"]; !val.is_null())
            label.font_family(val);
        if (const auto& offset = aLabelData["offset"]; offset.size() == 2)
            label.offset({offset[0], offset[1]});

        if (const auto& display_name = aLabelData["display_name"]; !display_name.is_null()) {
            label.display_name(display_name);
        }
        else if (const auto& name_type_v = aLabelData["name_type"]; !name_type_v.is_null()) {
            const std::string_view name_type = name_type_v;
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
            else { // serum
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
    }

} // Mod::add_label

// ----------------------------------------------------------------------

void Mod::add_labels(ChartDraw& aChartDraw, const acmacs::chart::PointIndexList& aIndices, size_t aBaseIndex, const rjson::value& aLabelData)
{
    if (const auto& show = aLabelData["show"]; show.is_null() || show) { // true by default
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
    const std::string label = rjson::get_or(aLegendData, "label", "use \"label\" in \"legend\"");
    if (const auto& replace = aLegendData["replace"]; !replace.is_null() && replace) {
        if (const auto& count = aLegendData["count"]; !count.is_null() && count)
            aChartDraw.legend_point_label().remove_line(label + " (" + std::to_string(aIndices.size()) + ")");
        else
            aChartDraw.legend_point_label().remove_line(label);
    }
    add_legend(aChartDraw, aIndices, aStyle, label, aLegendData);

} // Mod::add_legend

// ----------------------------------------------------------------------

void Mod::add_legend(ChartDraw& aChartDraw, const acmacs::chart::PointIndexList& aIndices, const acmacs::PointStyle& aStyle, std::string aLabel, const rjson::value& aLegendData)
{
    if (const auto& show = aLegendData["show"]; (show.is_null() || show) && !aIndices.empty()) { // show is true by default
        if (rjson::get_or(aLegendData, "type", "") == "continent_map") {
            if (const auto& offset = aLegendData["offset"]; offset.size() != 2)
                aChartDraw.continent_map();
            else
                aChartDraw.continent_map({offset[0], offset[1]}, Pixels{rjson::get_or(aLegendData, "size", 100.0)});
        }
        else {
            auto& legend = aChartDraw.legend_point_label();
            if (const auto& count = aLegendData["count"]; !count.is_null() && count)
                aLabel += " (" + std::to_string(aIndices.size()) + ")";
            legend.add_line(*aStyle.outline, *aStyle.fill, aLabel);
        }
    }

} // Mod::add_legend

// ----------------------------------------------------------------------

static Mods factory(const rjson::value& aMod, const rjson::value& aSettingsMods, const rjson::value& aUpdate);

// ----------------------------------------------------------------------

void apply_mods(ChartDraw& aChartDraw, const rjson::value& aMods, const rjson::value& aModData, bool aIgnoreUnrecognized)
{
    const auto& mods_data_mod = aModData["mods"];
    rjson::for_each(aMods, [&mods_data_mod,&aChartDraw,aIgnoreUnrecognized,&aModData](const rjson::value& mod_desc) {
        Timeit ti{"INFO: Applying " + rjson::to_string(mod_desc) + ": "};
        try {
            for (const auto& mod: factory(mod_desc, mods_data_mod, {})) {
                mod->apply(aChartDraw, aModData);
            }
        }
        catch (std::bad_variant_access&) {
            std::cerr << "ERROR: std::bad_variant_access: in handling mod: " << mod_desc << '\n';
            throw unrecognized_mod{mod_desc};
        }
        catch (unrecognized_mod&) {
            if (aIgnoreUnrecognized)
                std::cerr << "WARNING: unrecognized mod: " << mod_desc << '\n';
            else
                throw;
        }
    });

} // apply_mods

// ----------------------------------------------------------------------

void ModAntigens::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = rjson::get_or(args(), "report", false);
    const auto report_names_threshold = rjson::get_or(args(), "report_names_threshold", 10UL);
    if (const auto& select = args()["select"]; !select.is_null()) {
        const auto indices = SelectAntigens(verbose, report_names_threshold).select(aChartDraw, select);
        const auto styl = style();
          // if (verbose)
          //     std::cerr << "DEBUG ModAntigens " << indices << ' ' << args() << ' ' << styl << '\n';
        aChartDraw.modify(indices, styl, drawing_order());
        if (const auto& label = args()["label"]; !label.is_null())
            add_labels(aChartDraw, indices, 0, label);
        if (const auto& legend = args()["legend"]; !legend.is_null())
            add_legend(aChartDraw, indices, styl, legend);
    }
    else {
        throw unrecognized_mod{"no \"select\" in \"antigens\" mod: " + rjson::to_string(args())};
    }

} // ModAntigens::apply

// ----------------------------------------------------------------------

acmacs::Coordinates ModMoveBase::get_move_to(ChartDraw& aChartDraw, bool aVerbose) const
{
    acmacs::Coordinates move_to;
    if (const auto& to = args()["to"]; !to.is_null()) {
        move_to = acmacs::Coordinates{to[0], to[1]};
    }
    else if (const auto& to_antigen = args()["to_antigen"]; !to_antigen.is_null()) {
        const auto antigens = SelectAntigens(aVerbose).select(aChartDraw, to_antigen);
        if (antigens.size() != 1)
            throw unrecognized_mod{"\"to_antigen\" does not select single antigen, mod: " + rjson::to_string(args())};
        move_to = aChartDraw.layout()->get(antigens.front());
    }
    else if (const auto& to_serum = args()["to_serum"]; !to_serum.is_null()) {
        const auto sera = SelectSera(aVerbose).select(aChartDraw, to_serum);
        if (sera.size() != 1)
            throw unrecognized_mod{"\"to_serum\" does not select single serum, mod: " + rjson::to_string(args())};
        move_to = aChartDraw.layout()->get(sera.front() + aChartDraw.number_of_antigens());
    }
    else
        throw unrecognized_mod{"neither of \"to\", \"to_antigen\", \"to__serum\" provided in mod: " + rjson::to_string(args())};
    return move_to;

} // ModMoveBase::get_move_to

// ----------------------------------------------------------------------

void ModMoveAntigens::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = rjson::get_or(args(), "report", false);
    if (const auto& select = args()["select"]; !select.is_null()) {
        auto& projection = aChartDraw.projection();
        if (auto flip_scale = rjson::get_or(args(), "flip_over_serum_line", std::numeric_limits<double>::max()); flip_scale < (std::numeric_limits<double>::max() / 2)) {
            const acmacs::chart::SerumLine serum_line(projection);
            auto layout = aChartDraw.layout();
            for (auto index : SelectAntigens(verbose).select(aChartDraw, select)) {
                const auto flipped = serum_line.line().flip_over(layout->get(index), flip_scale);
                projection.move_point(index, flipped);
            }
        }
        else {
            const auto move_to = get_move_to(aChartDraw, verbose);
            for (auto index : SelectAntigens(verbose).select(aChartDraw, select)) {
                projection.move_point(index, move_to);
            }
        }
    }
    else {
        throw unrecognized_mod{"no \"select\" in \"move_antigens\" mod: " + rjson::to_string(args())};
    }

} // ModMoveAntigens::apply

// ----------------------------------------------------------------------

void ModMoveSera::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = rjson::get_or(args(), "report", false);
    if (const auto& select = args()["select"]; !select.is_null()) {
        const auto move_to = get_move_to(aChartDraw, verbose);
        auto& projection = aChartDraw.projection();
        for (auto index: SelectSera(verbose).select(aChartDraw, select)) {
            projection.move_point(index + aChartDraw.number_of_antigens(), move_to);
        }
    }
    else {
        throw unrecognized_mod{"no \"select\" in \"move_sera\" mod: " + rjson::to_string(args()) };
    }

} // ModMoveSera::apply

// ----------------------------------------------------------------------

class ModRotate : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
    {
        if (const auto& degrees_v = args()["degrees"]; !degrees_v.is_null()) {
            const double pi_180 = std::acos(-1) / 180.0;
            aChartDraw.rotate(static_cast<double>(degrees_v) * pi_180);
        }
        else if (const auto& radians_v = args()["radians"]; !radians_v.is_null()) {
            aChartDraw.rotate(radians_v);
        }
        else {
            throw unrecognized_mod{"mod: " + rjson::to_string(args())};
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
        if (const auto& direction_v = args()["direction"]; !direction_v.is_null()) {
            const std::string_view direction = direction_v;
            if (direction == "ew")
                aChartDraw.flip(0, 1);
            else if (direction == "ns")
                aChartDraw.flip(1, 0);
            else
                throw unrecognized_mod{"mod: " + rjson::to_string(args())};
        }
        else {
            throw unrecognized_mod{"mod: " + rjson::to_string(args())};
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
        if (const auto& abs = args()["abs"]; !abs.is_null()) {
            if (abs.size() != 3)
                throw unrecognized_mod{"\"abs\" must be array of 3 floats. mod: " + rjson::to_string(args())};
            aChartDraw.viewport({abs[0], abs[1]}, abs[2]);
        }
        else if (const auto& rel = args()["rel"]; !rel.is_null()) {
            if (rel.size() != 3)
                throw unrecognized_mod{"\"rel\" must be array of 3 floats. mod: " + rjson::to_string(args())};
            aChartDraw.calculate_viewport(false);
            const auto& orig_viewport = aChartDraw.viewport();
            const auto new_size = static_cast<double>(rel[2]) + orig_viewport.size.width;
            if (new_size < 1)
                throw unrecognized_mod{"invalid size difference in \"rel\". mod: " + rjson::to_string(args())};
            aChartDraw.viewport(orig_viewport.origin + acmacs::Location2D{static_cast<double>(rel[0]), static_cast<double>(rel[1])}, new_size);
        }
        else {
            throw unrecognized_mod{"mod: " + rjson::to_string(args())};
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
        if (const auto& color = args()["color"]; !color.is_null())
            aChartDraw.background_color(Color(static_cast<std::string_view>(color)));
        else
            throw unrecognized_mod{"mod: " + rjson::to_string(args())};
    }

}; // class ModBackground

// ----------------------------------------------------------------------

class ModBorder : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            aChartDraw.border(Color(rjson::get_or(args(), "color", "black")), rjson::get_or(args(), "line_width", 1.0));
        }

}; // class ModBorder

// ----------------------------------------------------------------------

class ModGrid : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            aChartDraw.grid(Color(rjson::get_or(args(), "color", "grey80")), rjson::get_or(args(), "line_width", 1.0));
        }

}; // class ModGrid

// ----------------------------------------------------------------------

class ModPointScale : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            aChartDraw.scale_points(rjson::get_or(args(), "scale", 1.0), rjson::get_or(args(), "outline_scale", 1.0));
        }

}; // class ModPointScale

// ----------------------------------------------------------------------

class ModTitle : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
    {
        auto& title = aChartDraw.title();
        if (const auto& show = args()["show"]; show.is_null() || show) { // true by default
            title.show(true);
            if (const auto& display_name = args()["display_name"]; !display_name.is_null())
                rjson::for_each(display_name, [&title](const rjson::value& line) { title.add_line(line); });
            else
                title.add_line(aChartDraw.chart().make_name(aChartDraw.projection_no()));
            if (const auto& offset = args()["offset"]; !offset.is_null())
                title.offset({offset[0], offset[1]});
            if (const auto& padding = args()["padding"]; !padding.is_null())
                title.padding(padding);
            if (const auto& text_size = args()["text_size"]; !text_size.is_null())
                title.text_size(text_size);
            if (const auto& text_color = args()["text_color"]; !text_color.is_null())
                title.text_color(Color(static_cast<std::string_view>(text_color)));
            if (const auto& background = args()["background"]; !background.is_null())
                title.background(Color(static_cast<std::string_view>(background)));
            if (const auto& border_color = args()["border_color"]; !border_color.is_null())
                title.border_color(Color(static_cast<std::string_view>(border_color)));
            if (const auto& border_width = args()["border_width"]; !border_width.is_null())
                title.border_width(border_width);
            if (const auto& font_weight = args()["font_weight"]; !font_weight.is_null())
                title.weight(font_weight);
            if (const auto& font_slant = args()["font_slant"]; !font_slant.is_null())
                title.slant(font_slant);
            if (const auto& font_family = args()["font_family"]; !font_family.is_null())
                title.font_family(font_family);
        }
        else {
            title.show(false);
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
        if (const auto& show = args()["show"]; show.is_null() || show) { // true by default
            auto& legend = aChartDraw.legend_point_label();
            if (const auto& data = args()["data"]; !data.is_null()) {
                rjson::for_each(data, [&legend](const rjson::value& line_data) {
                    legend.add_line(Color(rjson::get_or(line_data, "outline", "black")), Color(rjson::get_or(line_data, "fill", "pink")),
                                    rjson::get_or(line_data, "display_name", "* no display_name *"));
                });
            }
            if (const auto& offset = args()["offset"]; !offset.is_null())
                legend.offset({offset[0], offset[1]});
            if (const auto& label_size = args()["label_size"]; !label_size.is_null())
                legend.label_size(label_size);
            if (const auto& point_size = args()["point_size"]; !point_size.is_null())
                legend.point_size(point_size);
            if (const auto& background = args()["background"]; !background.is_null())
                legend.background(Color(static_cast<std::string_view>(background)));
            if (const auto& border_color = args()["border_color"]; !border_color.is_null())
                legend.border_color(Color(static_cast<std::string_view>(border_color)));
            if (const auto& border_width = args()["border_width"]; !border_width.is_null())
                legend.border_width(border_width);
        }
        else {
            aChartDraw.remove_legend();
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
                    line.color(Color(rjson::get_or(args(), "color", "black")));
                    line.line_width(rjson::get_or(args(), "width", 1.0));
                }
            }
        }

 protected:
   std::vector<acmacs::Location2D> begins_ends(ChartDraw& aChartDraw, std::string aPrefix) const
   {
       std::vector<acmacs::Location2D> result;
       const auto verbose = rjson::get_or(args(), "report", false);
       if (const auto& from = args()[aPrefix]; !from.is_null()) {
           result.push_back({from[0], from[1]});
       }
       else if (const auto& from_antigen = args()[aPrefix + "_antigen"]; !from_antigen.is_null()) {
           auto layout = aChartDraw.layout();
           for (auto index : SelectAntigens(verbose).select(aChartDraw, from_antigen)) {
               const auto coord = layout->get(index);
               result.push_back({coord[0], coord[1]});
           }
       }
       else if (const auto& from_serum = args()[aPrefix + "_serum"]; !from_serum.is_null()) {
           auto layout = aChartDraw.layout();
           for (auto index : SelectSera(verbose).select(aChartDraw, from_serum)) {
               const auto coord = layout->get(index + aChartDraw.number_of_antigens());
               result.push_back({coord[0], coord[1]});
           }
       }
       else
           throw unrecognized_mod{"neither \"" + aPrefix + "\" nor \"" + aPrefix + "_antigen\" nor \"" + aPrefix + "_serum\" provided in mod: " + rjson::to_string(args())};
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
                    const auto color(rjson::get_or(args(), "color", "black"));
                    arrow.color(Color(color), Color(rjson::get_or(args(), "head_color", color)));
                    arrow.line_width(rjson::get_or(args(), "width", 1.0));
                    arrow.arrow_head_filled(rjson::get_or(args(), "head_filled", true));
                    arrow.arrow_width(rjson::get_or(args(), "arrow_width", 5.0));
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
        if (const auto& c1 = rjson::one_of(args(), "c1", "corner1"), c2 = rjson::one_of(args(), "c2", "corner2"); !c1.is_null() && !c2.is_null()) {
            auto& rectangle = aChartDraw.rectangle({c1[0], c1[1]}, {c2[0], c2[1]});
            rectangle.filled(rjson::get_or(args(), "filled", false));
            rectangle.color(Color(rjson::get_or(args(), "color", "#80FF00FF")));
            rectangle.line_width(rjson::get_or(args(), "line_width", 1));
        }
        else
            throw unrecognized_mod{"mod: " + rjson::to_string(args())};
    }

}; // class ModRectangle

// ----------------------------------------------------------------------

class ModCircle : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
    {
        if (const auto& center = args()["center"]; !center.is_null()) {
            auto& circle = aChartDraw.circle({center[0], center[1]}, Scaled{rjson::get_or(args(), "size", 1.0)});
            circle.color(Color(rjson::get_or(args(), "fill", "transparent")), Color(rjson::get_or(args(), "outline", "#80FF00FF")));
            circle.outline_width(rjson::get_or(args(), "outline_width", 1.0));
            circle.aspect(Aspect{rjson::get_or(args(), "aspect", 1.0)});
            circle.rotation(Rotation{rjson::get_or(args(), "rotation", 0.0)});
        }
        else
            throw unrecognized_mod{"mod: " + rjson::to_string(args())};
    }

}; // class ModCircle

// ----------------------------------------------------------------------

Mods factory(const rjson::value& aMod, const rjson::value& aSettingsMods, const rjson::value& aUpdate)
{
    std::string name;
    rjson::value args{rjson::object{}};
    if (aMod.is_object()) {
        name = rjson::get_or(aMod, "N", "");
        if (name.empty() && !aMod["?N"].is_null())
            name = "?"; // no "N" but "?N" present, avoid warning about commented out mode
        args.update(aMod);
    }
    else if (aMod.is_string()) {
        name = static_cast<std::string>(aMod);
    }
    args.update(aUpdate);

    Mods result;

    auto get_referenced_mod = [&aSettingsMods](std::string aName) -> const rjson::value& {
        auto get_mod = [&aSettingsMods, &aName]() -> const rjson::value& {
            if (!aName.empty() && aName[0] == '*')
                return aSettingsMods[aName.substr(1)];
            else
                return aSettingsMods[aName];
        };
        if (const auto& mod = get_mod(); !mod.is_null())
            return mod;
        else if (!aName.empty() && aName[0] == '*') // optional mod
            return rjson::EmptyArray;
        else
            throw unrecognized_mod{"mod not found: " + aName};
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
        rjson::for_each(get_referenced_mod(name), [&aSettingsMods,&args,&result](const rjson::value& submod_desc) {
            for (auto&& submod : factory(submod_desc, aSettingsMods, args)) {
                result.push_back(std::move(submod));
            }
        });
    }

    return result;

} // factory

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
