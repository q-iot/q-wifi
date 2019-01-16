#!/bin/bash

export SDK_PATH="/mnt/share/esp8266/ESP8266_RTOS_SDK"
export BIN_PATH="/mnt/share/esp8266/ESP8266_RTOS_SDK/bin"

echo "gen_misc.sh version 20150911"
echo ""

echo "start..."
echo ""

echo $SDK_PATH
echo $BIN_PATH

cd ../../

#make clean

#��boot 1024kģʽ
make BOOT=none APP=0 SPI_SPEED=40 SPI_MODE=DIO SPI_SIZE_MAP=4

cd $pwd

#BOOT= new:�ƶ�������none:��boot
#APP= 0:����eagle.flash.bin��eagle.irom0text.bin��֧���ƶ�����
#APP= 1:����user1.bin
#APP= 2:����user2.bin
#SPI_SPEED��SPI_MODEһ�㲻��
#SPI_SIZE_MAP= 0:flash size 512K
#SPI_SIZE_MAP= 2:flash size 1024K
#SPI_SIZE_MAP= 3:flash size 2048K
#SPI_SIZE_MAP= 6:flash size 4096K
