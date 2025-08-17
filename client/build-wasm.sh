#!/bin/bash

if [[ ! -e './fonts/twemoji.ttf' ]]; then
    echo 'Creating a symlink for twemoji'
    TWEMOJI_PATH=$(fc-list : family file | grep 'Twemoji' | sed 's/: Twemoji//')
    if [[ ! -f "${TWEMOJI_PATH}" ]]; then
        echo "Couldn't find a twemoji installation"
        exit 1
    fi

    if [[ ! -e './fonts' ]]; then
        mkdir './fonts'
    fi  
    ln -s "${TWEMOJI_PATH}" './fonts/twemoji.ttf'
fi

source /opt/qt6-wasm/qtwasm_env.sh
set -e

/opt/qt6-wasm/bin/qt-cmake -B build-wasm -GNinja # -DCMAKE_BUILD_TYPE=RelWithDebInfo # Rust breaks if compiled in release
cd build-wasm
ninja

read -p 'Do you want to serve the files? [y/N]: ' ANSWER
if [[ "${ANSWER}" == 'y' ]]; then
    python -m http.server 8000
fi

