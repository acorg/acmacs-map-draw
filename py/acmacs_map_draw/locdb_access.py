# -*- Python -*-
# license
# license.
# ----------------------------------------------------------------------

import os
from pathlib import Path
import logging; module_logger = logging.getLogger(__name__)

# ----------------------------------------------------------------------

sLocDb = None

def get_locdb(locdb_file :Path = Path(os.environ["ACMACSD_ROOT"], "data", "locationdb.json.xz")):
    from acmacs_base.timeit import timeit
    global sLocDb
    if sLocDb is None:
        with timeit("Loading locationdb from " + str(locdb_file)):
            sLocDb = LocDb()
            sLocDb.import_from(str(locdb_file))
    return sLocDb

# ----------------------------------------------------------------------
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
