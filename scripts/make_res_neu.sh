#!/bin/bash

#
# A script to build res.neu for the test app
#

echo "Packing resources to resources.neu..."
cd ./bin
mkdir -p ./.tmp
cp -r ./resources ./.tmp/
cp ./neutralino.config.json ./.tmp/neutralino.config.json
asar pack ./.tmp resources.neu
rm -r ./.tmp

echo "Check resources.neu structure"
asar list resources.neu

echo "OK: Packed."
