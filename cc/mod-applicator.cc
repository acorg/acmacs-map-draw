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
    void add_legend(ChartDraw& aChartDraw, const std::vector<size_t>& aIndices, const PointStyle& aStyle, const rjson::value& aLegendData);
    void add_legend(ChartDraw& aChartDraw, const std::vector<size_t>& aIndices, const PointStyle& aStyle, std::string aLabel, const rjson::value& aLegendData);

 private:
    const rjson::object& mArgs;

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

void Mod::add_labels(ChartDraw& aChartDraw, const std::vector<size_t>& aIndices, size_t aBaseIndex, const rjson::value& aLabelData)
{
    if (aLabelData.get_or_default("show", true)) {
        for (auto index: aIndices) {
            auto& label = aChartDraw.add_label(index + aBaseIndex);
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
                            label.display_name(aChartDraw.chart().antigen(index).abbreviated_name());
                        else if (name_type == "abbreviated_with_passage_type")
                            label.display_name(aChartDraw.chart().antigen(index).abbreviated_name_with_passage_type());
                        else {
                            if (name_type != "full")
                                std::cerr << "WARNING: unrecognized \"name_type\" for label for antigen " << index << '\n';
                            label.display_name(aChartDraw.chart().antigen(index).full_name());
                        }
                    }
                    else {      // serum
                        if (name_type == "abbreviated")
                            label.display_name(aChartDraw.chart().serum(index).abbreviated_name());
                        else if (name_type == "abbreviated_with_passage_type")
                            label.display_name(aChartDraw.chart().serum(index).abbreviated_name_with_passage_type());
                        else {
                            if (name_type != "full")
                                std::cerr << "WARNING: unrecognized \"name_type\" for label for serum " << index << '\n';
                            label.display_name(aChartDraw.chart().serum(index).full_name());
                        }
                    }
                }
                catch (rjson::field_not_found&) {
                }
            }
        }
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

Mods factory(const rjson::value& aMod, const rjson::object& aSettingsMods);

// ----------------------------------------------------------------------

void apply_mods(ChartDraw& aChartDraw, const rjson::array& aMods, const rjson::object& aModData, bool aIgnoreUnrecognized)
{
    const auto& mods_data_mod = aModData.get_or_empty_object("mods");
    for (const auto& mod_desc: aMods) {
        try {
            for (const auto& mod: factory(mod_desc, mods_data_mod)) {
                mod->apply(aChartDraw, aModData);
            }
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
            // std::cerr << "apply " << args() << '\n';
            const auto verbose = args().get_or_default("verbose", false) || args().get_or_default("report", false);
            try {
                const auto indices = SelectAntigens(verbose).select(aChartDraw.chart(), args()["select"]);
                const auto styl = style();
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
            const auto verbose = args().get_or_default("verbose", false) || args().get_or_default("report", false);
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

class ModAminoAcids : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& /*aModData*/) override
        {
            const auto verbose = args().get_or_default("verbose", false) || args().get_or_default("report", false);
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

Mods factory(const rjson::value& aMod, const rjson::object& aSettingsMods)
{
#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexit-time-destructors"
#endif
    static const rjson::object empty_args;
#pragma GCC diagnostic pop

    std::string name;
    const rjson::object* args = &empty_args;
    if (auto ptr_obj = std::get_if<rjson::object>(&aMod)) {
        name = ptr_obj->get_or_default("N", "");
        if (name.empty()) {
            if (auto [present, val] = ptr_obj->get_value_if("?N"); present)
                name = "?";     // no "N" but "?N" present, avoid warning about commented out mode
        }
        args = ptr_obj;
    }
    else if (auto ptr_str = std::get_if<rjson::string>(&aMod)) {
        name = *ptr_str;
    }

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
        result.emplace_back(new ModAntigens(*args));
    }
    else if (name == "sera") {
        result.emplace_back(new ModSera(*args));
    }
    else if (name == "amino-acids") {
        result.emplace_back(new ModAminoAcids(*args));
    }
    else if (name == "rotate") {
        result.emplace_back(new ModRotate(*args));
    }
    else if (const auto& referenced_mod = get_referenced_mod(name); !referenced_mod.empty()) {
        for (const auto& submod_desc: referenced_mod) {
            for (auto&& submod: factory(submod_desc, aSettingsMods))
                result.push_back(std::move(submod));
        }
    }
    else if (name.empty()) {
        std::cerr << "WARNING: mod ignored (no \"N\"): " << *args << '\n';
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
