#include <iostream>

#include "acmacs-base/argc-argv.hh"
#include "acmacs-base/enumerate.hh"
#include "acmacs-base/string.hh"
#include "acmacs-base/virus-name.hh"
#include "acmacs-base/csv.hh"
#include "seqdb/seqdb.hh"
#include "locationdb/locdb.hh"
#include "acmacs-chart-2/factory-import.hh"
#include "acmacs-chart-2/chart.hh"

static void write_csv(std::string aFilename, std::shared_ptr<acmacs::chart::Antigens> antigens, std::shared_ptr<acmacs::chart::Sera> sera, std::shared_ptr<acmacs::chart::Layout> layout, const std::vector<seqdb::SeqdbEntrySeq>& entry_seqs, const LocDb& locdb);

// ----------------------------------------------------------------------

int main(int argc, char* const argv[])
{
    int exit_code = 0;
    try {
        argc_argv args(argc, argv, {
                {"--projection", 0L},
                {"--time", false, "report time of loading chart"},
                {"--verbose", false},
                {"-h", false},
                {"--help", false},
                {"-v", false},
        });
        if (args["-h"] || args["--help"] || args.number_of_arguments() != 2) {
            std::cerr << "Usage: " << args.program() << " [options] <chart-file> <output.csv[.xz]>\n" << args.usage_options() << '\n';
            exit_code = 1;
        }
        else {
            const auto report_time = do_report_time(args["--time"]);
            auto chart = acmacs::chart::import_from_file(args[0], acmacs::chart::Verify::None, report_time);
            auto antigens = chart->antigens();
            auto sera = chart->sera();
            auto layout = chart->projection(args["--projection"])->layout();
            const auto& seqdb = seqdb::get(seqdb::ignore_errors::no, report_time);
            const auto& locdb = get_locdb(report_time);
            const auto entry_seqs = seqdb.match(*antigens, chart->info()->virus_type(acmacs::chart::Info::Compute::Yes)); // entry for each antigen
            const std::string filename = args[1];
            if (filename == "-")
                write_csv(filename, antigens, sera, layout, entry_seqs, locdb);
            else
                write_csv(filename, antigens, sera, layout, entry_seqs, locdb);
        }
    }
    catch (std::exception& err) {
        std::cerr << "ERROR: " << err.what() << '\n';
        exit_code = 2;
    }
    return exit_code;
}

// ----------------------------------------------------------------------

void write_csv(std::string aFilename, std::shared_ptr<acmacs::chart::Antigens> antigens, std::shared_ptr<acmacs::chart::Sera> sera, std::shared_ptr<acmacs::chart::Layout> layout,
               const std::vector<seqdb::SeqdbEntrySeq>& entry_seqs, const LocDb& locdb)
{
    const auto number_of_dimensions = layout->number_of_dimensions();
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
        const auto location = locdb.find(virus_name::location(name));
        writer.add_field(location.country());
        writer.add_field(locdb.continent_of_country(location.country()));
        writer.add_field(std::to_string(location.latitude()));
        writer.add_field(std::to_string(location.longitude()));
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

    acmacs::file::write(aFilename, writer);

} // write_csv

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
