#!/bin/bash
# Copyright (c) 2022, XMOS Ltd, All rights reserved
set -e # exit on first error
set -x # echo on

# help text
help()
{
   echo "XCORE-VOICE Sample Rate Conversion test"
   echo
   echo "Syntax: check_sample_rate_conversion.sh [-h] adapterID"
   echo 
   echo "Options:"
   echo "   h     Print this Help."
}

# flag arguments
while getopts h option
do
    case "${option}" in
        h) help
           exit;;
    esac
done

# assign vars
FIRMWARE="dist/test_ffva_sample_rate_conv.xe"
DATA_PARTITION="dist/example_ffva_ua_adec_altarch_data_partition.bin"
OUTPUT_DIR=test/sample_rate_conversion/test_output
if [ ! -z "${@:$OPTIND:1}" ]
then
    ADAPTER_ID="--adapter-id ${@:$OPTIND:1}"
fi

# discern repository root
SLN_VOICE_ROOT=`git rev-parse --show-toplevel`

# Create output folder
mkdir -p ${OUTPUT_DIR}

# make sine wave input file
INPUT_WAV=${OUTPUT_DIR}/"sample_rate_conversion_input.wav"
TMP_CH1_WAV=${OUTPUT_DIR}/"sample_rate_conversion_input_ch1.wav"
TMP_CH2_WAV=${OUTPUT_DIR}/"sample_rate_conversion_input_ch2.wav"
SAMPLE_RATE="48000"
LENGTH="10"
VOLUME="0.5"
sox --null --channels=1 --bits=16 --rate=${SAMPLE_RATE} ${TMP_CH1_WAV} synth ${LENGTH} sine 1000 vol ${VOLUME}
sox --null --channels=1 --bits=16 --rate=${SAMPLE_RATE} ${TMP_CH2_WAV} synth ${LENGTH} sine 2000 vol ${VOLUME}
sox --combine merge ${TMP_CH1_WAV} ${TMP_CH2_WAV} ${INPUT_WAV}

# flash the data partition
xflash ${ADAPTER_ID} --quad-spi-clock 50MHz --factory ${FIRMWARE} --boot-partition-size 0x100000 --data ${DATA_PARTITION}

# wait for device to reset (may not be necessary)
sleep 3

# call xrun (in background)
xrun --xscope ${ADAPTER_ID} ${FIRMWARE} &
XRUN_PID=$!

# wait for app to load and enumerate on host
sleep 15

# process sine wave input file
OUTPUT_WAV=${OUTPUT_DIR}/"sample_rate_conversion_output.wav"
bash ${SLN_VOICE_ROOT}/tools/audio/process_wav.sh -a -c2 -r${SAMPLE_RATE} ${INPUT_WAV} ${OUTPUT_WAV}

# wait for process_wav.sh to fully die
sleep 1

# kill xrun
kill -INT ${XRUN_PID}

# clean up
rm ${TMP_CH1_WAV}
rm ${TMP_CH2_WAV}

echo "Arguments for pytest:"
echo "  OUTPUT_WAV="${OUTPUT_WAV}
echo "  LENGTH="${LENGTH}

# reset board for the next test
xgdb -batch -ex "connect ${ADAPTER_ID} --reset-to-mode-pins" -ex detach
