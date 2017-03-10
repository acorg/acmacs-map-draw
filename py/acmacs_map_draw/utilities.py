# -*- Python -*-
# license
# license.
# ----------------------------------------------------------------------

import os, socket, subprocess, tempfile, shutil
from contextlib import contextmanager
import logging; module_logger = logging.getLogger(__name__)

# ----------------------------------------------------------------------

def open_image(filename):
    hostname = socket.gethostname()
    if os.environ.get('USER') == 'eu' and hostname[:4] == 'jagd':
        if os.path.isdir(filename):
            import glob
            subprocess.run("qlmanage -p '{}'".format("' '".join(glob.glob(os.path.join(filename, "*.pdf")))), shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        else:
            # os.system("open '{}'".format(filename))
            subprocess.run("qlmanage -p '{}'".format(filename), shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

# ----------------------------------------------------------------------

@contextmanager
def temp_output(output=None, make_temp_output=True, suffix=".pdf"):
    remove_output = not output
    try:
        output = output or (make_temp_output and tempfile.mkstemp(suffix=suffix)[1])
        yield output
    finally:
        if remove_output and output:
            try:
                if os.path.isdir(output):
                    shutil.rmtree(output)
                else:
                    os.remove(output)
            except Exception as err:
                module_logger.error(err)

# ----------------------------------------------------------------------
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
