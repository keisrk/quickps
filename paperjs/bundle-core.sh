#!/bin/sh

set -e

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
ROOT_DIR=$(cd "$SCRIPT_DIR/.." && pwd)

PREAMBLE="$1"
OUTFILE="$2"

# Locate or download esbuild
ESBUILD="./esbuild"
if [ ! -x "$ESBUILD" ]; then
    if [ -x "$ROOT_DIR/build/esbuild" ]; then
        ESBUILD="$ROOT_DIR/build/esbuild"
    elif [ -x "$SCRIPT_DIR/esbuild" ]; then
        ESBUILD="$SCRIPT_DIR/esbuild"
    else
        curl -fsSL https://esbuild.github.io/dl/latest | sh
        ESBUILD="./esbuild"
    fi
fi

# Locate or download nodom.js
NODOM="./nodom.js"
if [ ! -f "$NODOM" ]; then
    if [ -f "$ROOT_DIR/build/nodom.js" ]; then
        NODOM="$ROOT_DIR/build/nodom.js"
    elif [ -f "$SCRIPT_DIR/nodom.js" ]; then
        NODOM="$SCRIPT_DIR/nodom.js"
    else
        curl -sSL https://registry.npmjs.org/nodom/-/nodom-2.4.0.tgz | tar -xzf - --strip-components=2 package/dist/nodom.js
        NODOM="./nodom.js"
    fi
fi

# Locate or download paper-core.js
PAPER_CORE="./paper-core.js"
if [ ! -f "$PAPER_CORE" ]; then
    if [ -f "$ROOT_DIR/build/paper-core.js" ]; then
        PAPER_CORE="$ROOT_DIR/build/paper-core.js"
    else
        curl -sSL https://registry.npmjs.org/paper/-/paper-0.16.0.tgz | tar -xzf - --strip-components=2 package/dist/paper-core.js
        PAPER_CORE="./paper-core.js"
    fi
fi

"$ESBUILD" --bundle --format=esm \
    --external:./node/self.js \
    --external:./node/extend.js \
    --external:canvas \
    --alias:nodom.js="$NODOM" \
    --alias:paper-core.js="$PAPER_CORE" \
    --inject:"$PREAMBLE" \
    --outfile="$OUTFILE" \
    "$SCRIPT_DIR/paper-core-entry.js"
