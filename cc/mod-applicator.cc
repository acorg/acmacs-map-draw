#include <string>
using namespace std::string_literals;

#include "acmacs-base/enumerate.hh"
#include "acmacs-base/rjson.hh" // include rjson.hh before including string.hh (included via seqdb.hh)
#include "seqdb/seqdb.hh"
#include "acmacs-map-draw/draw.hh"

#include "mod-applicator.hh"
#include "select.hh"
#include "point-style-draw.hh"

// ----------------------------------------------------------------------

class Mod
{
 public:
    inline Mod(const rjson::object& aArgs) : mArgs{aArgs} {}
    virtual inline ~Mod() { /* std::cerr << "~Mod " << args() << '\n'; */ }

    virtual void apply(ChartDraw& aChartDraw, const rjson::value& aModData) = 0;

 protected:
    const rjson::object& args() const { return mArgs; }

    inline PointStyle style() const { return point_style_from_json(args()); }
    inline PointDrawingOrder drawing_order() const { return drawing_order_from_json(args()); }
    void add_labels(ChartDraw& aChartDraw, const std::vector<size_t>& aIndices, size_t aBaseIndex, const rjson::value& aLabelData);
    void add_label(ChartDraw& aChartDraw, size_t aIndex, size_t aBaseIndex, const rjson::value& aLabelData);
    void add_legend(ChartDraw& aChartDraw, const std::vector<size_t>& aIndices, const PointStyle& aStyle, const rjson::value& aLegendData);
    void add_legend(ChartDraw& aChartDraw, const std::vector<size_t>& aIndices, const PointStyle& aStyle, std::string aLabel, const rjson::value& aLegendData);

 private:
    const rjson::object mArgs;  // not reference! "N" is probably wrong due to updating args in factory!

    friend inline std::ostream& operator << (std::ostream& out, const Mod& aMod) { return out << aMod.args(); }

}; // class Mod

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
    auto& label = aChartDraw.add_label(aIndex + aBaseIndex);
    try { label.color(static_cast<std::string>(aLabelData["color"])); } catch (rjson::field_not_found&) {}
    try { label.size(aLabelData["size"]); } catch (rjson::field_not_found&) {}
    try { label.weight(aLabelData["weight"]); } catch (rjson::field_not_found&) {}
    try { label.slant(aLabelData["slant"]); } catch (rjson::field_not_found&) {}
    try { label.font_family(aLabelData["font_family"]); } catch (rjson::field_not_found&) {}

    try {
        const rjson::array& offset = aLabelData["offset"];
        label.offset(offset[0], offset[1]);
    }
    catch (std::exception&) {
    }

    try {
        label.display_name(aLabelData["display_name"]);
    }
    catch (rjson::field_not_found&) {
        try {
            const std::string name_type = aLabelData["name_type"];
            if (aBaseIndex == 0) { // antigen
                if (name_type == "abbreviated")
                    label.display_name(aChartDraw.chart().antigen(aIndex).abbreviated_name());
                else if (name_type == "abbreviated_with_passage_type")
                    label.display_name(aChartDraw.chart().antigen(aIndex).abbreviated_name_with_passage_type());
                else {
                    if (name_type != "full")
                        std::cerr << "WARNING: unrecognized \"name_type\" for label for antigen " << aIndex << '\n';
                    label.display_name(aChartDraw.chart().antigen(aIndex).full_name());
                }
            }
            else {      // serum
                if (name_type == "abbreviated")
                    label.display_name(aChartDraw.chart().serum(aIndex).abbreviated_name());
                else if (name_type == "abbreviated_with_passage_type")
                    label.display_name(aChartDraw.chart().serum(aIndex).abbreviated_name_with_passage_type());
                else {
                    if (name_type != "full")
                        std::cerr << "WARNING: unrecognized \"name_type\" for label for serum " << aIndex << '\n';
                    label.display_name(aChartDraw.chart().serum(aIndex).full_name());
                }
            }
        }
        catch (rjson::field_not_found&) {
        }
    }

} // Mod::add_label

// ----------------------------------------------------------------------

void Mod::add_labels(ChartDraw& aChartDraw, const std::vector<size_t>& aIndices, size_t aBaseIndex, const rjson::value& aLabelData)
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

