#!/bin/bash

# To run this tests do the following:
# 1. Configure a Rapsberry-Pi:
#   a. Install dfu-util on a Raspbnerry-Pi and add it to the path
#   b. Clone vocalfusion-rpi-setup and run ./setup.sh xvf3800-int
# 2. Flash a voice-reference board with example_ffva_int_fixed_delay
# 3. Generate an upgrade image using `xflash --upgrade 1 example_ffva_int_fixed_delay.xe --factory-version 15.2 -o download1.bin
# 4. Generate a different upgrade image using `xflash --upgrade 1 example_ffva_int_fixed_delay.xe --factory-version 15.2 -o download2.bin
# 5. Copy the files download1.bin, download2.bin, emptyfile.bin and test_dfu_i2c.sh to the Raspberry-Pi
# 6. On the Raspberry-Pi, set the correct value of ITERATION_NUM and run this script: ./check_dfu_i2c.sh

ITERATION_NUM=400
UPGRADE_FILE_1="download1.bin"
UPGRADE_FILE_2="download2.bin"
EMPTY_FILE="emptyfile.bin"
UPLOAD_FILE="upload.bin"

check_upgrade() {

  ./xvf_dfu --download $1
  ./xvf_dfu -r
  
  sleep 3  # 3s delay

  ./xvf_dfu --upload-upgrade $UPLOAD_FILE
  revval=$(diff $UPLOAD_FILE $1)
  echo "Output: $output"
  if [[ $output == 0 ]]; then
    echo "Upload is correct"
  else
    echo "Upload is incorrect"
    exit 1
  fi

  sleep 3  # 3s delay

}

counter=0
while [ $counter -lt $ITERATION_NUM ]
do
  counter=$(( $counter + 1 ))
  echo "DFU attempt number $counter"

  check_upgrade $UPGRADE_FILE_1

  check_upgrade $UPGRADE_FILE_1

  check_upgrade $EMPTY_FILE

done
