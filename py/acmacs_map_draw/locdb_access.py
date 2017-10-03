# -*- Python -*-
# license
# license.
# ----------------------------------------------------------------------

import os
from pathlib import Path
import logging; module_logger = logging.getLogger(__name__)
from acmacs_base.timeit import timeit
import locationdb_backend

# ----------------------------------------------------------------------

sLocDb = None

def get_locdb(locdb_file :Path = None):
    if locdb_file:
        locationdb_backend.locdb_setup(str(locdb_file))
    return locationdb_backend.get_locdb(timer=True)

# ----------------------------------------------------------------------
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
