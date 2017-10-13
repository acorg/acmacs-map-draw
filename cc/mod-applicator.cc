#include <string>
using namespace std::string_literals;

#include "acmacs-base/enumerate.hh"
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

 private:
    const rjson::object& mArgs;

    friend inline std::ostream& operator << (std::ostream& out, const Mod& aMod) { return out << aMod.args(); }

}; // class Mod

using Mods = std::vector<std::unique_ptr<Mod>>;

inline std::ostream& operator << (std::ostream& out, const Mods& aMods)
{
    out << "[\n";
    for (const auto& mod: aMods)
        out << "  " << *mod << '\n';
    out << ']';
    return out;
}

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

Mods factory(const rjson::value& aMod, const rjson::object& aSettingsMods);

// ----------------------------------------------------------------------

void apply_mods(ChartDraw& aChartDraw, const rjson::array& aMods, const rjson::object& aModData, bool aIgnoreUnrecognized)
{
    const auto& mods_data_mod = aModData.get_or_empty_object("mods");
    for (const auto& mod_desc: aMods) {
        if (const auto mods = factory(mod_desc, mods_data_mod); !mods.empty()) {
            for (const auto& mod: mods) {
                mod->apply(aChartDraw, aModData);
            }
        }
        else if (aIgnoreUnrecognized)
            std::cerr << "WARNING: unrecognized mod: " << mod_desc << '\n';
        else
            throw unrecognized_mod{"unrecognized mod: "s + mod_desc.to_json()};
    }

} // apply_mods

// ----------------------------------------------------------------------

// class AllGrey : public Mod
// {
//  public:
//     inline AllGrey() = default;

//     void apply(ChartDraw& aChartDraw, const rjson::object& /*aModData*/) override
//         {
//             for (auto [ag_no, antigen]: enumerate(aChartDraw.chart().antigens())) {
//                 aChartDraw.modify(ag_no, PointStyleEmpty().fill(antigen.reference() ? TRANSPARENT : GREY).outline(GREY));
//             }
//             for (auto [sr_no, serum]: enumerate(aChartDraw.chart().sera())) {
//                 aChartDraw.modify_serum(sr_no, PointStyleEmpty().fill(TRANSPARENT).outline(GREY));
//             }
//         }

// }; // class AllGrey

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
                aChartDraw.modify(indices.begin(), indices.end(), style(), drawing_order());
                try { add_labels(aChartDraw, indices, 0, args()["label"]); } catch (rjson::field_not_found&) {}
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
            // std::cerr << "apply " << args() << '\n';
            const auto verbose = args().get_or_default("verbose", false) || args().get_or_default("report", false);
            try {
                const auto indices = SelectSera(verbose).select(aChartDraw.chart(), args()["select"]);
                aChartDraw.modify_sera(indices.begin(), indices.end(), style(), drawing_order());
                try { add_labels(aChartDraw, indices, aChartDraw.number_of_antigens(), args()["label"]); } catch (rjson::field_not_found&) {}
            }
            catch (rjson::field_not_found&) {
                throw unrecognized_mod{"no \"select\" in \"sera\" mod: " + args().to_json() };
            }
        }

}; // class ModSera

// ----------------------------------------------------------------------

// class Clades : public Mod
// {
//  public:
//     using Mod::Mod;

//     void apply(ChartDraw& aChartDraw, const rjson::value& aModData) override
//         {
//             const auto& seqdb = seqdb::get(report_time::Yes);
//             std::vector<seqdb::SeqdbEntrySeq> seqdb_entries;
//             seqdb.match(aChartDraw.chart().antigens(), seqdb_entries, true);

//             const bool report = args().get_or_default("report", false);
//             const auto num_digits = static_cast<int>(std::log10(aChartDraw.chart().number_of_antigens())) + 1;

//             const std::string fill = aModData.get_or_default("clade_fill", "pink");
//             for (auto [ag_no, entry_seq]: enumerate(seqdb_entries)) {
//                 if (entry_seq) {
//                     for (const auto& clade: entry_seq.seq().clades()) {
//                         try {
//                               // const std::string fill = clade_fill.get_field<std::string>(clade);
//                             aChartDraw.modify(ag_no, PointStyleEmpty().fill(fill).outline(BLACK), PointDrawingOrder::Raise);
//                             if (report)
//                                 std::cout << "AG " << std::setfill(' ') << std::setw(num_digits) << ag_no << ' ' << aChartDraw.chart().antigen(static_cast<size_t>(ag_no)).full_name() << ' ' << clade << ' ' << fill << '\n';
//                         }
//                         catch (rjson::field_not_found&) {
//                         }
//                     }
//                 }
//             }
//               //}
//         }

// }; // class Clades

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
    // else if (name == "clades") {
    //     result.emplace_back(new Clades(*args));
    // }
    else if (const auto& referenced_mod = get_referenced_mod(name); !referenced_mod.empty()) {
        for (const auto& submod_desc: referenced_mod) {
            for (auto&& submod: factory(submod_desc, aSettingsMods))
                result.push_back(std::move(submod));
        }
    }
    else if (name.empty()) {
        std::cerr << "WARNING: mod ignored (no \"N\"): " << *args << '\n';
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
