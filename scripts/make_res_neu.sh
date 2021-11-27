#!/bin/bash

#
# A script to build res.neu for the test app
#

echo "Packing resources to res.neu..."
cd ./bin
mkdir -p ./.tmp
cp -r ./resources ./.tmp/
cp ./neutralino.config.json ./.tmp/neutralino.config.json
asar pack ./.tmp res.neu
rm -r ./.tmp

echo "Check res.neu structure"
asar list res.neu

echo "OK: Packed."
