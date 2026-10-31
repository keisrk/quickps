#!/bin/sh

set -e

# Download the latest esbuild
curl -fsSL https://esbuild.github.io/dl/latest | sh

# paper.js
curl -sSL https://registry.npmjs.org/paper/-/paper-0.16.0.tgz | tar -xzf - --strip-components=2 package/dist/paper-full.js

# nodom
curl -sSL https://registry.npmjs.org/nodom/-/nodom-2.4.0.tgz  | tar -xzf - --strip-components=2 package/dist/nodom.js

# acorn
curl -sSL https://registry.npmjs.org/acorn/-/acorn-8.19.0.tgz | tar -xzf - --strip-components=2 package/dist/acorn.js

# Bundle paper.js
./esbuild --bundle --format=esm                   \
    --external:./node/self.js                     \
    --external:./node/extend.js                   \
    --alias:nodom.js=${PWD}/nodom.js              \
    --alias:acorn.js=${PWD}/acorn.js              \
    --inject:$1                                   \
    --outfile=$2                                  \
    paper-full.js

