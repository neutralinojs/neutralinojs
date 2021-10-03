#!/bin/bash

#
# A script to build client and update for development use
#

if [ ! -e ../neutralino.js ]; then
    echo "ERR: You need to download client's repo to ../ location relative to the server repo."
    exit
fi

echo "Building client..."
cd ../neutralino.js
npm run build -- --dev

echo "Updating client..."
cp ./dist/neutralino.js  ../neutralinojs/bin/resources/js/neutralino.js

echo "OK: Updated."
