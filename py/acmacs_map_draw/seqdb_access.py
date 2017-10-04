# -*- Python -*-
# license
# license.
# ----------------------------------------------------------------------

import os, pprint
from pathlib import Path
import logging; module_logger = logging.getLogger(__name__)
from acmacs_base.timeit import timeit
import seqdb_backend

# ----------------------------------------------------------------------

def match(chart, seqdb_file=None, verbose=False):
    seqdb = seqdb_backend.get_seqdb(timer=True)
    with timeit("Matching seqdb"):
        per_antigen = seqdb.match_antigens(antigens=chart.antigens(), verbose=verbose)
        module_logger.info('{} antigens matched against seqdb'.format(sum(1 for e in per_antigen if e)))
    return per_antigen

# ----------------------------------------------------------------------

def sequenced(chart, seqdb_file=None, verbose=False):
    r = [ag_no for ag_no, entry_seq in enumerate(match(chart, seqdb_file=seqdb_file, verbose=verbose)) if entry_seq]
    return r

# ----------------------------------------------------------------------

def not_sequenced(chart, seqdb_file=None, verbose=False):
    """Returns list of antigen indices for antigens not found in seqdb"""
    r = [ag_no for ag_no, entry_seq in enumerate(match(chart, seqdb_file=seqdb_file, verbose=verbose)) if not entry_seq]
    module_logger.info('{} antigens NOT matched against seqdb'.format(len(r)))
    return r

# ----------------------------------------------------------------------

def aa_at_positions(chart, positions, seqdb_file=None, verbose=False):
    seqdb = seqdb_backend.get_seqdb(timer=True)
    with timeit("Matching seqdb"):
        aa_indices = seqdb.aa_at_positions_for_antigens(antigens=chart.antigens(), positions=positions, verbose=verbose)
        module_logger.info('{} antigens matched against seqdb'.format(sum(len(v) for v in aa_indices.values())))
    return aa_indices

# ----------------------------------------------------------------------

def antigen_clades(chart, seqdb_file=None, verbose=False):
    clade_data = {}
    for ag_no, antigen_seq in enumerate(match(chart, seqdb_file=seqdb_file, verbose=verbose)):
        if antigen_seq:
            clades = antigen_seq.seq.clades()
            if clades:
                for clade in clades:
                    clade_data.setdefault(clade, []).append(ag_no)
            else:
                clade_data.setdefault("", []).append(ag_no)   # no clades but sequenced (for B/Vic)
    return clade_data

# ----------------------------------------------------------------------
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
