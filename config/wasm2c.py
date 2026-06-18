# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os
import subprocess

def wasm2c(output, wasm2c_bin, wasm_lib, lfi_incstub, *rlbox_lfibin_path):
    output.close()
    module_name = os.path.basename(os.path.splitext(wasm_lib)[0])

    with open(lfi_incstub, 'r') as lfi_incstub_in:
        content = lfi_incstub_in.read()

    content = content.replace('REPLACE_ME_PATH', rlbox_lfibin_path[0])

    lfi_incstub_target = os.path.join(os.path.dirname(output.name), "lfiincstub.s")
    with open(lfi_incstub_target, 'w') as lfi_incstub_out:
        lfi_incstub_out.write(content)


    return subprocess.run(
        [wasm2c_bin, "-n", module_name, "-o", output.name, "--disable-simd", wasm_lib]
    ).returncode