void Mod::add_legend(ChartDraw& aChartDraw, const std::vector<size_t>& aIndices, const PointStyle& aStyle, const rjson::value& aLegendData)
{
    const std::string label = aLegendData.get_or_default("label", "use \"label\" in \"legend\"");
    add_legend(aChartDraw, aIndices, aStyle, label, aLegendData);

} // Mod::add_legend

// ----------------------------------------------------------------------

void Mod::add_legend(ChartDraw& aChartDraw, const std::vector<size_t>& aIndices, const PointStyle& aStyle, std::string aLabel, const rjson::value& aLegendData)
{
    if (aLegendData.get_or_default("show", true) && !aIndices.empty()) {
        auto& legend = aChartDraw.legend();
        if (aLegendData.get_or_default("count", false))
            aLabel += " (" + std::to_string(aIndices.size()) + ")";
        legend.add_line(aStyle.outline(), aStyle.fill(), aLabel);
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
        Timeit ti{"Applying " + mod_desc.to_json() + ": "};
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
    }

} // apply_mods

// ----------------------------------------------------------------------

class ModAntigens : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            const auto verbose = args().get_or_default("report", false);
            try {
                const auto indices = SelectAntigens(verbose).select(aChartDraw.chart(), args()["select"]);
                const auto styl = style();
                // if (verbose)
                //     std::cerr << "DEBUG ModAntigens " << indices << ' ' << args() << ' ' << styl << '\n';
                aChartDraw.modify(indices.begin(), indices.end(), styl, drawing_order());
                try { add_labels(aChartDraw, indices, 0, args()["label"]); } catch (rjson::field_not_found&) {}
                try { add_legend(aChartDraw, indices, styl, args()["legend"]); } catch (rjson::field_not_found&) {}
            }
            catch (rjson::field_not_found&) {
                throw unrecognized_mod{"no \"select\" in \"antigens\" mod: " + args().to_json() };
            }
        }

}; // class ModAntigens

// ----------------------------------------------------------------------

class ModSera : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            const auto verbose = args().get_or_default("report", false);
            try {
                const auto indices = SelectSera(verbose).select(aChartDraw.chart(), args()["select"]);
                const auto styl = style();
                aChartDraw.modify_sera(indices.begin(), indices.end(), styl, drawing_order());
                try { add_labels(aChartDraw, indices, aChartDraw.number_of_antigens(), args()["label"]); } catch (rjson::field_not_found&) {}
            }
            catch (rjson::field_not_found&) {
                throw unrecognized_mod{"no \"select\" in \"sera\" mod: " + args().to_json() };
            }
        }

}; // class ModSera

// ----------------------------------------------------------------------

class ModMoveBase : public Mod
{
 public:
    using Mod::Mod;

 protected:
    Coordinates get_move_to(ChartDraw& aChartDraw, bool aVerbose) const
        {
            Coordinates move_to;
            if (auto [to_present, to] = args().get_array_if("to"); to_present) {
                move_to = Coordinates{to[0], to[1]};
            }
            else if (auto [to_antigen_present, to_antigen] = args().get_object_if("to_antigen"); to_antigen_present) {
                const auto antigens = SelectAntigens(aVerbose).select(aChartDraw.chart(), to_antigen);
                if (antigens.size() != 1)
                    throw unrecognized_mod{"\"to_antigen\" does not select single antigen, mod: " + args().to_json()};
                move_to = aChartDraw.layout()[antigens[0]];
            }
            else if (auto [to_serum_present, to_serum] = args().get_object_if("to_serum"); to_serum_present) {
                const auto sera = SelectSera(aVerbose).select(aChartDraw.chart(), to_serum);
                if (sera.size() != 1)
                    throw unrecognized_mod{"\"to_serum\" does not select single serum, mod: " + args().to_json()};
                move_to = aChartDraw.layout()[sera[0] + aChartDraw.number_of_antigens()];
            }
            else
                throw unrecognized_mod{"neither \"to\" nor \"to_antigen\" nor \"to__serum\" provided in mod: " + args().to_json()};
            return move_to;
        }

}; // class ModMoveBase

// ----------------------------------------------------------------------

