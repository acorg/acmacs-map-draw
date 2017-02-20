# -*- Python -*-
# license
# license.
# ----------------------------------------------------------------------

import os
from pathlib import Path
import logging; module_logger = logging.getLogger(__name__)
from acmacs_base.timeit import timeit
from acmacs_map_draw_backend import Seqdb

# ----------------------------------------------------------------------

def match(chart, verbose=False):
    seqdb = get_seqdb()
    with timeit("Matching seqdb"):
        per_antigen = seqdb.match_antigens(antigens=chart.antigens(), verbose=verbose)
        module_logger.info('{} antigens matched against seqdb'.format(sum(1 for e in per_antigen if e)))
    return per_antigen

# ----------------------------------------------------------------------

def antigen_clades(chart, verbose=False):
    clade_data = {}
    for ag_no, antigen_seq in enumerate(match(chart, verbose=verbose)):
        if antigen_seq:
            clades = antigen_seq.seq.clades()
            if clades:
                for clade in clades:
                    clade_data.setdefault(clade, []).append(ag_no)
            else:
                clade_data.setdefault("", []).append(ag_no)   # no clades but sequenced (for B/Vic)
    return clade_data

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
