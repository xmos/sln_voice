#!/bin/bash

# To run this test do the following:
# 1. Configure a Rapsberry-Pi:
#   a. Follow the instructions for XVF3800-INT on https://github.com/xmos/vocalfusion-rpi-setup?tab=readme-ov-file#setup
#   b. Clone on the Raspberry Pi the repo host_xvf_control using `git clone https://github.com/xmos/host_xvf_control`
#   c. In the file host_xvf_control/src/dfu/transport_config.yaml update the value of I2C_ADDRESS from 0x2C to 0x42
#   d. Build the xvf_dfu host application on a Raspberry-Pi using the instructions in https://github.com/xmos/host_xvf_control/blob/main/README.rst
# 2. Prepare the hardware
#   a. Attach an XK-VOICE-L71 board to the Raspberry-Pi expander
#   b. Connect an xTAG to XK-VOICE-L71 board and to a host machine
# 3. Build the application example_ffva_int_fixed_delay and flash it to the board
# 4. Generate an upgrade image using `xflash --upgrade 1 example_ffva_int_fixed_delay.xe --factory-version 15.2 -o download1.bin
# 5. Generate a different upgrade image using `xflash --upgrade 1 example_ffva_int_fixed_delay.xe --factory-version 15.2 -o download2.bin
# 6. Copy the files download1.bin, download2.bin, emptyfile.bin and check_dfu_i2c.sh to the host_xvf_control/build folder on the Raspberry-Pi
# 7. On the Raspberry-Pi, set the desired value of ITERATION_NUM in check_dfu_i2c.sh and run this script: `source check_dfu_i2c.sh`

ITERATION_NUM=2
UPGRADE_FILE_1="download1.bin"
UPGRADE_FILE_2="download2.bin"
EMPTY_FILE="emptyfile.bin"
UPLOAD_FILE="upload.bin"

check_upgrade() {

  # Download upgrade image
  ./xvf_dfu --download $1

  # Reboot the device
  ./xvf_dfu --reboot

  # 3s delay
  sleep 3

  # Read app version, just for debugging
  ./xvf_dfu --version

  # Upload upgrade image
  ./xvf_dfu --upload-upgrade $UPLOAD_FILE

  # Compare uploaded and downloaded images
  upload_size=$(stat -c '%s' $UPLOAD_FILE)
  # The uploaded file is not padded,
  # so don't include the padding of the downloaded image
  cmp -n $upload_size $UPLOAD_FILE $1
  if [[ $? == 0 ]]; then
    echo "Upload of $1 is correct"
  else
    echo "Upload of $1 is incorrect"
    exit 1
  fi

  # Delete uploaded images
  rm $UPLOAD_FILE

  # 1s delay
  sleep 1
}

counter=0

# Checking if necessary files exist
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
  echo "------------------------"
  echo "DFU attempt number $counter"
  echo "------------------------"

  # Download first upgrade image
  check_upgrade $UPGRADE_FILE_1

  # Download second upgrade image
  check_upgrade $UPGRADE_FILE_2

  # Download empty upgrade image
  check_upgrade $EMPTY_FILE

done
