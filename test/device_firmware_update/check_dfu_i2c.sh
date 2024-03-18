#!/bin/bash

# To run this tests do the following:
# 1. Configure a Rapsberry-Pi:
#   a. Follow the instructions for XVF3800-INT on https://github.com/xmos/vocalfusion-rpi-setup?tab=readme-ov-file#setup
#   b. Install dfu-util on a Raspbnerry-Pi and add it to the path
# 2. Connect a voice-reference board to the Raspberry-Pi expander
# 2. Connect an xTAG to voice-reference board from a host machine, and flash a voice-reference board with example_ffva_int_fixed_delay
# 3. Generate an upgrade image using `xflash --upgrade 1 example_ffva_int_fixed_delay.xe --factory-version 15.2 -o download1.bin
# 4. Generate a different upgrade image using `xflash --upgrade 1 example_ffva_int_fixed_delay.xe --factory-version 15.2 -o download2.bin
# 5. Copy the files download1.bin, download2.bin, emptyfile.bin and check_dfu_i2c.sh to the Raspberry-Pi
# 6. On the Raspberry-Pi, set the desired value of ITERATION_NUM in check_dfu_i2c.sh and run this script: `source check_dfu_i2c.sh`

ITERATION_NUM=400
UPGRADE_FILE_1="download1.bin"
UPGRADE_FILE_2="download2.bin"
EMPTY_FILE="emptyfile.bin"
UPLOAD_FILE="upload.bin"

check_upgrade() {

  # Download upgrade image
  ./xvf_dfu --download $1
  # Reboot the device
  ./xvf_dfu -r

  # 3s delay
  sleep 3

  # Upload upgrade image
  ./xvf_dfu --upload-upgrade $UPLOAD_FILE

  # Compare uploaded and downloaded images
  diff $UPLOAD_FILE $1
  if [[ $? == 0 ]]; then
    echo "Upload of $1 is correct"
  else
    echo "Upload of $1 is incorrect"
    exit 1
  fi

  # Delete downloaded image
  rm $1

  # 1s delay
  sleep 1
}

counter=0

#Checking if the file exists
if [ ! -f $UPGRADE_FILE_1 ]; then
  echo "File $UPGRADE_FILE_1 doesn't exist."
  exit -1
fi
if [ ! -f $UPGRADE_FILE_2 ]; then
  echo "File $UPGRADE_FILE_2 doesn't exist."
  exit -1
fi
if [ ! -f $EMPTY_FILE ]; then
  echo "File $EMPTY_FILE doesn't exist."
  exit -1
fi

while [ $counter -lt $ITERATION_NUM ]
do
  counter=$(( $counter + 1 ))
  echo "DFU attempt number $counter"

  # Download first upgrade image
  check_upgrade $UPGRADE_FILE_1

  # Download second upgrade image
  check_upgrade $UPGRADE_FILE_2

  # Download empty upgrade image
  check_upgrade $EMPTY_FILE

done
