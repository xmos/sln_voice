#!/bin/bash
# Copyright (c) 2022, XMOS Ltd, All rights reserved

set -e

# help text
help()
{
   echo "XCORE-VOICE Sample Rate COnversion test"
   echo
   echo "Syntax: check_commands.sh [-h] firmware output_directory"
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
OUTPUT_DIR=${@:$OPTIND+1:1}

# discern repository root
SLN_VOICE_ROOT=`git rev-parse --show-toplevel`
source ${SLN_VOICE_ROOT}/tools/ci/helper_functions.sh

# Create output folder
mkdir -p ${OUTPUT_DIR}

# make sine wave input file
INPUT_WAV=${OUTPUT_DIR}/"sample_rate_conversion_input.wav"
TMP_CH1_WAV=${OUTPUT_DIR}/"sample_rate_conversion_input_ch1.wav"
TMP_CH2_WAV=${OUTPUT_DIR}/"sample_rate_conversion_input_ch2.wav"
INPUT_FILE=${OUTPUT_DIR}/"sample_rate_conversion_input.wav"
SAMPLE_RATE="48000"
LENGTH="30"
VOLUME="0.5"
sox --null --channels=1 --bits=16 --rate=${SAMPLE_RATE} ${TMP_CH1_WAV} synth ${LENGTH} sine 1000 vol ${VOLUME}
sox --null --channels=1 --bits=16 --rate=${SAMPLE_RATE} ${TMP_CH2_WAV} synth ${LENGTH} sine 2000 vol ${VOLUME}
sox --combine merge ${TMP_CH1_WAV} ${TMP_CH2_WAV} ${INPUT_WAV}

# call xrun (in background)
xrun --xscope ${FIRMWARE} &
XRUN_PID=$!

# wait for app to load
sleep 10

# process sine wave input file
OUTPUT_WAV=${OUTPUT_DIR}/"sample_rate_conversion_output.wav"

(bash ${SLN_VOICE_ROOT}/tools/audio/process_wav.sh -a -c2 -r${SAMPLE_RATE} ${INPUT_WAV} ${OUTPUT_WAV})

# kill xrun
pkill -P ${XRUN_PID}

# clean up
rm ${TMP_CH1_WAV}
rm ${TMP_CH2_WAV}

#TODO:
# check output file for correct frequency
#   needs pytest so need install of python environment and instructions




        