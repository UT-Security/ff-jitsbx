#!/bin/bash
set -e
set -o pipefail
set -o xtrace

if ! command -v python3.11 >/dev/null 2>&1
then
    sudo apt install -y python3.11 python3.11-venv
fi

PYTHON_MINOR_VER=$(python3 -c "import sys; print(sys.version_info[1])")

if [ "$PYTHON_MINOR_VER" != "11" ]; then
    if [ ! -d "../ff_build_py" ]; then
        python3.11 -m venv ../ff_build_py
    fi
    source ../ff_build_py/bin/activate
fi

function download_toolchain() {
    mkdir -p ./default-build-toolchain

    wget https://github.com/UT-Security/ff-jitsbx-mozbuild/releases/download/esr-115/firefox-esr115-mozbuild.tar.gz
    tar -xzf firefox-esr115-mozbuild.tar.gz -C default-build-toolchain --strip-components=1
    rm firefox-esr115-mozbuild.tar.gz
}

function download_android_toolchain() {
    mkdir -p ./default-android-build-toolchain
    MOZBUILD_STATE_PATH="$(realpath .)/default-android-build-toolchain" \
        ./mach python python/mozboot/mozboot/android.py --no-interactive
}

function download_lfi_toolchain() {
    mkdir -p ./lfi-toolchain

    wget https://github.com/lfi-project/lfi/releases/download/v0.12/x86_64-lfi-clang.tar.gz
    tar -xzf x86_64-lfi-clang.tar.gz -C lfi-toolchain --strip-components=1
    rm x86_64-lfi-clang.tar.gz

    mkdir -p ./lfi-toolchain-aarch64

    wget https://github.com/lfi-project/lfi/releases/download/v0.12/aarch64-lfi-clang.tar.gz
    tar -xzf aarch64-lfi-clang.tar.gz -C lfi-toolchain-aarch64 --strip-components=1
    rm aarch64-lfi-clang.tar.gz
}

function download_largelfi_toolchain() {
    mkdir -p ./largelfi-toolchain

    wget https://github.com/UT-Security/-largesbx-compiler-builds/releases/download/curr/x86_64-lfi-large-clang.tar.gz
    tar -xzf x86_64-lfi-large-clang.tar.gz -C largelfi-toolchain --strip-components=1
    rm x86_64-lfi-large-clang.tar.gz

    mkdir -p ./largelfi-toolchain-aarch64

    wget https://github.com/UT-Security/-largesbx-compiler-builds/releases/download/curr/aarch64-lfi-large-clang.tar.gz
    tar -xzf aarch64-lfi-large-clang.tar.gz -C largelfi-toolchain-aarch64 --strip-components=1
    rm aarch64-lfi-large-clang.tar.gz
}

if [ ! -f ./done-default-build-toolchain ]; then
    download_toolchain;
    touch ./done-default-build-toolchain
fi

######################################

if [ ! -f ./done-default-android-build-toolchain ]; then
    download_android_toolchain;
    touch ./done-default-android-build-toolchain
fi

ANDROID_NDK_BIN="$(realpath .)/default-android-build-toolchain/android-ndk-r23c/toolchains/llvm/prebuilt/linux-x86_64/bin"
export PATH="$ANDROID_NDK_BIN:$PATH"

######################################

if [ ! -f ./done-lfi-toolchain-aarch64 ]; then
    download_lfi_toolchain;
    touch ./done-lfi-toolchain-aarch64
fi

######################################

if [ ! -f ./done-largelfi-toolchain-aarch64 ]; then
    download_largelfi_toolchain;
    touch ./done-largelfi-toolchain-aarch64
fi

######################################

if [ ! -d ../rlbox_lfi_sandbox ]; then
    git clone --recursive git@github.com:UT-Security/rlbox_lfi_sandbox.git ../rlbox_lfi_sandbox
fi

