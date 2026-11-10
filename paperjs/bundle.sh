#!/bin/sh

set -e

# Download the latest esbuild if not present
if [ ! -x "./esbuild" ]; then
    curl -fsSL https://esbuild.github.io/dl/latest | sh
fi

# paper.js
if [ ! -f "paper-full.js" ]; then
    curl -sSL https://registry.npmjs.org/paper/-/paper-0.16.0.tgz | tar -xzf - --strip-components=2 package/dist/paper-full.js
fi

# nodom
if [ ! -f "nodom.js" ]; then
    curl -sSL https://registry.npmjs.org/nodom/-/nodom-2.4.0.tgz  | tar -xzf - --strip-components=2 package/dist/nodom.js
fi

# acorn
if [ ! -f "acorn.js" ]; then
    curl -sSL https://registry.npmjs.org/acorn/-/acorn-8.19.0.tgz | tar -xzf - --strip-components=2 package/dist/acorn.js
fi

# Bundle paper.js
./esbuild --bundle --format=esm                   \
    --external:./node/self.js                     \
    --external:./node/extend.js                   \
    --external:canvas                             \
    --alias:nodom.js=${PWD}/nodom.js              \
    --alias:acorn.js=${PWD}/acorn.js              \
    --inject:$1                                   \
    --outfile=$2                                  \
    paper-full.js