class ModMoveAntigens : public ModMoveBase
{
 public:
    using ModMoveBase::ModMoveBase;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            const auto verbose = args().get_or_default("report", false);
            try {
                const auto move_to = get_move_to(aChartDraw, verbose);
                for (auto index: SelectAntigens(verbose).select(aChartDraw.chart(), args()["select"])) {
                    aChartDraw.layout().set(index, move_to);
                }
            }
            catch (rjson::field_not_found&) {
                throw unrecognized_mod{"no \"select\" in \"move_antigens\" mod: " + args().to_json() };
            }
        }

}; // class ModMoveAntigens

// ----------------------------------------------------------------------

class ModMoveSera : public ModMoveBase
{
 public:
    using ModMoveBase::ModMoveBase;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            const auto verbose = args().get_or_default("report", false);
            try {
                const auto move_to = get_move_to(aChartDraw, verbose);
                for (auto index: SelectSera(verbose).select(aChartDraw.chart(), args()["select"])) {
                    aChartDraw.layout().set(index + aChartDraw.number_of_antigens(), move_to);
                }
            }
            catch (rjson::field_not_found&) {
                throw unrecognized_mod{"no \"select\" in \"move_sera\" mod: " + args().to_json() };
            }
        }

}; // class ModMoveSera

// ----------------------------------------------------------------------

