#!/bin/bash
set -e
set -o pipefail

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

pushd . && cd ../rlbox_lfi_sandbox && git pull --rebase --autostash && popd

######################################

if [ ! -d ../lfi-runtime ]; then
    git clone --recursive git@github.com:lfi-project/lfi-runtime.git ../lfi-runtime
fi

pushd . && cd ../lfi-runtime
git pull --rebase --autostash
meson setup ./build_debug --buildtype debug && ninja -C ./build_debug
meson setup ./build_release --buildtype release && ninja -C ./build_release
popd

######################################

if [ ! -f ./done-bootstrap ]; then
    rustup override set 1.76.0;
    # Bootstrap will fail
    MOZCONFIG=./mozconfig_debug ./mach --no-interactive bootstrap --application-choice browser || echo "---------Ignoring bootstrap failure------";
    touch ./done-bootstrap
fi

######################################

export MOZBUILD_STATE_PATH="$(realpath .)/default-build-toolchain/"

MOZCONFIG=./mozconfig_debug ./mach build

MOZCONFIG=./mozconfig_lfi_debug ./mach build
