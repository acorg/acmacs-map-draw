#include "acmacs-base/virus-name.hh"
#include "acmacs-base/csv.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/enumerate.hh"
#include "seqdb/seqdb.hh"
#include "locationdb/locdb.hh"
#include "acmacs-map-draw/export.hh"

// ----------------------------------------------------------------------

std::string export_layout_sequences_into_csv(std::string filename, const acmacs::chart::Chart& chart, size_t projection_no)
{
    auto antigens = chart.antigens();
    auto sera = chart.sera();
    auto layout = chart.projection(projection_no)->layout();
    const auto number_of_dimensions = layout->number_of_dimensions();
    const auto& seqdb = seqdb::get(seqdb::ignore_errors::no, report_time::no);
    const auto& locdb = get_locdb();
    const auto entry_seqs = seqdb.match(*antigens, chart.info()->virus_type(acmacs::chart::Info::Compute::Yes)); // entry for each antigen

    acmacs::CsvWriter writer;

    writer.add_field("AG/SR");
    writer.add_field("name");
    writer.add_field("x");
    if (number_of_dimensions > 1) {
        writer.add_field("y");
        if (number_of_dimensions > 2)
            writer.add_field("z");
    }
    writer.add_field("date");
    writer.add_field("reassortant");
    writer.add_field("annotations");
    writer.add_field("passage/serum_id");
    writer.add_field("country");
    writer.add_field("region");
    writer.add_field("latitude");
    writer.add_field("longitude");
    writer.add_field("sequence");
    writer.new_row();

    auto add_location_data = [&writer, &locdb](std::string name) {
        try {
            const auto location = locdb.find_for_virus_name(name);
            writer.add_field(location.country());
            writer.add_field(locdb.continent_of_country(location.country()));
            writer.add_field(std::to_string(location.latitude()));
            writer.add_field(std::to_string(location.longitude()));
        }
        catch (LocationNotFound& /*err*/) {
            std::cerr << "WARNING: LocationNotFound: \"" << virus_name::location(name) << "\" of \"" << name << "\"\n";
        }
        catch (virus_name::Unrecognized& /*err*/) {
            std::cerr << "WARNING: cannot parse name to find location: \"" << name << "\"\n";
        }
    };

    for (auto [ag_no, antigen] : acmacs::enumerate(*antigens)) {
        writer.add_field("AG");
        writer.add_field(antigen->name());
        for (size_t dim = 0; dim < number_of_dimensions; ++dim) {
            if (const auto coord = layout->coordinate(ag_no, dim); !std::isnan(coord))
                writer.add_field(acmacs::to_string(coord));
            else
                writer.add_field("");
        }
        writer.add_field(antigen->date());
        writer.add_field(antigen->reassortant());
        writer.add_field(antigen->annotations().join());
        writer.add_field(antigen->passage());
        add_location_data(antigen->name());
        if (const auto& entry_seq = entry_seqs[ag_no]; entry_seq) {
            writer.add_field(entry_seq.seq().amino_acids(true));
        }
        else
            writer.add_field("");
        writer.new_row();
    }

    const auto number_of_antigens = antigens->size();
    for (auto [sr_no, serum] : acmacs::enumerate(*sera)) {
        writer.add_field("SR");
        writer.add_field(serum->name());
        for (size_t dim = 0; dim < number_of_dimensions; ++dim) {
            if (const auto coord = layout->coordinate(sr_no + number_of_antigens, dim); !std::isnan(coord))
                writer.add_field(acmacs::to_string(coord));
            else
                writer.add_field("");
        }
        writer.add_field(""); // date
        writer.add_field(serum->reassortant());
        writer.add_field(serum->annotations().join());
        writer.add_field(serum->serum_id());
        add_location_data(serum->name());
        writer.add_field(""); // sequence
        writer.new_row();
    }

    if (!filename.empty())
        acmacs::file::write(filename, writer);

    return writer;

} // export_layout_sequences_into_csv

// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
