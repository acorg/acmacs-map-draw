#include "acmacs-base/rjson.hh"
#include "acmacs-chart-2/serum-line.hh"
#include "acmacs-map-draw/draw.hh"
#include "acmacs-map-draw/mod-applicator.hh"
#include "acmacs-map-draw/mod-serum.hh"
#include "acmacs-map-draw/mod-procrustes.hh"
#include "acmacs-map-draw/mod-amino-acids.hh"
#include "acmacs-map-draw/mod-blobs.hh"
#include "acmacs-map-draw/select.hh"
#include "acmacs-map-draw/point-style-draw.hh"
#include "acmacs-map-draw/mod-connection-lines.hh"

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
            label.color(Color(val.to<std::string_view>()));
        if (const auto& val = aLabelData["size"]; !val.is_null())
            label.size(val.to<double>());
        if (const auto& val = aLabelData["weight"]; !val.is_null())
            label.weight(val.to<std::string_view>());
        if (const auto& val = aLabelData["slant"]; !val.is_null())
            label.slant(val.to<std::string_view>());
        if (const auto& val = aLabelData["font_family"]; !val.is_null())
            label.font_family(val.to<std::string_view>());
        if (const auto& offset = aLabelData["offset"]; offset.size() == 2)
            label.offset({offset[0].to<double>(), offset[1].to<double>()});

        if (const auto& display_name = aLabelData["display_name"]; !display_name.is_null()) {
            label.display_name(display_name.to<std::string_view>());
        }
        else if (const auto& name_type_v = aLabelData["name_type"]; !name_type_v.is_null()) {
            const std::string_view name_type{name_type_v.to<std::string_view>()};
            if (aBaseIndex == 0) { // antigen
                auto antigen = aChartDraw.chart().antigen(aIndex);
                std::string name;
                if (name_type == "abbreviated")
                    name = antigen->abbreviated_name();
                else if (name_type == "abbreviated_name_with_passage_type" || name_type == "abbreviated_with_passage_type")
                    name = antigen->abbreviated_name_with_passage_type();
                else if (name_type == "abbreviated_location_with_passage_type")
                    name = antigen->abbreviated_location_with_passage_type();
                else if (name_type == "abbreviated_location_year")
                    name = antigen->abbreviated_location_year();
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
                else if (name_type == "abbreviated_location_year")
                    label.display_name(serum->abbreviated_location_year());
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
    if (const auto& show = aLabelData["show"]; show.is_null() || show.to<bool>()) { // true by default
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
    const std::string label(rjson::get_or(aLegendData, "label", "use \"label\" in \"legend\""));
    if (const auto& replace = aLegendData["replace"]; !replace.is_null() && replace.to<bool>()) {
        if (const auto& count = aLegendData["count"]; !count.is_null() && count.to<bool>()) {
            // std::cerr << "DEBUG: remove line " << label + " (" + std::to_string(aIndices.size()) + ")" << '\n';
            aChartDraw.legend_point_label().remove_line(label + " (" + std::to_string(aIndices->size()) + ")");
        }
        else {
            // std::cerr << "DEBUG: add_legend remove line " << label << '\n';
            aChartDraw.legend_point_label().remove_line(label);
        }
    }
    add_legend(aChartDraw, aIndices, aStyle, label, aLegendData);

} // Mod::add_legend

// ----------------------------------------------------------------------

void Mod::add_legend(ChartDraw& aChartDraw, const acmacs::chart::PointIndexList& aIndices, const acmacs::PointStyle& aStyle, std::string aLabel, const rjson::value& aLegendData)
{
    // std::cerr << "DEBUG: add_legend " << aLabel << '\n';
    if (const auto& show = aLegendData["show"]; (show.is_null() || show.to<bool>()) && !aIndices->empty()) { // show is true by default
        if (rjson::get_or(aLegendData, "type", "") == "continent_map") {
            if (const auto& offset = aLegendData["offset"]; offset.size() != 2)
                aChartDraw.continent_map();
            else
                aChartDraw.continent_map({offset[0].to<double>(), offset[1].to<double>()}, Pixels{rjson::get_or(aLegendData, "size", 100.0)});
        }
        else {
            auto& legend = aChartDraw.legend_point_label();
            if (const auto& count = aLegendData["count"]; !count.is_null() && count.to<bool>())
                aLabel += " (" + std::to_string(aIndices->size()) + ")";
            legend.add_line(acmacs::color::get(*aStyle.outline), acmacs::color::get(*aStyle.fill), aLabel);
        }
    }

} // Mod::add_legend

// ----------------------------------------------------------------------

static Mods factory(ChartDraw& aChartDraw, const rjson::value& aMod, const rjson::value& aSettingsMods, const rjson::value& aUpdate, acmacs::verbose verbose, size_t level = 0);

// ----------------------------------------------------------------------

void apply_mods(ChartDraw& aChartDraw, const rjson::value& aMods, const rjson::value& aModData, acmacs::verbose verbose, bool aIgnoreUnrecognized)
{
    const auto& mods_data_mod = aModData["mods"];
    rjson::for_each(aMods, [&mods_data_mod,&aChartDraw,aIgnoreUnrecognized,&aModData,verbose](const rjson::value& mod_desc) {
        try {
            // if (verbose == acmacs::verbose::yes)
            //     fmt::print(stderr, "DEBUG: apply mod: {}\n", mod_desc);
            for (const auto& mod: factory(aChartDraw, mod_desc, mods_data_mod, {}, verbose)) {
                // Timeit ti{"INFO: Applying " + rjson::to_string(mod_desc) + ": "};
                mod->apply(aChartDraw, aModData);
            }
        }
        catch (std::bad_variant_access&) {
            fmt::print(stderr, "ERROR: std::bad_variant_access: in handling mod: {}\n", mod_desc);
            throw unrecognized_mod(mod_desc.to<std::string>());
        }
        catch (unrecognized_mod&) {
            if (aIgnoreUnrecognized)
                fmt::print(stderr, "WARNING: unrecognized mod: {}\n", mod_desc);
            else
                throw;
        }
    });

} // apply_mods

// ----------------------------------------------------------------------

void ModAntigens::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = rjson::get_or(args(), "report", false);
    const auto report_names_threshold = rjson::get_or(args(), "report_names_threshold", 30UL);
    if (const auto& select = args()["select"]; !select.is_null()) {
        const auto indices = SelectAntigens(acmacs::verbose_from(verbose), report_names_threshold).select(aChartDraw, select);
        const auto styl = style();
          // if (verbose)
          // std::cerr << "DEBUG: ModAntigens " << indices << ' ' << args() << ' ' << styl << '\n';
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

acmacs::PointCoordinates ModMoveBase::get_move_to(ChartDraw& aChartDraw, bool aVerbose) const
{
    acmacs::PointCoordinates move_to(acmacs::number_of_dimensions_t{2});
    if (const auto& to = args()["to"]; !to.is_null()) {
        move_to = acmacs::PointCoordinates(to[0].to<double>(), to[1].to<double>());
    }
    else if (const auto& to_antigen = args()["to_antigen"]; !to_antigen.is_null()) {
        const auto antigens = SelectAntigens(acmacs::verbose_from(aVerbose)).select(aChartDraw, to_antigen);
        if (antigens->size() != 1)
            throw unrecognized_mod{"\"to_antigen\" does not select single antigen, mod: " + rjson::to_string(args())};
        move_to = aChartDraw.layout()->at(antigens->front());
    }
    else if (const auto& to_serum = args()["to_serum"]; !to_serum.is_null()) {
        const auto sera = SelectSera(acmacs::verbose_from(aVerbose)).select(aChartDraw, to_serum);
        if (sera->size() != 1)
            throw unrecognized_mod{"\"to_serum\" does not select single serum, mod: " + rjson::to_string(args())};
        move_to = aChartDraw.layout()->at(sera->front() + aChartDraw.number_of_antigens());
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
            for (auto index : SelectAntigens(acmacs::verbose_from(verbose)).select(aChartDraw, select)) {
                const auto flipped = serum_line.line().flip_over(layout->at(index), flip_scale);
                projection.move_point(index, flipped);
            }
        }
        else if (const auto& flip_line = args()["flip_over_line"]; !flip_line.is_null()) {
            acmacs::PointCoordinates from{flip_line["from"][0].to<double>(), flip_line["from"][1].to<double>()}, to{flip_line["to"][0].to<double>(), flip_line["to"][1].to<double>()};
            if (!rjson::get_or(flip_line, "transform", true)) {
                const auto transformation = aChartDraw.transformation().inverse();
                from = transformation.transform(from);
                to = transformation.transform(to);
            }
            const acmacs::LineDefinedByEquation line(from, to);
            // fmt::print(stderr, "DEBUG: flip line ({}X + {})\n", line.slope(), line.intercept());
            auto layout = aChartDraw.layout();
            for (auto index : SelectAntigens(acmacs::verbose_from(verbose)).select(aChartDraw, select)) {
                const auto flipped = line.flip_over(layout->at(index), 1.0);
                projection.move_point(index, flipped);
            }
        }
        else if (auto relative = args().get("relative"); !relative.is_null()) {
            auto layout = aChartDraw.layout();
            for (auto index : SelectAntigens(acmacs::verbose_from(verbose)).select(aChartDraw, select)) {
                const auto coord = layout->at(index);
                projection.move_point(index, acmacs::PointCoordinates(coord.x() + relative[0].to<double>(), coord.y() + relative[1].to<double>()));
            }
        }
        else {
            const auto move_to = get_move_to(aChartDraw, verbose);
            for (auto index : SelectAntigens(acmacs::verbose_from(verbose)).select(aChartDraw, select)) {
                projection.move_point(index, move_to);
            }
        }
    }
    else {
        throw unrecognized_mod{"no \"select\" in \"move_antigens\" mod: " + rjson::to_string(args())};
    }

} // ModMoveAntigens::apply

// ----------------------------------------------------------------------

void ModMoveAntigensStress::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = rjson::get_or(args(), "report", false);
    if (const auto& select = args()["select"]; !select.is_null()) {
        auto& projection = aChartDraw.projection();
        const auto projection_stress = projection.stress();
        const auto transformation = projection.transformation();
        if (auto relative = args().get("relative"); !relative.is_null()) {
            auto layout = projection.layout();
            for (auto index : SelectAntigens(acmacs::verbose_from(verbose)).select(aChartDraw, select)) {
                const auto coord = layout->at(index);
                const acmacs::PointCoordinates move_to{coord.x() + relative[0].to<double>(), coord.y() + relative[1].to<double>()};
                const auto stress = projection.stress_with_moved_point(index, move_to);
                std::cerr << "DEBUG: stress_with_moved_point " << stress << '\n';
                auto& point = aChartDraw.point(transformation.transform(move_to), Pixels{rjson::get_or(args(), "size", 1.0)});
                point.color(Color(rjson::get_or(args(), "fill", "transparent")), Color(rjson::get_or(args(), "outline", "black")));
                point.outline_width(rjson::get_or(args(), "outline_width", 1.0));
                point.label(acmacs::to_string(stress, 4) + " " + acmacs::to_string(stress - projection_stress, 4));
            }
        }
        else {
            // const auto move_to = get_move_to(aChartDraw, verbose);
            // for (auto index : SelectAntigens(verbose).select(aChartDraw, select)) {
            //     // projection.move_point(index, move_to);
            // }
        }
    }
    else {
        throw unrecognized_mod{"no \"select\" in \"move_antigens\" mod: " + rjson::to_string(args())};
    }

} // ModMoveAntigensStress::apply

// ----------------------------------------------------------------------

void ModMoveSera::apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/)
{
    const auto verbose = rjson::get_or(args(), "report", false);
    if (const auto& select = args()["select"]; !select.is_null()) {
        auto& projection = aChartDraw.projection();
        if (auto relative = args().get("relative"); !relative.is_null()) {
            auto layout = aChartDraw.layout();
            for (auto index : SelectSera(acmacs::verbose_from(verbose)).select(aChartDraw, select)) {
                const auto coord = layout->at(index + aChartDraw.number_of_antigens());
                projection.move_point(index + aChartDraw.number_of_antigens(), acmacs::PointCoordinates(coord.x() + relative[0].to<double>(), coord.y() + relative[1].to<double>()));
            }
        }
        else if (const auto& flip_line = args()["flip_over_line"]; !flip_line.is_null()) {
            acmacs::PointCoordinates from{flip_line["from"][0].to<double>(), flip_line["from"][1].to<double>()}, to{flip_line["to"][0].to<double>(), flip_line["to"][1].to<double>()};
            if (!rjson::get_or(flip_line, "transform", true)) {
                const auto transformation = aChartDraw.transformation().inverse();
                from = transformation.transform(from);
                to = transformation.transform(to);
            }
            const acmacs::LineDefinedByEquation line(from, to);
            auto layout = aChartDraw.layout();
            for (auto index : SelectSera(acmacs::verbose_from(verbose)).select(aChartDraw, select)) {
                const auto flipped = line.flip_over(layout->at(index + aChartDraw.number_of_antigens()), 1.0);
                projection.move_point(index + aChartDraw.number_of_antigens(), flipped);
            }
        }
        else {
            const auto move_to = get_move_to(aChartDraw, verbose);
            for (auto index : SelectSera(acmacs::verbose_from(verbose)).select(aChartDraw, select)) {
                projection.move_point(index + aChartDraw.number_of_antigens(), move_to);
            }
        }
    }
    else {
        throw unrecognized_mod{"no \"select\" in \"move_sera\" mod: " + rjson::to_string(args())};
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
            aChartDraw.rotate(degrees_v.to<double>() * pi_180);
        }
        else if (const auto& radians_v = args()["radians"]; !radians_v.is_null()) {
            aChartDraw.rotate(radians_v.to<double>());
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
            const std::string_view direction{direction_v.to<std::string_view>()};
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
            aChartDraw.viewport({abs[0].to<double>(), abs[1].to<double>()}, abs[2].to<double>());
        }
        else if (const auto& rel = args()["rel"]; !rel.is_null()) {
            if (rel.size() != 3)
                throw unrecognized_mod{"\"rel\" must be array of 3 floats. mod: " + rjson::to_string(args())};
            aChartDraw.calculate_viewport(false);
            const auto& orig_viewport = aChartDraw.viewport();
            const auto new_size = rel[2].to<double>() + orig_viewport.size.width;
            if (new_size < 1)
                throw unrecognized_mod{"invalid size difference in \"rel\". mod: " + rjson::to_string(args())};
            aChartDraw.viewport(orig_viewport.origin + acmacs::PointCoordinates{rel[0].to<double>(), rel[1].to<double>()}, new_size);
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
            aChartDraw.background_color(Color(color.to<std::string_view>()));
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
        if (const auto& show = args()["show"]; show.is_null() || show.to<bool>()) { // true by default
            title.show(true);
            auto substitute_vars = [&aChartDraw](std::string source) -> std::string {
                auto info = aChartDraw.chart().info();
                return string::replace(source,
                                       "{lab}", info->lab(acmacs::chart::Info::Compute::Yes, acmacs::chart::Info::FixLab::reverse),
                                       "{assay}", info->assay(acmacs::chart::Info::Compute::Yes),
                                       "{assay_short}", info->assay(acmacs::chart::Info::Compute::Yes).hi_or_neut(),
                                       "{virus_type}", info->virus_type(acmacs::chart::Info::Compute::Yes),
                                       "{lineage}", aChartDraw.chart().lineage(),
                                       "{rbc}", info->rbc_species(acmacs::chart::Info::Compute::Yes),
                                       "{subset}", info->subset(acmacs::chart::Info::Compute::Yes),
                                       "{date}", info->date(acmacs::chart::Info::Compute::Yes),
                                       "{name}", aChartDraw.chart().make_name()
                                       );
            };
            if (const auto& display_name = args()["display_name"]; !display_name.is_null()) {
                if (display_name.is_array())
                    rjson::for_each(display_name, [&title,&substitute_vars](const rjson::value& line) { title.add_line(substitute_vars(line.to<std::string>())); });
                else if (display_name.is_string())
                    title.add_line(substitute_vars(display_name.to<std::string>()));
                else
                    throw std::exception{};
            }
            else
                title.add_line(aChartDraw.chart().make_name(aChartDraw.projection_no()));
            if (const auto& offset = args()["offset"]; !offset.is_null())
                title.offset({offset[0].to<double>(), offset[1].to<double>()});
            if (const auto& padding = args()["padding"]; !padding.is_null())
                title.padding(padding.to<double>());
            if (const auto& text_size = args()["text_size"]; !text_size.is_null())
                title.text_size(text_size.to<double>());
            if (const auto& text_color = args()["text_color"]; !text_color.is_null())
                title.text_color(Color(text_color.to<std::string_view>()));
            if (const auto& background = args()["background"]; !background.is_null())
                title.background(Color(background.to<std::string_view>()));
            if (const auto& border_color = args()["border_color"]; !border_color.is_null())
                title.border_color(Color(border_color.to<std::string_view>()));
            if (const auto& border_width = args()["border_width"]; !border_width.is_null())
                title.border_width(border_width.to<double>());
            if (const auto& font_weight = args()["font_weight"]; !font_weight.is_null())
                title.weight(font_weight.to<std::string>());
            if (const auto& font_slant = args()["font_slant"]; !font_slant.is_null())
                title.slant(font_slant.to<std::string>());
            if (const auto& font_family = args()["font_family"]; !font_family.is_null())
                title.font_family(font_family.to<std::string>());
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
        if (const auto& show = args()["show"]; show.is_null() || show.to<bool>()) { // true by default
            auto& legend = aChartDraw.legend_point_label();
            if (const auto& data = args()["data"]; !data.is_null()) {
                rjson::for_each(data, [&legend](const rjson::value& line_data) {
                    legend.add_line(Color(rjson::get_or(line_data, "outline", "black")), Color(rjson::get_or(line_data, "fill", "pink")),
                                    std::string(rjson::get_or(line_data, "display_name", "* no display_name *")));
                });
            }
            if (const auto& offset = args()["offset"]; !offset.is_null())
                legend.offset({offset[0].to<double>(), offset[1].to<double>()});
            if (const auto& label_size = args()["label_size"]; !label_size.is_null())
                legend.label_size(label_size.to<double>());
            if (const auto& point_size = args()["point_size"]; !point_size.is_null())
                legend.point_size(point_size.to<double>());
            if (const auto& background = args()["background"]; !background.is_null())
                legend.background(Color(background.to<std::string_view>()));
            if (const auto& border_color = args()["border_color"]; !border_color.is_null())
                legend.border_color(Color(border_color.to<std::string_view>()));
            if (const auto& border_width = args()["border_width"]; !border_width.is_null())
                legend.border_width(border_width.to<double>());
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
            const bool transform = rjson::get_or(args(), "transform", false);
            const auto begins = begins_ends(aChartDraw, "from", transform);
            const auto ends = begins_ends(aChartDraw, "to", transform);
            for(const auto& begin: begins) {
                for(const auto& end: ends) {
                    aChartDraw.line(begin, end)
                            .color(Color(rjson::get_or(args(), "color", "black")))
                            .line_width(rjson::get_or(args(), "width", 1.0));
                }
            }
        }

 protected:
    std::vector<acmacs::PointCoordinates> begins_ends(ChartDraw& aChartDraw, std::string aPrefix, bool transform) const
   {
       std::vector<acmacs::PointCoordinates> result;
       const auto verbose = rjson::get_or(args(), "report", false);
       if (const auto& from = args()[aPrefix]; !from.is_null()) {
           acmacs::PointCoordinates point{from[0].to<double>(), from[1].to<double>()};
           if (transform)
               point = aChartDraw.transformation().transform(point);
           result.push_back(std::move(point));
       }
       else if (const auto& from_antigen = args()[aPrefix + "_antigen"]; !from_antigen.is_null()) {
           auto layout = aChartDraw.transformed_layout();
           for (auto index : SelectAntigens(acmacs::verbose_from(verbose)).select(aChartDraw, from_antigen))
               result.push_back(layout->at(index));
       }
       else if (const auto& from_serum = args()[aPrefix + "_serum"]; !from_serum.is_null()) {
           auto layout = aChartDraw.transformed_layout();
           for (auto index : SelectSera(acmacs::verbose_from(verbose)).select(aChartDraw, from_serum))
               result.push_back(layout->at(index + aChartDraw.number_of_antigens()));
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
            const bool transform = rjson::get_or(args(), "transform", false);
            const auto begins = begins_ends(aChartDraw, "from", transform);
            const auto ends = begins_ends(aChartDraw, "to", transform);
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
            // auto& rectangle = aChartDraw.rectangle({c1[0], c1[1]}, {c2[0], c2[1]});
            // rectangle.filled(rjson::get_or(args(), "filled", false));
            // rectangle.color(Color(rjson::get_or(args(), "color", "#80FF00FF")));
            // rectangle.line_width(rjson::get_or(args(), "line_width", 1));

            acmacs::Transformation transformation;
            if (!rjson::get_or(args(), "transform", true))
                transformation = aChartDraw.transformation();
            if (const auto rotation = rjson::get_or(args(), "rotate", 0.0); !float_zero(rotation))
                transformation.rotate(RotationRadiansOrDegrees(rotation));

            // std::cerr << "DEBUG: transformation: " << transformation << '\n';
            const Color color{rjson::get_or(args(), "color", "#80FF00FF")};
            aChartDraw.path()
                    .color(color)
                    .line_width(rjson::get_or(args(), "line_width", 1))
                    .add(transformation.transform(acmacs::PointCoordinates{c1[0].to<double>(), c1[1].to<double>()}))
                    .add(transformation.transform(acmacs::PointCoordinates{c1[0].to<double>(), c2[1].to<double>()}))
                    .add(transformation.transform(acmacs::PointCoordinates{c2[0].to<double>(), c2[1].to<double>()}))
                    .add(transformation.transform(acmacs::PointCoordinates{c2[0].to<double>(), c1[1].to<double>()}))
                    .close(rjson::get_or(args(), "filled", false) ? color : TRANSPARENT);
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
            const auto transformation = aChartDraw.transformation();
            auto& circle = aChartDraw.circle(transformation.transform(acmacs::PointCoordinates{center[0].to<double>(), center[1].to<double>()}), Scaled{rjson::get_or(args(), "size", 1.0)});
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

Mods factory(ChartDraw& aChartDraw, const rjson::value& aMod, const rjson::value& aSettingsMods, const rjson::value& aUpdate, acmacs::verbose verbose, size_t level)
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
        name = aMod.to<std::string_view>();
    }
    try {
        args.update(aUpdate);
    }
    catch (std::exception& err) {
        fmt::print(stderr, "ERROR: {}\n  main rjson::value: {}\n  rjson::value to merge in: {}\n", err, args, aUpdate);
        throw;
    }

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
        else {
            fmt::print(stderr, "ERROR: mod not found: \"{}\"\n  available mods:\n", aName);
            rjson::for_each(aSettingsMods, [](std::string_view key, const auto&) {
                if (key[0] != '?')
                    fmt::print("   {}\n", key);
            });
            throw unrecognized_mod{"mod not found: " + aName};
        }
    };

    if (verbose == acmacs::verbose::yes)
        fmt::print(stderr, "DEBUG: {:{}s}mod: \"{}\"\n", "", level * 4, name);

    if (name == "antigens") {
        result.emplace_back(new ModAntigens(args));
    }
    else if (name == "sera") {
        result.emplace_back(new ModSera(args));
    }
    else if (name == "move_antigens") {
        result.emplace_back(new ModMoveAntigens(args));
    }
    else if (name == "move_antigens_stress") {
        result.emplace_back(new ModMoveAntigensStress(args));
    }
    else if (name == "move_sera") {
        result.emplace_back(new ModMoveSera(args));
    }
    else if (name == "amino-acids") {
        result.emplace_back(new ModAminoAcids(args));
    }
    else if (name == "compare-sequences") {
        result.emplace_back(new ModCompareSequences(args));
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
    else if (name == "serum_circles") {
        result.emplace_back(new ModSerumCircles(args));
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
    else if (name == "blobs") {
        result.emplace_back(new ModBlobs(args));
    }
    else if (name == "connection_lines") {
        result.emplace_back(new ModConnectionLines(args));
    }
    else if (name == "error_lines") {
        result.emplace_back(new ModErrorLines(args));
    }
    else if (name == "color_by_number_of_connection_lines") {
        result.emplace_back(new ModColorByNumberOfConnectionLines(args));
    }
    else if (name == "comment") {
        // comment mod silently ignored
    }
    else if (name.empty() && aMod["?#"].is_null()) {
        fmt::print(stderr, "WARNING: mod ignored (no \"N\"): {}\n", args);
    }
    else if ((name.empty() && !aMod["?#"].is_null()) || name.front() == '?' || name.back() == '?') {
        // commented out
    }
    else {
        auto info = aChartDraw.chart().info();
        const auto substituted = string::replace(
            name,
            "{lab}", info->lab(acmacs::chart::Info::Compute::Yes, acmacs::chart::Info::FixLab::reverse),
            "{assay}", info->assay(acmacs::chart::Info::Compute::Yes).hi_or_neut(),
            "{virus_type}", info->virus_type(acmacs::chart::Info::Compute::Yes),
            "{lineage}", aChartDraw.chart().lineage(),
            "{rbc}", info->rbc_species(acmacs::chart::Info::Compute::Yes),
            "{subset}", info->subset(acmacs::chart::Info::Compute::Yes),
            "{date}", info->date(acmacs::chart::Info::Compute::Yes)
                                                 );
        if (verbose == acmacs::verbose::yes && name != substituted)
            fmt::print(stderr, "DEBUG: {:{}s}         \"{}\" --> \"{}\"\n", "", level * 4, name, substituted);
        rjson::for_each(get_referenced_mod(substituted), [&aChartDraw, &aSettingsMods, &args, &result, verbose, level](const rjson::value& submod_desc) {
            for (auto&& submod : factory(aChartDraw, submod_desc, aSettingsMods, args, verbose, level + 1)) {
                result.push_back(std::move(submod));
            }
        });
    }

    return result;

} // factory

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