class ModAminoAcids : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            const auto verbose = args().get_or_default("report", false);
            try {
                if (auto [pos_present, pos] = args().get_array_if("pos"); pos_present) {
                    aa_pos(aChartDraw, pos, verbose);
                }
                else if (auto [groups_present, groups] = args().get_array_if("groups"); groups_present) {
                    for (const auto& group: groups)
                        aa_group(aChartDraw, group, verbose);
                }
                else {
                    std::cerr << "No pos no groups" << '\n';
                    throw std::bad_variant_access{};
                }
            }
            catch (std::bad_variant_access&) {
                throw unrecognized_mod{"expected either \"pos\":[] or \"groups\":[] mod: " + args().to_json() };
            }
        }


 private:
    std::vector<Color> mColors;
    long mIndexDiff = 0;

    void aa_pos(ChartDraw& aChartDraw, const rjson::array& aPos, bool aVerbose)
        {
            const auto& seqdb = seqdb::get(do_report_time(aVerbose));
            const auto aa_indices = seqdb.aa_at_positions_for_antigens(aChartDraw.chart().antigens(), {std::begin(aPos), std::end(aPos)}, aVerbose);
            std::vector<std::string> aa_sorted(aa_indices.size()); // most frequent aa first
            std::transform(std::begin(aa_indices), std::end(aa_indices), std::begin(aa_sorted), [](const auto& entry) -> std::string { return entry.first; });
            std::sort(std::begin(aa_sorted), std::end(aa_sorted), [&aa_indices](const auto& n1, const auto& n2) -> bool { return aa_indices.find(n1)->second.size() > aa_indices.find(n2)->second.size(); });
            for (auto [index, aa]: acmacs::enumerate(aa_sorted)) {
                const auto& indices_for_aa = aa_indices.find(aa)->second;
                auto styl = style();
                styl.fill(fill_color(index, aa));
                aChartDraw.modify(std::begin(indices_for_aa), std::end(indices_for_aa), styl, drawing_order());
                try { add_legend(aChartDraw, indices_for_aa, styl, aa, args()["legend"]); } catch (rjson::field_not_found&) {}
                if (aVerbose)
                    std::cerr << "INFO: amino-acids at " << aPos << ": " << aa << ' ' << indices_for_aa.size() << '\n';
            }
        }

    void aa_group(ChartDraw& aChartDraw, const rjson::object& aGroup, bool aVerbose)
        {
            const rjson::array& pos_aa = aGroup["pos_aa"];
            std::vector<size_t> positions(pos_aa.size());
            std::transform(std::begin(pos_aa), std::end(pos_aa), std::begin(positions), [](const auto& src) { return std::stoul(src); });
            std::string target_aas(pos_aa.size(), ' ');
            std::transform(std::begin(pos_aa), std::end(pos_aa), std::begin(target_aas), [](const auto& src) { return static_cast<std::string>(src).back(); });
            const auto& seqdb = seqdb::get(do_report_time(aVerbose));
            const auto aa_indices = seqdb.aa_at_positions_for_antigens(aChartDraw.chart().antigens(), positions, aVerbose);
            if (const auto aap = aa_indices.find(target_aas); aap != aa_indices.end()) {
                auto styl = style();
                styl = point_style_from_json(aGroup);
                aChartDraw.modify(std::begin(aap->second), std::end(aap->second), styl, drawing_order());
                try { add_legend(aChartDraw, aap->second, styl, string::join(" ", std::begin(pos_aa), std::end(pos_aa)), args()["legend"]); } catch (rjson::field_not_found&) {}
                if (aVerbose)
                    std::cerr << "INFO: amino-acids group " << pos_aa << ": " << ' ' << aap->second.size() << '\n';
            }
            else {
                std::cerr << "WARNING: no \"" << target_aas << "\" in " << aa_indices << '\n';
            }
        }

    Color fill_color(long aIndex, std::string aAA)
        {
            if (aAA == "X") {
                --mIndexDiff;
                return args().get_or_default("X_color", "grey25");
            }
            if (mColors.empty()) {
                if (auto [colors_present, colors] = args().get_array_if("colors"); colors_present)
                    mColors.assign(std::begin(colors), std::end(colors));
                else
                    mColors = Color::distinct();
            }
            const auto index = static_cast<size_t>(aIndex + mIndexDiff);
            if (index < mColors.size())
                return mColors[index];
            else
                throw unrecognized_mod{"too few distinct colors in mod: " + args().to_json()};
        }

}; // class ModAminoAcids

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
                    const std::string direction = direction_v;
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
                    aChartDraw.viewport(abs[0], abs[1], abs[2]);
                }
                else if (auto [present_rel, rel] = args().get_array_if("rel"); present_rel) {
                    if (rel.size() != 3)
                        throw unrecognized_mod{"\"rel\" must be array of 3 floats. mod: " + args().to_json()};
                    aChartDraw.calculate_viewport(false);
                    const auto& orig_viewport = aChartDraw.viewport();
                    const auto new_size = static_cast<double>(rel[2]) + orig_viewport.size.width;
                    if (new_size < 1)
                        throw unrecognized_mod{"invalid size difference in \"rel\". mod: " + args().to_json()};
                    aChartDraw.viewport(orig_viewport.left() + static_cast<double>(rel[0]), orig_viewport.top() + static_cast<double>(rel[1]), new_size);
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
                aChartDraw.background_color(static_cast<std::string>(args()["color"]));
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
                aChartDraw.border(args().get_or_default("color", "black"), args().get_or_default("line_width", 1.0));
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
                aChartDraw.grid(args().get_or_default("color", "grey80"), args().get_or_default("line_width", 1.0));
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
                        for (std::string line: display_name)
                            title.add_line(line);
                    }
                    else {
                        title.add_line(aChartDraw.chart().make_name(aChartDraw.projection_no()));
                    }
                    if (auto [offset_present, offset] = args().get_array_if("offset"); offset_present)
                        title.offset(offset[0], offset[1]);
                    if (auto [padding_present, padding] = args().get_value_if("padding"); padding_present)
                        title.padding(padding);
                    if (auto [text_size_present, text_size] = args().get_value_if("text_size"); text_size_present)
                        title.text_size(text_size);
                    if (auto [text_color_present, text_color] = args().get_value_if("text_color"); text_color_present)
                        title.text_color(static_cast<std::string>(text_color));
                    if (auto [background_present, background] = args().get_value_if("background"); background_present)
                        title.background(static_cast<std::string>(background));
                    if (auto [border_color_present, border_color] = args().get_value_if("border_color"); border_color_present)
                        title.border_color(static_cast<std::string>(border_color));
                    if (auto [border_width_present, border_width] = args().get_value_if("border_width"); border_width_present)
                        title.border_width(border_width);
                    if (auto [font_weight_present, font_weight] = args().get_value_if("font_weight"); font_weight_present)
                        title.weight(static_cast<std::string>(font_weight));
                    if (auto [font_slant_present, font_slant] = args().get_value_if("font_slant"); font_slant_present)
                        title.slant(static_cast<std::string>(font_slant));
                    if (auto [font_family_present, font_family] = args().get_value_if("font_family"); font_family_present)
                        title.font_family(static_cast<std::string>(font_family));
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
                    auto& legend = aChartDraw.legend();
                    if (auto [data_present, data] = args().get_array_if("data"); data_present) {
                        for (const rjson::object& line_data: data)
                            legend.add_line(line_data.get_or_default("outline", "black"), line_data.get_or_default("fill", "pink"), line_data.get_or_default("display_name", "* no display_name *"));
                    }
                    if (auto [offset_present, offset] = args().get_array_if("offset"); offset_present)
                        legend.offset({offset[0], offset[1]});
                    if (auto [label_size_present, label_size] = args().get_value_if("label_size"); label_size_present)
                        legend.label_size(label_size);
                    if (auto [point_size_present, point_size] = args().get_value_if("point_size"); point_size_present)
                        legend.point_size(point_size);
                    if (auto [background_present, background] = args().get_value_if("background"); background_present)
                        legend.background(static_cast<std::string>(background));
                    if (auto [border_color_present, border_color] = args().get_value_if("border_color"); border_color_present)
                        legend.border_color(static_cast<std::string>(border_color));
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

