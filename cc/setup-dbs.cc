#include "setup-dbs.hh"

#include "acmacs-base/acmacsd.hh"
#include "locationdb/locdb.hh"
#include "hidb-5/hidb.hh"
#include "seqdb/seqdb.hh"

// ----------------------------------------------------------------------

void setup_dbs(std::string aDbsDir, bool aVerbose)
{
    if (aDbsDir.empty())
        aDbsDir = acmacs::acmacsd_root() + "/data";

    locdb_setup(aDbsDir + "/locationdb.json.xz", aVerbose);
    hidb::setup(aDbsDir, {}, aVerbose);
    seqdb::setup(aDbsDir + "/seqdb.json.xz", aVerbose);

} // setup_dbs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
