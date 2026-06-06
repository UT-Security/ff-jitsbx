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

if [ ! -f ./done-default-build-toolchain ]; then
    download_toolchain;
    touch ./done-default-build-toolchain
fi


if [ ! -f ./done-bootstrap ]; then
    rustup override set 1.76.0;
    # Bootstrap will fail
    MOZCONFIG=./mozconfig_debug ./mach --no-interactive bootstrap --application-choice browser || echo "---------Ignoring bootstrap failure------";
    touch ./done-bootstrap
fi

export MOZBUILD_STATE_PATH="$(realpath .)/default-build-toolchain/"

MOZCONFIG=./mozconfig_debug ./mach build
