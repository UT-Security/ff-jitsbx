#!/bin/bash
set -e
set -o pipefail
set -o xtrace


PYTHON_MINOR_VER=$(python3 -c "import sys; print(sys.version_info[1])")

if [ "$PYTHON_MINOR_VER" != "11" ]; then
    echo "Python 3.11 is not the installed version. You need to setup venv with python3.11 first."
    echo "1. Install python 3.11"
    echo "2. Run 'python3.11 -m venv ff_build_py && source ff_build_py/bin/activate'"
    echo "3. Rerun this build script"
    echo "You can run 'deactivate' to close your venv"
    exit 1
fi

function download_toolchain() {
    mkdir -p ./default-build-toolchain

    if [[ "$(uname -m)" == "x86_64" ]]; then
        wget https://github.com/UT-Security/ff-jitsbx-mozbuild/releases/download/esr-115/firefox-esr115-mozbuild.tar.gz
        tar -xzf firefox-esr115-mozbuild.tar.gz -C default-build-toolchain --strip-components=1
        rm firefox-esr115-mozbuild.tar.gz
    else
        wget https://github.com/UT-Security/ff-jitsbx-mozbuild/releases/download/esr-115/firefox-esr115-mozbuild-aarch64.tar.xz
        tar -xf firefox-esr115-mozbuild-aarch64.tar.xz -C default-build-toolchain --strip-components=1
        rm firefox-esr115-mozbuild-aarch64.tar.xz
    fi
}

function download_lfi_toolchain() {
    mkdir -p ./lfi-toolchain

    if [[ "$(uname -m)" == "x86_64" ]]; then
        wget https://github.com/lfi-project/lfi/releases/download/v0.12/x86_64-lfi-clang.tar.gz
        tar -xzf x86_64-lfi-clang.tar.gz -C lfi-toolchain --strip-components=1
        rm x86_64-lfi-clang.tar.gz
    else
        wget https://github.com/lfi-project/lfi/releases/download/v0.12/aarch64-lfi-clang.tar.gz
        tar -xzf aarch64-lfi-clang.tar.gz -C lfi-toolchain --strip-components=1
        rm aarch64-lfi-clang.tar.gz
    fi
}

function download_largelfi_toolchain() {
    mkdir -p ./largelfi-toolchain

    if [[ "$(uname -m)" == "x86_64" ]]; then
        scp $USER@rashford.csres.utexas.edu:/var/tmp/largesbx-data/toolchains/x86_64-lfi-large-clang.tar.gz ./x86_64-lfi-large-clang.tar.gz
        tar -xzf x86_64-lfi-large-clang.tar.gz -C largelfi-toolchain --strip-components=2
        rm x86_64-lfi-large-clang.tar.gz
    else
        echo "!!!!!!!!!Large sandbox toolchain not yet supported on aarch64"
    fi
}

if [ ! -f ./done-default-build-toolchain ]; then
    download_toolchain;
    touch ./done-default-build-toolchain
fi

######################################

if [ ! -f ./done-lfi-toolchain ]; then
    download_lfi_toolchain;
    touch ./done-lfi-toolchain
fi

######################################

if [ ! -f ./done-largelfi-toolchain ]; then
    download_largelfi_toolchain;
    touch ./done-largelfi-toolchain
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

if [ ! -d ../lfi-runtime ]; then
    git clone --recursive git@github.com:lfi-project/lfi-runtime.git ../lfi-runtime
fi

pushd .
cd ../lfi-runtime
git pull --rebase --autostash

meson setup --reconfigure ./build_debug --buildtype debug \
    -D c_args="-fno-exceptions" -D cpp_args="-fno-exceptions" -D c_link_args="-fno-exceptions" -Dsegue_cache_gs=true
ninja -C ./build_debug

meson setup --reconfigure ./build_release --buildtype debug \
    -D c_args="-fno-exceptions" -D cpp_args="-fno-exceptions" -D c_link_args="-fno-exceptions" -Dsegue_cache_gs=true
ninja -C ./build_release

popd

######################################

if [ ! -d ../largelfi-runtime ]; then
    git clone --recursive -b large-sandbox git@github.com:lfi-project/lfi-runtime.git ../largelfi-runtime
fi

pushd .
cd ../largelfi-runtime
git pull --rebase --autostash

meson setup --reconfigure ./build_debug --buildtype debug \
    -D c_args="-fno-exceptions" -D cpp_args="-fno-exceptions" -D c_link_args="-fno-exceptions" -Denable_large_sandbox=true -Denable_gs_context=true -Denable_segue=false
ninja -C ./build_debug

meson setup --reconfigure ./build_release --buildtype debug \
    -D c_args="-fno-exceptions" -D cpp_args="-fno-exceptions" -D c_link_args="-fno-exceptions" -Denable_large_sandbox=true -Denable_gs_context=true -Denable_segue=false
ninja -C ./build_release

popd

######################################

if [ ! -f ./done-bootstrap ]; then

    # Don't use the bootstrap as the firefox sysroot is too restricted. Just use the system compiler
    # Bootstrap will fail
    # MOZCONFIG=./mozconfig_stock_debug ./mach --no-interactive bootstrap --application-choice browser || echo "---------Ignoring bootstrap failure------";

    sudo apt install -y libasound2-dev libpulse-dev libpango1.0-dev libx11-xcb-dev libxrandr-dev libxcomposite-dev libxcursor-dev libxdamage-dev libxfixes-dev libxi-dev libxtst-dev libgtk-3-dev libdbus-glib-1-dev xvfb linux-tools-common;

    if [ ! -x "$(command -v rustup)" ] ; then
        curl https://sh.rustup.rs -sSf | sh -s -- --default-toolchain 1.76.0 -y;
        . "$HOME/.cargo/env"
    fi

    rustup override set 1.76.0;

    touch ./done-bootstrap
fi

######################################

# Use the system compiler and libc++ as the Firefox one is too old
# export MOZBUILD_STATE_PATH="$(realpath .)/default-build-toolchain/"

MOZCONFIG=./mozconfig_stock_release ./mach build

MOZCONFIG=./mozconfig_wasm_release ./mach build

LFI_TOOLCHAIN_PATH="$(realpath .)/lfi-toolchain" MOZCONFIG=./mozconfig_lfi_release ./mach build

if [[ "$(uname -m)" == "x86_64" ]]; then
    LFI_TOOLCHAIN_PATH="$(realpath .)/largelfi-toolchain" MOZCONFIG=./mozconfig_largelfi_release ./mach build
fi
