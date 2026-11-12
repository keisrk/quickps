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

# Locate or download acorn.js
ACORN="./acorn.js"
if [ ! -f "$ACORN" ]; then
    if [ -f "$ROOT_DIR/build/acorn.js" ]; then
        ACORN="$ROOT_DIR/build/acorn.js"
    elif [ -f "$SCRIPT_DIR/acorn.js" ]; then
        ACORN="$SCRIPT_DIR/acorn.js"
    else
        curl -sSL https://registry.npmjs.org/acorn/-/acorn-8.19.0.tgz | tar -xzf - --strip-components=2 package/dist/acorn.js
        ACORN="./acorn.js"
    fi
fi

# Locate or download paper-full.js
PAPER_FULL="./paper-full.js"
if [ ! -f "$PAPER_FULL" ]; then
    if [ -f "$ROOT_DIR/build/paper-full.js" ]; then
        PAPER_FULL="$ROOT_DIR/build/paper-full.js"
    elif [ -f "$SCRIPT_DIR/paper-full.js" ]; then
        PAPER_FULL="$SCRIPT_DIR/paper-full.js"
    else
        curl -sSL https://registry.npmjs.org/paper/-/paper-0.16.0.tgz | tar -xzf - --strip-components=2 package/dist/paper-full.js
        PAPER_FULL="./paper-full.js"
    fi
fi

"$ESBUILD" --bundle --format=esm \
    --external:./node/self.js \
    --external:./node/extend.js \
    --external:canvas \
    --alias:nodom.js="$NODOM" \
    --alias:acorn.js="$ACORN" \
    --inject:"$PREAMBLE" \
    --outfile="$OUTFILE" \
    "$PAPER_FULL"
