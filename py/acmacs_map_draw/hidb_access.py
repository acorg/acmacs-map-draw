# -*- Python -*-
# license
# license.
# ----------------------------------------------------------------------

import logging; module_logger = logging.getLogger(__name__)
import hidb_backend

# ----------------------------------------------------------------------

sVirusTypeNormalizer = {
    "h3": "A(H3N2)", "H3": "A(H3N2)", "A(H3N2)": "A(H3N2)",
    "h1": "A(H1N1)", "H1": "A(H1N1)", "A(H1N1)": "A(H1N1)",
    "b": "B", "B": "B",
    }

def get_hidb(virus_type=None, chart=None):
    if chart is not None:
        virus_type = chart.chart_info().virus_type()
    virus_type = sVirusTypeNormalizer[virus_type]
    module_logger.info("get_hidb {}".format(virus_type))
    hidb = hidb_backend.get_hidb(virus_type, True)
    return hidb

# ----------------------------------------------------------------------
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
