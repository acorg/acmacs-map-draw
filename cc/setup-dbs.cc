#include "setup-dbs.hh"

#include "acmacs-base/acmacsd.hh"
#include "locationdb/locdb.hh"
#include "hidb-5/hidb.hh"
#include "seqdb-3/seqdb.hh"

// ----------------------------------------------------------------------

void setup_dbs(std::string_view /*aDbsDir*/, bool aVerbose)
{
    // const std::string db_dir = aDbsDir.empty() ? acmacs::acmacsd_root() + "/data" : std::string{aDbsDir};

    locdb_setup(acmacs::locdb_v2(), aVerbose);
    hidb::setup(acmacs::hidb_v5_dir(), {}, aVerbose);
    acmacs::seqdb::setup(acmacs::seqdb_v3_dir() + "/seqdb.json.xz");

} // setup_dbs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
