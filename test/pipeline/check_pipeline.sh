#!/bin/bash
# Copyright (c) 2022, XMOS Ltd, All rights reserved

set -e

# help text
help()
{
   echo "XCORE-VOICE pipeline test"
   echo
   echo "Syntax: check_pipeline.sh [-h] firmware input_directory input_list output_directory amazon_wwe_directory adapterID"
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
AMAZON_DIR=${@:$OPTIND+4:1}
if [ ! -z "${@:$OPTIND+5:1}" ]
then
    ADAPTER_ID="--adapter-id ${@:$OPTIND+5:1}"
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

AMAZON_EXE="x86/amazon_ww_filesim"
AMAZON_MODEL="models/common/WR_250k.en-US.alexa.bin"
AMAZON_WAV="amazon_ww_input.wav"
AMAZON_THRESH="500"

# xflash erase
xflash ${ADAPTER_ID} --erase-all --target-file "${SLN_VOICE_ROOT}"/examples/ffd/bsp_config/XK_VOICE_L71/XK_VOICE_L71.xn

# flash the filesystem
if [[ ${FIRMWARE} == *"example_ffd_usb_audio_test"* ]]
then
    # build_tests.sh creates example_ffd_fat.fs used here
    xflash ${ADAPTER_ID} --quad-spi-clock 50MHz --factory dist/example_ffd_usb_audio_test.xe --boot-partition-size 0x100000 --data dist/example_ffd_fat.fs
elif [[ ${FIRMWARE} == *"example_ffva_ua_adec_test"* ]]
then
    # build_tests.sh creates example_ffva_ua_adec_fat.fs used here
    xflash ${ADAPTER_ID} --quad-spi-clock 50MHz --factory dist/example_ffva_ua_adec_test.xe --boot-partition-size 0x100000 --data dist/example_ffva_ua_adec_fat.fs
fi

# wait for device to reset (may not be necessary)
sleep 3

# Create output folder
mkdir -p ${OUTPUT_DIR}

# fresh logs
RESULTS="${OUTPUT_DIR}/results.csv"
rm -rf ${RESULTS}

# fresh list.txt for amazon_ww_filesim
rm -f "${OUTPUT_DIR}/list.txt"
echo "${AMAZON_WAV}" >> "${OUTPUT_DIR}/list.txt"

# call xrun (in background)
xrun ${ADAPTER_ID} --xscope ${FIRMWARE} &
XRUN_PID=$!

# wait for app to load
(sleep 10)

echo "***********************************"
echo "Log file: ${RESULTS}"
echo "***********************************"

for ((j = 0; j < ${#INPUT_ARRAY[@]}; j += 1)); do
    read -ra FIELDS <<< ${INPUT_ARRAY[j]}
    FILE_NAME=${FIELDS[0]}
    AEC=${FIELDS[1]}
    MIN=${FIELDS[2]}
    MAX=${FIELDS[3]}

    # determine AEC flag
    if [ "${AEC}" == "Y" ] ; then
        AEC_FLAG="-a"
    else
        AEC_FLAG=""
    fi

    OUTPUT_LOG="${OUTPUT_DIR}/${FILE_NAME}.log"
    INPUT_WAV="${INPUT_DIR}/${FILE_NAME}.wav"
    OUTPUT_WAV="${OUTPUT_DIR}/processed_${FILE_NAME}.wav"
    MONO_OUTPUT_WAV="${OUTPUT_DIR}/mono_${FILE_NAME}.wav"
    
    # process the input wav
    (bash ${SLN_VOICE_ROOT}/tools/audio/process_wav.sh -c4 ${AEC_FLAG} ${INPUT_WAV} ${OUTPUT_WAV})

    # single out ASR channel
    sox ${OUTPUT_WAV} ${MONO_OUTPUT_WAV} remix 1

    # check wakeword detections
    cp ${MONO_OUTPUT_WAV} ${OUTPUT_DIR}/${AMAZON_WAV}
    if [ "$uname" == "Linux" ] ; then
        (${AMAZON_DIR}/${AMAZON_EXE} -t ${AMAZON_THRESH} -m ${AMAZON_DIR}/${AMAZON_MODEL} ${OUTPUT_DIR}/list.txt 2>&1 | tee ${OUTPUT_LOG})
    elif [ "$uname" == "Darwin" ] ; then
        # use dockerized amazon_ww_filesim to generate logs of keyword detection
        (docker run --rm -v ${AMAZON_DIR}:/ww -v ${OUTPUT_DIR}:/input -w /input debian:buster-slim /ww/${AMAZON_EXE} -t ${AMAZON_THRESH} -m /ww/${AMAZON_MODEL} list.txt 2>&1 | tee ${OUTPUT_LOG})
    fi    

    # count keyword occurrences in the log
    DETECTIONS=$(grep -o -I "'ALEXA' detected" ${OUTPUT_LOG} | wc -l)
    # trim whitespace
    DETECTIONS="${DETECTIONS//[[:space:]]/}"
    # log results
    echo "filename=${INPUT_WAV}, keyword=alexa, detected=${DETECTIONS}, min=${MIN}, max=${MAX}" >> ${RESULTS}
done 

# kill xrun
kill -INT ${XRUN_PID}

# clean up
rm "${OUTPUT_DIR}/list.txt"
rm "${OUTPUT_DIR}/${AMAZON_WAV}"

# print results
cat ${RESULTS}