class ModUseChartPlotSpec : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            try {
                const auto& plot_spec = aChartDraw.chart().plot_spec();
                for (size_t point_no = 0; point_no < aChartDraw.number_of_points(); ++point_no) {
                    const auto& source = plot_spec.style_for(point_no);
                    auto style{PointStyleEmpty()};
                    style.fill(source.fill_color())
                            .outline(source.outline_color())
                            .outline_width(Pixels{source.outline_width()})
                            .shape(source.shape_as_string())
                            .size(Pixels{source.size() * 5})
                            .rotation(source.rotation())
                            .aspect(source.aspect());
                    aChartDraw.modify(point_no, style);
                }
                aChartDraw.drawing_order() = plot_spec.drawing_order();
            }
            catch (rjson::field_not_found&) {
                throw unrecognized_mod{"mod: " + args().to_json()};
            }
        }

}; // class ModUseChartPlotSpec

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
                    line.color(args().get_or_default("color", "black"));
                    line.line_width(args().get_or_default("width", 1.0));
                }
            }
        }

 protected:
    std::vector<Location> begins_ends(ChartDraw& aChartDraw, std::string aPrefix) const
        {
            std::vector<Location> result;
            const auto verbose = args().get_or_default("report", false);
            if (auto [from_present, from] = args().get_array_if(aPrefix); from_present) {
                result.emplace_back(from[0], from[1]);
            }
            else if (auto [from_antigen_present, from_antigen] = args().get_object_if(aPrefix + "_antigen"); from_antigen_present) {
                for (auto index: SelectAntigens(verbose).select(aChartDraw.chart(), from_antigen)) {
                    const auto& coord = aChartDraw.layout()[index];
                    result.emplace_back(coord[0], coord[1]);
                }
            }
            else if (auto [from_serum_present, from_serum] = args().get_object_if(aPrefix + "_serum"); from_serum_present) {
                for (auto index: SelectSera(verbose).select(aChartDraw.chart(), from_serum)) {
                    const auto& coord = aChartDraw.layout()[index + aChartDraw.number_of_antigens()];
                    result.emplace_back(coord[0], coord[1]);
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
                    const std::string color = args().get_or_default("color", "black");
                    const std::string head_color = args().get_or_default("head_color", color);
                    arrow.color(color, head_color);
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
            const rjson::array& c1 = args()["corner1"];
            const rjson::array& c2 = args()["corner2"];
            auto& rectangle = aChartDraw.rectangle({c1[0], c1[1]}, {c2[0], c2[1]});
            rectangle.filled(args().get_or_default("filled", false));
            rectangle.color(args().get_or_default("color", "#80FF00FF"));
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
            circle.color(args().get_or_default("fill", "transparent"), args().get_or_default("outline", "#80FF00FF"));
            circle.outline_width(args().get_or_default("outline_width", 1.0));
            circle.aspect(Aspect{args().get_or_default("aspect", 1.0)});
            circle.rotation(Rotation{args().get_or_default("rotation", 0.0)});
        }

}; // class ModCircle

// ----------------------------------------------------------------------

class ModSerumHomologous : public Mod
{
 public:
    using Mod::Mod;

 protected:
    size_t select_mark_serum(ChartDraw& aChartDraw, bool aVerbose)
        {
            const size_t serum_index = select_serum(aChartDraw, aVerbose);
            if (auto [present, mark_serum] = args().get_object_if("mark_serum"); present) {
                aChartDraw.modify_serum(serum_index, point_style_from_json(mark_serum), drawing_order_from_json(mark_serum));
                try { add_label(aChartDraw, serum_index, aChartDraw.number_of_antigens(), mark_serum["label"]); } catch (rjson::field_not_found&) {}
            }
            return serum_index;
        }

    size_t select_serum(ChartDraw& aChartDraw, bool aVerbose) const
        {
            const auto sera = SelectSera(aVerbose).select(aChartDraw.chart(), args()["serum"]);
            if (sera.size() != 1)
                throw unrecognized_mod{"\"serum\" does not select single serum, mod: " + args().to_json()};
            return sera[0];
        }

    std::vector<size_t> select_mark_antigens(ChartDraw& aChartDraw, size_t aSerumIndex, bool aVerbose)
        {
            const auto antigen_indices = select_antigens(aChartDraw, aSerumIndex, aVerbose);
            if (auto [present, mark_antigen] = args().get_object_if("mark_antigen"); present) {
                aChartDraw.modify(std::begin(antigen_indices), std::end(antigen_indices), point_style_from_json(mark_antigen), drawing_order_from_json(mark_antigen));
                try { add_labels(aChartDraw, antigen_indices, 0, mark_antigen["label"]); } catch (rjson::field_not_found&) {}
            }
            return antigen_indices;
        }

    std::vector<size_t> select_antigens(ChartDraw& aChartDraw, size_t aSerumIndex, bool aVerbose) const
        {
            if (auto [antigen_present, antigen_select] = args().get_object_if("antigen"); antigen_present) {
                const auto antigens = SelectAntigens(aVerbose).select(aChartDraw.chart(), args()["antigen"]);
                if (antigens.empty())
                    throw unrecognized_mod{"\"antigen\" does not select an antigen, mod: " + args().to_json()};
                return antigens;
            }
            else {
                return select_homologous_antigens(aChartDraw, aSerumIndex, aVerbose);
            }
        }

    std::vector<size_t> select_homologous_antigens(ChartDraw& aChartDraw, size_t aSerumIndex, bool /*aVerbose*/) const
        {
            hidb::get(aChartDraw.chart().chart_info().virus_type()).find_homologous_antigens_for_sera_of_chart(aChartDraw.chart());
            const auto& antigens = aChartDraw.chart().serum(aSerumIndex).homologous();
            if (antigens.empty())
                throw unrecognized_mod{"no homologous antigens for serum, mod: " + args().to_json()};
            return antigens;
        }

}; // class ModSerumHomologous

// ----------------------------------------------------------------------

class ModSerumCircle : public ModSerumHomologous
{
 public:
    using ModSerumHomologous::ModSerumHomologous;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            const auto verbose = args().get_or_default("report", false);
            const size_t serum_index = select_mark_serum(aChartDraw, verbose);
            const auto antigen_indices = select_mark_antigens(aChartDraw, serum_index, verbose);
            make_serum_circle(aChartDraw, serum_index, Scaled{calculate_radius(aChartDraw, serum_index, antigen_indices, verbose)});
        }

 protected:
    void make_serum_circle(ChartDraw& aChartDraw, size_t aSerumIndex, Scaled aRadius) const
        {
            auto& circle = aChartDraw.serum_circle(aSerumIndex, aRadius);
            if (auto [present, circle_data] = args().get_object_if("circle"); present) {
                circle.fill(circle_data.get_or_default("fill", "transparent"));
                const auto outline = circle_data.get_or_default("outline", "pink");
                const auto outline_width = circle_data.get_or_default("outline_width", 1.0);
                circle.outline(outline, outline_width);
                if (auto [angles_present, angles] = circle_data.get_array_if("angle_degrees"); angles_present) {
                    const double pi_180 = std::acos(-1) / 180.0;
                    circle.angles(static_cast<double>(angles[0]) * pi_180, static_cast<double>(angles[1]) * pi_180);
                }
                const auto line_dash = circle_data.get_or_default("radius_line_dash", "");
                if (line_dash == "dash1")
                    circle.radius_line_dash1();
                else if (line_dash == "dash2")
                    circle.radius_line_dash2();
                else
                    circle.radius_line_no_dash();
                circle.radius_line(circle_data.get_or_default("radius_line_color", outline), circle_data.get_or_default("radius_line_width", outline_width));
            }
        }

    double calculate_radius(ChartDraw& aChartDraw, size_t aSerumIndex, const std::vector<size_t>& aAntigenIndices, bool aVerbose) const
        {
            std::vector<double> radii;
            std::transform(std::begin(aAntigenIndices), std::end(aAntigenIndices), std::back_inserter(radii), [&](size_t antigen_index) -> double { return aChartDraw.chart().serum_circle_radius(antigen_index, aSerumIndex, aChartDraw.projection_no(), false); });
            double radius = 0;
            for (auto rad: radii) {
                if (rad > 0 && (radius <= 0 || rad < radius))
                    radius = rad;
            }
            if (aVerbose) {
                std::cerr << "INFO: serum_circle radius: " << radius << "\n  SR " << aSerumIndex << ' ' << aChartDraw.chart().serum(aSerumIndex).full_name() << '\n';
                for (auto [no, antigen_index]: acmacs::enumerate(aAntigenIndices))
                    std::cerr << "    radius: " << radii[static_cast<size_t>(no)] << "  AG " << antigen_index << ' ' << aChartDraw.chart().antigen(antigen_index).full_name() << '\n';
            }
            return radius;
        }

}; // class ModSerumCircle

