#!/bin/bash

#
# A script to build res.neu for the test app
#

echo "Updating client..."
./scripts/update_client.sh

echo "Packing resources to resources.neu..."
cd ./bin
mkdir -p ./.res
cp -r ./resources ./.res/
cp ./neutralino.config.json ./.res/neutralino.config.json
asar pack ./.res resources.neu
rm -r ./.res

echo "Check resources.neu structure"
asar list resources.neu

cp resources.neu ./resources/updater_test/update_res.neu

echo "OK: Packed."
