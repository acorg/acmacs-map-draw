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

    virtual void apply(ChartDraw& aChartDraw, const rjson::object& aModData) = 0;

}; // class Mod

std::unique_ptr<Mod> factory(const rjson::value& aMod);

// ----------------------------------------------------------------------

void apply_mods(ChartDraw& aChartDraw, const rjson::array& aMods, const rjson::object& aModData, bool aIgnoreUnrecognized)
{
    for (const auto& mod_desc: aMods) {
        if (auto mod = factory(mod_desc))
            mod->apply(aChartDraw, aModData);
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

    void apply(ChartDraw& aChartDraw, const rjson::object& /*aModData*/) override
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

    void apply(ChartDraw& aChartDraw, const rjson::object& aModData) override
        {
            const auto& seqdb = seqdb::get(report_time::Yes);
            std::vector<seqdb::SeqdbEntrySeq> seqdb_entries;
            seqdb.match(aChartDraw.chart().antigens(), seqdb_entries, true);

            const bool report = mArgs.get_field("report", false);
            const auto num_digits = static_cast<int>(std::log10(aChartDraw.chart().number_of_antigens())) + 1;

            const auto& clade_fill = aModData["clade_fill"];
            for (auto [ag_no, entry_seq]: enumerate(seqdb_entries)) {
                if (entry_seq) {
                    for (const auto& clade: entry_seq.seq().clades()) {
                        try {
                            const std::string fill = clade_fill.get_field<std::string>(clade);
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
