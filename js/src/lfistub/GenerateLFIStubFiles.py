# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


import six
import os
import subprocess
from pathlib import Path
from tempfile import TemporaryDirectory

def generate_lfi_stub(main_out, libmozjs):
    with TemporaryDirectory() as dir:
        p = subprocess.Popen(["lfi-bind", "-dir", dir, "-gen-stub", libmozjs], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        main_tmp = os.path.join(dir, "embed", "stub", "arch", "x64", "main.s")
        thread_tmp = os.path.join(dir, "embed", "stub", "thread.c")

        stdout, stderr = p.communicate()
        
        stdout = six.ensure_text(stdout)
        stderr = six.ensure_text(stderr)

        if p.returncode != 0:
            print(stdout)
            print(stderr)

        with open(main_tmp) as f:
            main_out.write(f.read())

        with open(os.path.join(os.path.dirname(main_out.name), "thread.c"), "w") as thread_out:
            with open(thread_tmp) as f:
                thread_out.write(f.read())