pushd .
cd ../rlbox_lfi_sandbox
git pull --rebase --autostash
popd

######################################

if [ ! -d ../lfi-runtime-aarch64 ]; then
    git clone --recursive -b springboard git@github.com:lfi-project/lfi-runtime.git ../lfi-runtime-aarch64
fi

pushd .
cd ../lfi-runtime-aarch64
git pull --rebase --autostash

# Cross compile for aarch64
meson setup --reconfigure ./build_release_aarch64 --buildtype release --cross-file toolchains/aarch64-android.meson \
    -D c_args="-fno-exceptions -fno-emulated-tls" -D cpp_args="-fno-exceptions -fno-emulated-tls" -D c_link_args="-fno-exceptions"
ninja -C ./build_release_aarch64 liblfi.a

popd

######################################

if [ ! -d ../largelfi-runtime-aarch64 ]; then
    # Cross build
    git clone --recursive -b large-sandbox-aarch64 git@github.com:lfi-project/lfi-runtime.git ../largelfi-runtime-aarch64
fi

pushd .
cd ../largelfi-runtime-aarch64
git pull --rebase --autostash

# Cross compile for aarch64
meson setup --reconfigure ./build_release_aarch64 --buildtype release --cross-file toolchains/aarch64-android.meson \
    -D c_args="-fno-exceptions -fno-emulated-tls" -D cpp_args="-fno-exceptions -fno-emulated-tls" -D c_link_args="-fno-exceptions" -Dlarge_sandbox=true -Ddisable_signals=true

ninja -C ./build_release_aarch64 liblfi.a

popd

######################################

if [ ! -f ./done-bootstrap ]; then

    # Don't use the bootstrap as the firefox sysroot is too restricted. Just use the system compiler
    # Bootstrap will fail
    # MOZCONFIG=./mozconfig_stock_debug ./mach --no-interactive bootstrap --application-choice browser || echo "---------Ignoring bootstrap failure------";

    sudo apt install -y libasound2-dev libpulse-dev libpango1.0-dev libx11-xcb-dev libxrandr-dev libxcomposite-dev libxcursor-dev libxdamage-dev libxfixes-dev libxi-dev libxtst-dev libgtk-3-dev libdbus-glib-1-dev xvfb linux-tools-common cpufrequtils python3-simplejson;

    if [ ! -x "$(command -v rustup)" ] ; then
        curl https://sh.rustup.rs -sSf | sh -s -- --default-toolchain 1.76.0 -y;
        . "$HOME/.cargo/env"
    fi

    rustup override set 1.76.0;
    touch ./done-bootstrap
fi

rustup target add aarch64-linux-android;

######################################

# Android stock release
MOZCONFIG=mozconfig_android_stock_release ./mach build && \
MOZCONFIG=mozconfig_android_stock_release ./mach package

# Android wasm release
MOZCONFIG=mozconfig_android_wasm_release ./mach build && \
MOZCONFIG=mozconfig_android_wasm_release ./mach package

# Android lfi release
FIREFOX_COMPILE_FOR_ANDROID=1 LFI_TOOLCHAIN_PATH="$(realpath .)/lfi-toolchain" MOZCONFIG=mozconfig_android_lfi_release ./mach build && \
FIREFOX_COMPILE_FOR_ANDROID=1 LFI_TOOLCHAIN_PATH="$(realpath .)/lfi-toolchain" MOZCONFIG=mozconfig_android_lfi_release ./mach package

# Android largelfi release
FIREFOX_COMPILE_FOR_ANDROID=1 LFI_TOOLCHAIN_PATH="$(realpath .)/largelfi-toolchain" MOZCONFIG=mozconfig_android_largelfi_release ./mach build && \
FIREFOX_COMPILE_FOR_ANDROID=1 LFI_TOOLCHAIN_PATH="$(realpath .)/largelfi-toolchain" MOZCONFIG=mozconfig_android_largelfi_release ./mach package
