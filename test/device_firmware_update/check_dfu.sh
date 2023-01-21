#!/bin/bash
# Copyright (c) 2022, XMOS Ltd, All rights reserved
set -e # exit on first error
set -x # echo on

# help text
help()
{
   echo "XCORE-VOICE Device Firmware Update (DFU) Test"
   echo
   echo "Syntax: check_dfu.sh [-h] firmware output_dir adapterID"
   echo
   echo "options:"
   echo "h     Print this Help."
}

# flag arguments
while getopts h option
do
    case "${option}" in
        h) help
           exit;;
    esac
done

# assign command line args
if [ ! -z "${@:$OPTIND:1}" ] && [ ! -z "${@:$OPTIND+1:1}" ]
then
    FIRMWARE=${@:$OPTIND:1}
    OUTPUT_DIR=${@:$OPTIND+1:1}
fi
if [ ! -z "${@:$OPTIND+2:1}" ]
then
    ADAPTER_ID="--adapter-id ${@:$OPTIND+2:1}"
fi

# discern repository root
SLN_VOICE_ROOT=$(git rev-parse --show-toplevel)
source "${SLN_VOICE_ROOT}"/tools/ci/helper_functions.sh

# xflash erase
xflash --erase-all --target-file "${SLN_VOICE_ROOT}"/examples/ffd/bsp_config/XK_VOICE_L71/XK_VOICE_L71.xn

# flash the filesystem
# build_tests.sh creates dist/example_ffva_ua_adec_fat.fs to be used here
xflash ${ADAPTER_ID} --quad-spi-clock 50MHz --factory dist/example_ffva_ua_adec_test.xe --boot-partition-size 0x100000 --data dist/example_ffva_ua_adec_fat.fs

# wait for device to reset (may not be necessary)
sleep 3

# strip path and .xe from firmware
FIRMWARE_NAME="${FIRMWARE#*dist/}"
FIRMWARE_NAME="${FIRMWARE_NAME%%.*}"
echo "${FIRMWARE_NAME}"

# remove any previous images
rm -rf "${OUTPUT_DIR}"
mkdir "${OUTPUT_DIR}"

# get the tools version numbers for xflash
export_tools_version

# create the upgrade firmware
xflash ${ADAPTER_ID} --factory-version ${XTC_VERSION_MAJOR}.${XTC_VERSION_MINOR} --upgrade 0 ${FIRMWARE} -o ${OUTPUT_DIR}/${FIRMWARE_NAME}_upgrade.bin

# write the upgrade image
dfu-util -e -d 0020 -a 1 -D ${OUTPUT_DIR}/${FIRMWARE_NAME}_upgrade.bin --reset

# wait for dust to gather
sleep 1

# get readback upgrade image
dfu-util -e -d 0020 -a 1 -U ${OUTPUT_DIR}/readback_upgrade.bin
