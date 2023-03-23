#!/bin/bash
# Copyright (c) 2022, XMOS Ltd, All rights reserved

set -e

# help text
help()
{
   echo "XCORE-VOICE FFD commands test"
   echo
   echo "Syntax: check_commands.sh [-h] firmware input_directory input_list output_directory adapterID"
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

uname=`uname`

# assign command line args
FIRMWARE=${@:$OPTIND:1}
INPUT_DIR=${@:$OPTIND+1:1}
INPUT_LIST=${@:$OPTIND+2:1}
OUTPUT_DIR=${@:$OPTIND+3:1}
if [ ! -z "${@:$OPTIND+4:1}" ]
then
    ADAPTER_ID="--adapter-id ${@:$OPTIND+4:1}"
fi

# read input list
INPUT_ARRAY=()
while IFS= read -r line || [[ "$line" ]]; do
    if [[ ${line:0:1} != "#" ]]; then
        INPUT_ARRAY+=("$line")
    fi
done < ${INPUT_LIST}

# discern repository root
SLN_VOICE_ROOT=`git rev-parse --show-toplevel`
source ${SLN_VOICE_ROOT}/tools/ci/helper_functions.sh

# xflash erase
xflash ${ADAPTER_ID} --erase-all --target-file "${SLN_VOICE_ROOT}"/examples/ffd/bsp_config/XK_VOICE_L71/XK_VOICE_L71.xn

# flash the data partition
xflash ${ADAPTER_ID} --quad-spi-clock 50MHz --factory dist/example_ffd_usb_audio_test.xe --boot-partition-size 0x100000 --data dist/example_ffd_data_partition.bin

# wait for device to reset (may not be necessary)
sleep 3

# Create output folder
mkdir -p ${OUTPUT_DIR}

# fresh logs
RESULTS="${OUTPUT_DIR}/results.csv"
rm -rf ${RESULTS}

echo "***********************************"
echo "Log file: ${RESULTS}"
echo "***********************************"

for ((j = 0; j < ${#INPUT_ARRAY[@]}; j += 1)); do
    read -ra FIELDS <<< ${INPUT_ARRAY[j]}
    FILE_NAME=${FIELDS[0]}
    MIN=${FIELDS[1]}
    MAX=${FIELDS[2]}

    OUTPUT_LOG="${OUTPUT_DIR}/${FILE_NAME}.log"
    INPUT_WAV="${INPUT_DIR}/${FILE_NAME}.wav"
    OUTPUT_WAV="${OUTPUT_DIR}/processed_${FILE_NAME}.wav"
    
    # call xrun (in background)
    xrun ${ADAPTER_ID} --xscope ${FIRMWARE} &> ${OUTPUT_LOG} &
    XRUN_PID=$!

    # wait for app to load
    sleep 10

    # process the input wav
    (bash ${SLN_VOICE_ROOT}/tools/audio/process_wav.sh -c1 ${INPUT_WAV} ${OUTPUT_WAV})

    # wait for sox to stop
    sleep 5

    # kill xrun
    kill -INT ${XRUN_PID}

    # reset board
    xgdb -batch -ex "connect ${ADAPTER_ID} --reset-to-mode-pins" -ex detach

    # pause after reset
    sleep 3

    # count keyword occurrences in the log
    DETECTIONS=$(grep -o -I "KEYWORD:" ${OUTPUT_LOG} | wc -l)
    # trim whitespace
    DETECTIONS="${DETECTIONS//[[:space:]]/}"
    # log results
    echo "${INPUT_WAV}: ${DETECTIONS} detections"
    echo "filename=${INPUT_WAV}, detected=${DETECTIONS}, min=${MIN}, max=${MAX}" >> ${RESULTS}
done

# print results
cat ${RESULTS}
