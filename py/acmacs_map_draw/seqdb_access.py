# -*- Python -*-
# license
# license.
# ----------------------------------------------------------------------

import os
from pathlib import Path
import logging; module_logger = logging.getLogger(__name__)
from acmacs_base.timeit import timeit
from acmacs_chart_backend import Seqdb

# ----------------------------------------------------------------------

def match(chart):
    seqdb = get_seqdb()
    with timeit("Matching seqdb"):
        matched = chart.antigens().match_seqdb(seqdb, verbose=False)
        module_logger.info('{} antigens matched against seqdb'.format(matched))

# ----------------------------------------------------------------------

def get_seqdb(seqdb_file :Path = Path(os.environ["ACMACSD_ROOT"], "data", "seqdb.json.xz")):
    global sSeqdb
    if sSeqdb is None:
        filename = str(Path(seqdb_file).expanduser().resolve())
        with timeit("Loading seqdb from " + filename):
            sSeqdb = Seqdb()
            sSeqdb.load(filename)
        with timeit("Building hi name index"):
            sSeqdb.build_hi_name_index()
    return sSeqdb

# ----------------------------------------------------------------------

sSeqdb = None

# ----------------------------------------------------------------------
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
