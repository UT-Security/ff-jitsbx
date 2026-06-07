#!/bin/bash
set -e
set -o pipefail
set -o xtrace

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
    -D c_args="-fno-exceptions" -D cpp_args="-fno-exceptions" -D c_link_args="-fno-exceptions"
ninja -C ./build_debug

meson setup --reconfigure ./build_release --buildtype debug \
    -D c_args="-fno-exceptions" -D cpp_args="-fno-exceptions" -D c_link_args="-fno-exceptions"
ninja -C ./build_release

popd

######################################

if [ ! -f ./done-bootstrap ]; then
    rustup override set 1.76.0;

    # Don't use the bootstrap as the firefox sysroot is too restricted. Just use the system compiler
    # Bootstrap will fail
    # MOZCONFIG=./mozconfig_stock_debug ./mach --no-interactive bootstrap --application-choice browser || echo "---------Ignoring bootstrap failure------";

    sudo apt install libasound2-dev libpulse-dev libpango1.0-dev libx11-xcb-dev libxrandr-dev libxcomposite-dev libxcursor-dev libxdamage-dev libxfixes-dev libxi-dev libxtst-dev libgtk-3-dev libdbus-glib-1-dev

    touch ./done-bootstrap
fi

######################################

# Use the system compiler and libc++ as the Firefox one is too old
# export MOZBUILD_STATE_PATH="$(realpath .)/default-build-toolchain/"

MOZCONFIG=./mozconfig_stock_release ./mach build

MOZCONFIG=./mozconfig_wasm_release ./mach build

MOZCONFIG=./mozconfig_lfi_release ./mach build
