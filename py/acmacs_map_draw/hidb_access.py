# -*- Python -*-
# license
# license.
# ----------------------------------------------------------------------

import os
from pathlib import Path
from acmacs_base.timeit import timeit
from acmacs_map_draw_backend import HiDbSet

# ----------------------------------------------------------------------

sVirusTypeNormalizer = {
    "h3": "A(H3N2)", "H3": "A(H3N2)", "A(H3N2)": "A(H3N2)",
    "h1": "A(H1N1)", "H1": "A(H1N1)", "A(H1N1)": "A(H1N1)",
    "b": "B", "B": "B",
    }

def get_hidb(virus_type=None, chart=None, hidb_dir :Path = None):
    global sHidbSet
    if sHidbSet is None:
        sHidbSet = HiDbSet(str(Path(hidb_dir or Path(os.environ["ACMACSD_ROOT"], "data")).expanduser().resolve()))
    if chart is not None:
        virus_type = chart.chart_info().virus_type()
    virus_type  = sVirusTypeNormalizer[virus_type]
    with timeit("Getting hidb for " + virus_type):
        hidb = sHidbSet.get(virus_type)
    return hidb

# ----------------------------------------------------------------------

sHidbSet = None

# ----------------------------------------------------------------------
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
