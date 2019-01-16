#!/bin/bash

export SDK_PATH="/mnt/share/esp8266/ESP8266_RTOS_SDK"
export BIN_PATH="/mnt/share/esp8266/ESP8266_RTOS_SDK/bin"

echo "start clean ..."
echo ""

cd ../../
make clean
find . -name .output | xargs rm -fr
cd $pwd