// ----------------------------------------------------------------------

class ModSerumCoverage : public ModSerumHomologous
{
 public:
    using ModSerumHomologous::ModSerumHomologous;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            const auto verbose = args().get_or_default("report", false);
            const size_t serum_index = select_mark_serum(aChartDraw, verbose);
            const auto antigen_indices = select_antigens(aChartDraw, serum_index, verbose);

            std::vector<size_t> within, outside;
            aChartDraw.chart().serum_coverage(antigen_indices[0], serum_index, within, outside);
            if (verbose) {
                std::cerr << "INFO: serum coverage\n  SR " << serum_index << ' ' << aChartDraw.chart().serum(serum_index).full_name()
                          << "\n  AG " << antigen_indices[0] << ' ' << aChartDraw.chart().antigen(antigen_indices[0]).full_name()
                          << "\n  within 4fold:  " << within.size()
                          << "\n  outside 4fold: " << outside.size() << '\n';
            }
            if (!within.empty()) {
                const auto& within_4fold = args().get_or_empty_object("within_4fold");
                aChartDraw.modify(std::begin(within), std::end(within), point_style_from_json(within_4fold), drawing_order_from_json(within_4fold));
            }
            if (!outside.empty()) {
                const auto& outside_4fold = args().get_or_empty_object("outside_4fold");
                aChartDraw.modify(std::begin(outside), std::end(outside), point_style_from_json(outside_4fold), drawing_order_from_json(outside_4fold));
            }
        }

}; // class ModSerumCoverage

