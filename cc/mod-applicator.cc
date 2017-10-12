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
            }
            catch (rjson::field_not_found&) {
                throw unrecognized_mod{"no \"select\" in \"sera\" mod: " + args().to_json() };
            }
        }

}; // class ModSera

// ----------------------------------------------------------------------

class Clades : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::value& aModData) override
        {
            const auto& seqdb = seqdb::get(report_time::Yes);
            std::vector<seqdb::SeqdbEntrySeq> seqdb_entries;
            seqdb.match(aChartDraw.chart().antigens(), seqdb_entries, true);

            const bool report = args().get_or_default("report", false);
            const auto num_digits = static_cast<int>(std::log10(aChartDraw.chart().number_of_antigens())) + 1;

            const std::string fill = aModData.get_or_default("clade_fill", "pink");
            for (auto [ag_no, entry_seq]: enumerate(seqdb_entries)) {
                if (entry_seq) {
                    for (const auto& clade: entry_seq.seq().clades()) {
                        try {
                              // const std::string fill = clade_fill.get_field<std::string>(clade);
                            aChartDraw.modify(ag_no, PointStyleEmpty().fill(fill).outline(BLACK), PointDrawingOrder::Raise);
                            if (report)
                                std::cout << "AG " << std::setfill(' ') << std::setw(num_digits) << ag_no << ' ' << aChartDraw.chart().antigen(static_cast<size_t>(ag_no)).full_name() << ' ' << clade << ' ' << fill << '\n';
                        }
                        catch (rjson::field_not_found&) {
                        }
                    }
                }
            }
              //}
        }

}; // class Clades

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
    else if (name == "clades") {
        result.emplace_back(new Clades(*args));
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
