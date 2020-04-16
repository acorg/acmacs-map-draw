#include "acmacs-base/fmt.hh"
#include "acmacs-base/csv.hh"
#include "acmacs-base/read-file.hh"
#include "acmacs-base/enumerate.hh"
#include "acmacs-virus/virus-name-v1.hh"
#include "seqdb-3/seqdb.hh"
#include "locationdb/locdb.hh"
#include "acmacs-map-draw/export.hh"

// ----------------------------------------------------------------------

std::string export_layout_sequences_into_csv(std::string_view filename, const acmacs::chart::Chart& chart, size_t projection_no)
{
    auto antigens = chart.antigens();
    auto sera = chart.sera();
    auto layout = chart.projection(projection_no)->layout();
    const auto number_of_dimensions = layout->number_of_dimensions();
    const auto& seqdb = acmacs::seqdb::get();
    const auto entry_seqs = seqdb.match(*antigens, chart.info()->virus_type(acmacs::chart::Info::Compute::Yes)); // entry for each antigen

    acmacs::CsvWriter writer;

    writer.add_field("AG/SR");
    writer.add_field("name");
    writer.add_field("x");
    if (number_of_dimensions > acmacs::number_of_dimensions_t{1}) {
        writer.add_field("y");
        if (number_of_dimensions > acmacs::number_of_dimensions_t{2})
            writer.add_field("z");
    }
    writer.add_field("date");
    writer.add_field("reassortant");
    writer.add_field("annotations");
    writer.add_field("passage/serum_id");
    writer.add_field("lab_id");
    writer.add_field("country");
    writer.add_field("region");
    writer.add_field("latitude");
    writer.add_field("longitude");
    writer.add_field("sequence");
    writer.new_row();

    auto add_location_data = [&writer](std::string name) {
        try {
            const auto find_for_virus_name = [](std::string aVirusName) {
                if (const auto loc = acmacs::locationdb::get().find(::virus_name::location(aVirusName), acmacs::locationdb::include_continent::yes); loc.has_value())
                    return loc;
                else
                    return acmacs::locationdb::get().find(::virus_name::location_for_cdc_name(aVirusName), acmacs::locationdb::include_continent::yes);
            };

            if (const auto location = find_for_virus_name(name); location.has_value()) {
                writer.add_field(std::string{location->country()});
                writer.add_field(location->continent);
                writer.add_field(std::to_string(location->latitude()));
                writer.add_field(std::to_string(location->longitude()));
            }
            else
                AD_WARNING("LocationNotFound: \"{}\" of \"{}\"", virus_name::location(name), name);
        }
        catch (virus_name::Unrecognized& /*err*/) {
            AD_WARNING("cannot parse name to find location: \"{}\"", name);
        }
    };

    for (auto [ag_no, antigen] : acmacs::enumerate(*antigens)) {
        // fmt::print(stderr, "DEBUG: {} {}\n", ag_no, antigen->name());
        writer.add_field("AG");
        writer.add_field(*antigen->name());
        for (auto dim : acmacs::range(number_of_dimensions)) {
            if (const auto coord = layout->coordinate(ag_no, dim); !std::isnan(coord))
                writer.add_field(acmacs::to_string(coord));
            else
                writer.add_field("");
        }
        writer.add_field(*antigen->date());
        writer.add_field(*antigen->reassortant());
        writer.add_field(antigen->annotations().join());
        writer.add_field(*antigen->passage());
        writer.add_field(antigen->lab_ids().join());
        add_location_data(*antigen->name());
        if (const auto& entry_seq = entry_seqs[ag_no]; entry_seq) {
            try {
                writer.add_field(std::string{entry_seq.aa_aligned(seqdb)});
            }
            catch (std::exception& err) {
                fmt::print(stderr, "WARNING: {} {}: {}\n", ag_no, antigen->full_name(), err);
                writer.add_field("");
            }
        }
        else
            writer.add_field("");
        writer.new_row();
    }

    const auto number_of_antigens = antigens->size();
    for (auto [sr_no, serum] : acmacs::enumerate(*sera)) {
        writer.add_field("SR");
        writer.add_field(*serum->name());
        for (auto dim : acmacs::range(number_of_dimensions)) {
            if (const auto coord = layout->coordinate(sr_no + number_of_antigens, dim); !std::isnan(coord))
                writer.add_field(acmacs::to_string(coord));
            else
                writer.add_field("");
        }
        writer.add_field(""); // date
        writer.add_field(*serum->reassortant());
        writer.add_field(serum->annotations().join());
        writer.add_field(*serum->serum_id());
        writer.add_field(""); // lab_id
        add_location_data(*serum->name());
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