// ----------------------------------------------------------------------

Mods factory(const rjson::value& aMod, const rjson::object& aSettingsMods, const rjson::object& aUpdate)
{
    std::string name;
    rjson::object args;
    if (auto ptr_obj = std::get_if<rjson::object>(&aMod)) {
        name = ptr_obj->get_or_default("N", "");
        if (name.empty()) {
            if (auto [present, val] = ptr_obj->get_value_if("?N"); present)
                name = "?";     // no "N" but "?N" present, avoid warning about commented out mode
        }
        args.update(*ptr_obj);
    }
    else if (auto ptr_str = std::get_if<rjson::string>(&aMod)) {
        name = *ptr_str;
    }
    args.update(aUpdate);

    Mods result;

    auto get_referenced_mod = [&aSettingsMods](std::string aName) -> const rjson::array& {
        try {
            return aSettingsMods.get_or_empty_array(aName);
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
    else if (name == "use_chart_plot_spec") {
        result.emplace_back(new ModUseChartPlotSpec(args));
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
    else if (const auto& referenced_mod = get_referenced_mod(name); !referenced_mod.empty()) {
        for (const auto& submod_desc: referenced_mod) {
            for (auto&& submod: factory(submod_desc, aSettingsMods, args)) {
                result.push_back(std::move(submod));
            }
        }
    }
    else if (name.empty()) {
        std::cerr << "WARNING: mod ignored (no \"N\"): " << args << '\n';
    }
    else if (name.front() == '?' || name.back() == '?') {
          // commented out
    }
    else {
        throw unrecognized_mod{"unrecognized mod: "s + aMod.to_json()};
    }

    return result;

} // factory

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
