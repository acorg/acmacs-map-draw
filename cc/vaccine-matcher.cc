//#include "draw.hh"
#include "vaccine-matcher.hh"

// ----------------------------------------------------------------------

VaccineMatcherLabel& VaccineMatcherLabel::name_type(std::string aNameType)
{
    for_each_with_vacc([&](const hidb::Vaccines::Entry& ve) {
        const auto& antigen = static_cast<const Antigen&>(mChartDraw.chart().antigen(ve.antigen_index)); // dynamic_cast throws here (clang 4.1)
        std::string name;
        if (aNameType == "abbreviated")
            name = antigen.abbreviated_name(mLocDb);
        else if (aNameType == "abbreviated_with_passage_type")
            name = antigen.abbreviated_name_with_passage_type(mLocDb);
        else if (aNameType == "abbreviated_location_with_passage_type")
            name = antigen.abbreviated_location_with_passage_type(mLocDb);
        else {
            if (aNameType != "full") {
                log_warning("unsupported name_type \"", aNameType, "\" (\"full\" is used)");
            }
            name = antigen.full_name();
        }
          // std::cerr << "DEBUG: name " << aNameType << ": \"" << name << '"' << std::endl;
        mChartDraw.add_label(ve.antigen_index).display_name(name);
    });
    return *this;

} // VaccineMatcherLabel::name_type

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
