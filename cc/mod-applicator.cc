#include <string>
using namespace std::string_literals;

#include "acmacs-base/enumerate.hh"
#include "seqdb/seqdb.hh"
#include "acmacs-map-draw/draw.hh"

#include "mod-applicator.hh"
#include "select.hh"

// ----------------------------------------------------------------------

class Mod
{
 public:
    inline Mod(const rjson::object& aArgs) : mArgs{aArgs} {}
    virtual inline ~Mod() {}

    virtual void apply(ChartDraw& aChartDraw, const rjson::object& aModData) = 0;

 protected:
    const rjson::object& args() const { return mArgs; }

 private:
    const rjson::object& mArgs;

}; // class Mod

using Mods = std::vector<std::unique_ptr<Mod>>;

Mods factory(const rjson::value& aMod, const rjson::object& aSettingsMods);

// ----------------------------------------------------------------------

void apply_mods(ChartDraw& aChartDraw, const rjson::array& aMods, const rjson::object& aModData, bool aIgnoreUnrecognized)
{
    for (const auto& mod_desc: aMods) {
        if (const auto mods = factory(mod_desc, aModData.get_field("mods", rjson::object{})); !mods.empty()) {
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

    void apply(ChartDraw& aChartDraw, const rjson::object& /*aModData*/) override
        {
            // std::cerr << "ModAntigens::apply " << &args() << '\n';
            // std::cerr << "ModAntigens::apply " << args().to_json() << '\n';
            const auto verbose = args().get_field("verbose", rjson::boolean{false});
            if (const auto* select_data = args()["select"]; select_data) {
                SelectAntigens select(verbose);
                const auto indices = select.command(aChartDraw.chart(), *select_data);
            }
            else
                throw unrecognized_mod{"no \"select\" in \"antigens\" mod: " /* + args().to_json() */};
        }

}; // class ModAntigens

// ----------------------------------------------------------------------

class Clades : public Mod
{
 public:
    using Mod::Mod;

    void apply(ChartDraw& aChartDraw, const rjson::object& aModData) override
        {
            const auto& seqdb = seqdb::get(report_time::Yes);
            std::vector<seqdb::SeqdbEntrySeq> seqdb_entries;
            seqdb.match(aChartDraw.chart().antigens(), seqdb_entries, true);

            const bool report = args().get_field("report", false);
            const auto num_digits = static_cast<int>(std::log10(aChartDraw.chart().number_of_antigens())) + 1;

            if (const auto* clade_fill = aModData["clade_fill"]; clade_fill) {
                for (auto [ag_no, entry_seq]: enumerate(seqdb_entries)) {
                    if (entry_seq) {
                        for (const auto& clade: entry_seq.seq().clades()) {
                            try {
                                const std::string fill = clade_fill->get_field<std::string>(clade);
                                aChartDraw.modify(ag_no, PointStyleEmpty().fill(fill).outline(BLACK), ChartDraw::Raise);
                                if (report)
                                    std::cout << "AG " << std::setfill(' ') << std::setw(num_digits) << ag_no << ' ' << aChartDraw.chart().antigen(static_cast<size_t>(ag_no)).full_name() << ' ' << clade << ' ' << fill << '\n';
                            }
                            catch (rjson::object::field_not_found&) {
                            }
                        }
                    }
                }
            }
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
        try {
            name = ptr_obj->get_field<std::string>("N");
            args = ptr_obj;
        }
        catch (rjson::object::field_not_found&) {
        }
    }
    else if (auto ptr_str = std::get_if<rjson::string>(&aMod)) {
        name = *ptr_str;
    }

    Mods result;

    // if (name == "all_grey" || name == "all-grey")
    //     return std::make_unique<AllGrey>();
    // else
    if (name == "antigens") {
        std::cerr << "antigens: " << args->to_json() << '\n';
        result.emplace_back(new ModAntigens(*args));
    }
    if (name == "clades") {
        result.emplace_back(new Clades(*args));
    }
    else if (const auto* referenced_mod = aSettingsMods[name]; referenced_mod) {
        for (const auto& submod_desc: static_cast<const rjson::array&>(*referenced_mod)) {
            for (auto&& submod: factory(submod_desc, aSettingsMods))
                result.push_back(std::move(submod));
        }
    }
    return result;

} // factory

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
