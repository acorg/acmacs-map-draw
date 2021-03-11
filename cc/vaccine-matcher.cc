#include "acmacs-map-draw/vaccine-matcher.hh"

// ----------------------------------------------------------------------

VaccineMatcherLabel& VaccineMatcherLabel::name_type(std::string aNameType)
{
    for_each_with_vacc([&](const hidb::Vaccines::Entry& ve) {
        auto antigen = mChartDraw.chart().antigen(ve.chart_antigen_index);
        std::string name;
        if (aNameType == "abbreviated")
            name = antigen->format("{name_abbreviated}");
        else if (aNameType == "abbreviated_with_passage_type")
            name = antigen->format("{name_abbreviated}-{passage_type}");
        else if (aNameType == "abbreviated_location_with_passage_type")
            name = antigen->format("{abbreviated_location_with_passage_type}");
        else {
            if (aNameType != "full")
                AD_WARNING("unsupported name_type \"{}\" (\"full\" is used)", aNameType);
            name = antigen->name_full();
        }
          // std::cerr << "DEBUG: name " << aNameType << ": \"" << name << '"' << std::endl;
        mChartDraw.add_label(ve.chart_antigen_index).display_name(name);
    });
    return *this;

} // VaccineMatcherLabel::name_type

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
