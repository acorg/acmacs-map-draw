# -*- Python -*-
# license
# license.
# ----------------------------------------------------------------------

import os
from pathlib import Path
from acmacs_base.timeit import timeit
from acmacs_chart_backend import HiDbSet

# ----------------------------------------------------------------------

def get_hidb(virus_type=None, chart=None, hidb_dir :Path = Path(os.environ["ACMACSD_ROOT"], "data")):
    global sHidbSet
    if sHidbSet is None:
        sHidbSet = HiDbSet(str(Path(hidb_dir).expanduser().resolve()))
    if chart is not None:
        virus_type = chart.chart_info().virus_type()
    with timeit("Getting hidb for " + virus_type):
        hidb = sHidbSet.get(virus_type)
    return hidb

# ----------------------------------------------------------------------

sHidbSet = None

# ----------------------------------------------------------------------
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
