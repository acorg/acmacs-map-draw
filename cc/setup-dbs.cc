#include "setup-dbs.hh"

#include "locationdb/locdb.hh"

// ----------------------------------------------------------------------

void setup_dbs(std::string aDbsDir)
{
    if (!aDbsDir.empty()) {
        locdb_setup(aDbsDir + "/locationdb.json.xz");
    }

} // setup_dbs

// ----------------------------------------------------------------------
/// Local Variables:
/// eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
/// End:
