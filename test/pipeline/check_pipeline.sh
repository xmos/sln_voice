#!/bin/bash
# Copyright (c) 2022, XMOS Ltd, All rights reserved

set -e

# help text
help()
{
   echo "XCORE-VOICE pipeline test"
   echo
   echo "Syntax: check_pipeline.sh [-h] input_directory output_directory sensory_directory"
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
# TODO: add parse to check for and add possible missing trailing / on dirs
INPUT_DIR=${@:$OPTIND:1}
OUTPUT_DIR=${@:$OPTIND+1:1}
SENSORY_DIR=${@:$OPTIND+2:1}

# discern repository root
SLN_VOICE_ROOT=`git rev-parse --show-toplevel`
source ${SLN_VOICE_ROOT}/tools/ci/helper_functions.sh

# sensory
uname=`uname`
if [[ "$uname" == 'Linux' ]]; then
    SENSORY_EXE="${SENSORY_DIR}/spot_eval_exe/spot-eval_x86_64-pc-linux-gnu"
elif [[ "$uname" == 'Darwin' ]]; then
    SENSORY_EXE="${SENSORY_DIR}/spot_eval_exe/spot-eval_x86_64-apple-darwin"
fi
SENSORY_MODEL="${SENSORY_DIR}/model/spot-alexa-rpi-31000.snsr"
SENSORY_KW="alexa"

# audio filenames, min instances, max instances
QUICK_INPUT_FILES=(
    "InHouse_XVF3510v080_v1.2_20190423_Loc1_Clean_XMOS_DUT1_80dB_Take1           24      24"
    "InHouse_XVF3510v080_v1.2_20190423_Loc1_Noise1_65dB_XMOS_DUT1_80dB_Take1     22      24"
    "InHouse_XVF3510v080_v1.2_20190423_Loc1_Noise2_70dB__Take1                   21      25"
    "InHouse_XVF3510v080_v1.2_20190423_Loc2_Noise1_65dB__Take1                   24      25"
)

# Create output folder
mkdir -p ${OUTPUT_DIR}

# fresh logs
RESULTS="${OUTPUT_DIR}/results.csv"
rm -rf ${RESULTS}

echo "***********************************"
echo "Log file: ${RESULTS}"
echo "***********************************"

for ((j = 0; j < ${#QUICK_INPUT_FILES[@]}; j += 1)); do
    read -ra FIELDS <<< ${QUICK_INPUT_FILES[j]}
    FILE_NAME=${FIELDS[0]}
    MIN=${FIELDS[1]}
    MAX=${FIELDS[2]}

    OUTPUT_LOG="${OUTPUT_DIR}/${FILE_NAME}.log"
    
    # process .wavs with Sox
    INPUT_WAV="${INPUT_DIR}/${FILE_NAME}.wav"
    OUTPUT_WAV="${OUTPUT_DIR}/processed_${FILE_NAME}.wav"
    MONO_OUTPUT_WAV="${OUTPUT_DIR}/mono_${FILE_NAME}.wav"

    # process the wav
    (bash ${SLN_VOICE_ROOT}/tools/audio/process_wav.sh -c4 ${INPUT_WAV} ${OUTPUT_WAV})

    # single out ASR channel
    (sox ${OUTPUT_WAV} ${MONO_OUTPUT_WAV} remix 1)

    # use Sensory to generate logs of keyword detection
    (${SENSORY_EXE} -t ${SENSORY_MODEL} -s operating-point=5 -v ${MONO_OUTPUT_WAV} 2>&1 | tee ${OUTPUT_LOG})

    # count keyword occurrences in the log
    DETECTIONS=$(grep -o -I ${SENSORY_KW} ${OUTPUT_LOG} | wc -l)
    # trim whitespace
    DETECTIONS="${DETECTIONS//[[:space:]]/}"
    # log results
    (echo "filename=${INPUT_WAV}, keyword=${SENSORY_KW}, detected=${DETECTIONS}, min=${MIN}, max=${MAX}" >> ${RESULTS})
done 

(cat ${RESULTS})
