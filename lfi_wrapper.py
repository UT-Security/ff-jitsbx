#!/usr/bin/env python3

import sys
import subprocess
import os
import re

FLAG = "--forward-to-lfi-compiler"
this_dir = os.path.dirname(os.path.realpath(__file__))
default_clang_dir = os.path.join(this_dir, "default-build-toolchain/clang/bin/clang")
lfi_toolchain_dir = os.path.join(this_dir, "lfi-toolchain")

def replace_wasm_extensions(args):
    return [re.sub(r"\.wasm(?=\.|$)", ".lfi", arg) for arg in args]


# Compile input: ['/Code/LargeSandboxPaper/ff-jitsbx/default-build-toolchain/clang/bin/clang++', '--target=wasm32-wasi', '--sysroot=/Code/LargeSandboxPaper/ff-jitsbx/default-build-toolchain/sysroot-wasm32-wasi', '-o', 'GraphiteExtra.wasm', '-c', '-I/Code/LargeSandboxPaper/ff-jitsbx/security/rlbox', '-I/Code/LargeSandboxPaper/ff-jitsbx/obj_firefox_latest_lfi_debug/security/rlbox', '-I/Code/LargeSandboxPaper/ff-jitsbx/third_party/wasm2c/wasm2c', '-I/Code/LargeSandboxPaper/ff-jitsbx/gfx/graphite2/src', '-I/Code/LargeSandboxPaper/ff-jitsbx/obj_firefox_latest_lfi_debug/dist/include', '-I/Code/LargeSandboxPaper/ff-jitsbx/obj_firefox_latest_lfi_debug/dist/include/nspr', '-I/Code/LargeSandboxPaper/ff-jitsbx/obj_firefox_latest_lfi_debug/dist/include/nss', '-g', '-fno-exceptions', '-fno-strict-aliasing', '-DDEBUG=1', '-DMOZILLA_CLIENT', '-D_WASI_EMULATED_PROCESS_CLOCKS', '-DMOZ_IN_WASM_SANDBOX', '-DPACKAGE_VERSION="moz"', '-DPACKAGE_BUGREPORT="http://bugzilla.mozilla.org/"', '-DGRAPHITE2_STATIC', '-DGRAPHITE2_CUSTOM_HEADER="MozGrMalloc.h"', '-DGRAPHITE2_NFILEFACE', '-DGRAPHITE2_NTRACING', '-DGRAPHITE2_NSEGCACHE', '-I/Code/LargeSandboxPaper/ff-jitsbx/obj_firefox_latest_lfi_debug/dist/stl_wrappers', '-MD', '-MP', '-MF', '.deps/GraphiteExtra.wasm.pp', '-fcolor-diagnostics', '/Code/LargeSandboxPaper/ff-jitsbx/gfx/graphite2/geckoextra/src/GraphiteExtra.cpp']
# Linker input: $(WASM_CXX) -o $@ -Wl,--export-all -Wl,--stack-first -Wl,-z,stack-size=$(if $(MOZ_OPTIMIZE),262144,1048576) -Wl,--no-entry -Wl,--growable-table -Wl,--import-memory -Wl,--import-table $(CWASMOBJS) $(CPPWASMOBJS) -lwasi-emulated-process-clocks $(if $(WASM_USE_LFI),--forward-to-lfi-compiler,)
def replace_tools_sysroot_flags(args):
    # if 1st arg ends with "/clang" change it to "lfi-clang"
    # if 1st arg ends with "/clang++" change it to "lfi-clang++"
    if args[0].endswith("/clang"):
        args[0] = os.path.join(lfi_toolchain_dir, "bin/clang")
    elif args[0].endswith("/clang++"):
        args[0] = os.path.join(lfi_toolchain_dir, "bin/clang++")

    # remove args *"/sysroot-wasm32-wasi" or in the below removed_args
    removed_args = ["--target=wasm32-wasi", "-Wl,--export-all", "-Wl,--stack-first",
        "-Wl,--no-entry", "-Wl,--growable-table", "-Wl,--import-memory", "-Wl,--import-table",
        "-lwasi-emulated-process-clocks"
    ]
    args = list(filter(lambda arg: not arg.endswith("/sysroot-wasm32-wasi") and arg not in removed_args, args))
    return args

def add_required_flags(args):
    return  args + [
        "-include", os.path.join(this_dir, "config/external/rlbox_lfi_sandbox/lfi-compat.h"),
        "-Wl,--export-dynamic",
        "-static-pie",
        "-L", os.path.join(lfi_toolchain_dir, "sysroot/usr/lib"),
        "-lboxrt",
    ]


def main():
    original_args = list(filter(lambda arg: arg != FLAG, sys.argv[1:]))
    found = len(original_args) != (len(sys.argv) - 1)

    if found:
        print("Also calling LFI. Original args: " + str(original_args))

        is_linker_call = found = any(filter(lambda arg: arg == "-Wl,--import-memory", original_args))

        # Fixed up arguments
        lfi_ext_args = replace_wasm_extensions(original_args)
        lfi_tools_args = replace_tools_sysroot_flags(lfi_ext_args)
        lfi_final_args = add_required_flags(lfi_tools_args)

        # Update environment
        my_env = os.environ.copy()
        lfi_bin_dir = os.path.join(lfi_toolchain_dir, "lfi-bin")
        my_env["PATH"] = lfi_bin_dir + ":" + my_env['PATH']

        print("Also calling LFI with args: " + str(lfi_final_args))
        result_lfi = subprocess.run([*lfi_final_args], env=my_env)
        if result_lfi.returncode != 0:
            print("LFI failed for args: " + str(lfi_final_args))
            sys.exit(result_lfi.returncode)

        if is_linker_call:
            # Do additional work on the linker step
            pass

    if original_args[0] == "--version":
        # Special case ... this sometimes happens
        result = subprocess.run([default_clang_dir] + [*original_args])
    else:
        result = subprocess.run([*original_args])

    if result.returncode != 0:
        print("Regular compile failed for args: " + str(original_args))
        sys.exit(0)
    sys.exit(result.returncode)

main()