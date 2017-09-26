#include <string>
using namespace std::string_literals;

#include "acmacs-base/enumerate.hh"
#include "seqdb/seqdb.hh"
#include "acmacs-map-draw/draw.hh"

#include "mod-applicator.hh"

// ----------------------------------------------------------------------

class Mod
{
 public:
    virtual inline ~Mod() {}

    virtual void apply(ChartDraw& aChartDraw) = 0;

}; // class Mod

std::unique_ptr<Mod> factory(const rjson::value& aMod);

// ----------------------------------------------------------------------

void apply_mods(ChartDraw& aChartDraw, const rjson::array& aMods, const rjson::object& aModDescription, bool aIgnoreUnrecognized)
{
    for (const auto& mod_desc: aMods) {
        if (auto mod = factory(mod_desc))
            mod->apply(aChartDraw);
        else if (aIgnoreUnrecognized)
            std::cerr << "WARNING: unrecognized mod: " << mod_desc << '\n';
        else
            throw unrecognized_mod{"unrecognized mod: "s + mod_desc.to_json()};
    }

} // apply_mods

// ----------------------------------------------------------------------

class AllGrey : public Mod
{
 public:
    inline AllGrey() = default;

    void apply(ChartDraw& aChartDraw) override
        {
            for (auto [ag_no, antigen]: enumerate(aChartDraw.chart().antigens())) {
                aChartDraw.modify(ag_no, PointStyleEmpty().fill(antigen.reference() ? TRANSPARENT : GREY).outline(GREY));
            }
            for (auto [sr_no, serum]: enumerate(aChartDraw.chart().sera())) {
                aChartDraw.modify_serum(sr_no, PointStyleEmpty().fill(TRANSPARENT).outline(GREY));
            }
        }

}; // class AllGrey

// ----------------------------------------------------------------------

class Clades : public Mod
{
 public:
    inline Clades(const rjson::object& aArgs) : mArgs{aArgs} {}

    void apply(ChartDraw& aChartDraw) override
        {
            const std::map<std::string, const char*> CladeColor = {
                {"", "grey50"},
                  // H3
                {"3C3", "cornflowerblue"},
                {"3C2a", "red"},
                {"3C2a1", "darkred"},
                {"3C3a", "green"},
                {"3C3b", "blue"},
                  // H1pdm
                {"6B1", "blue"},
                {"6B2", "red"},
                  // B/Yam
                {"Y2", "cornflowerblue"},
                {"Y3", "red"},
                  // B/Vic
                {"1", "blue"},
                {"1A", "cornflowerblue"},
                {"1B", "red"},
            };

            const auto& seqdb = seqdb::get(mArgs.get_field("seqdb_file", "/Users/eu/AD/data/seqdb.json.xz"s), true);
            std::vector<seqdb::SeqdbEntrySeq> seqdb_entries;
            seqdb.match(aChartDraw.chart().antigens(), seqdb_entries, true);

            for (auto [ag_no, entry_seq]: enumerate(seqdb_entries)) {
                if (entry_seq) {
                    if (const auto& clades = entry_seq.seq().clades(); !clades.empty()) {
                        if (const auto clr = CladeColor.find(clades.front()); clr != CladeColor.end())
                            aChartDraw.modify(ag_no, PointStyleEmpty().fill(clr->second).outline(BLACK), ChartDraw::Raise);
                    }
                }
            }
              // for (auto [ag_no, antigen]: enumerate(aChartDraw.chart().antigens())) {
              //     aChartDraw.modify(ag_no, PointStyleEmpty().fill(antigen.reference() ? TRANSPARENT : GREY).outline(GREY));
              // }
              // for (auto [sr_no, serum]: enumerate(aChartDraw.chart().sera())) {
              //     aChartDraw.modify_serum(sr_no, PointStyleEmpty().fill(TRANSPARENT).outline(GREY));
              // }
        }

 private:
    const rjson::object& mArgs;

}; // class AllGrey

// ----------------------------------------------------------------------

std::unique_ptr<Mod> factory(const rjson::value& aMod)
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

    if (name == "all_grey" || name == "all-grey")
        return std::make_unique<AllGrey>();
    else if (name == "clades")
        return std::make_unique<Clades>(*args);
    return nullptr;

} // factory

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
