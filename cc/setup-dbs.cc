#include "setup-dbs.hh"

#include "acmacs-base/acmacsd.hh"
#include "locationdb/locdb.hh"
#include "hidb-5/hidb.hh"
#include "seqdb-3/seqdb.hh"

// ----------------------------------------------------------------------

void setup_dbs(std::string_view aDbsDir, bool aVerbose)
{
    const std::string db_dir = aDbsDir.empty() ? acmacs::acmacsd_root() + "/data" : std::string{aDbsDir};

    locdb_setup(db_dir + "/locationdb.json.xz", aVerbose);
    hidb::setup(db_dir, {}, aVerbose);
    acmacs::seqdb::setup(db_dir + "/seqdb.json.xz");

} // setup_dbs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
