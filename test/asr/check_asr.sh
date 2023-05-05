#!/bin/bash
# Copyright (c) 2022, XMOS Ltd, All rights reserved
set -e # exit on first error
set -x # echo on

# help text
help()
{
   echo "XCORE-VOICE ASR test"
   echo
   echo "Syntax: check_asr.sh [-h] firmware input_directory input_list output_directory adapterID"
   echo
   echo "Arguments:"
   echo "   asr_library            "Sensory" or "Wanson
   echo "   input_directory        Absolute path to directory with test vectors"
   echo "   input_list             Absolute path test vector input list file"
   echo "   output_directory       Absolute path to output directory"
   echo "   adapterID              Optional XTAG adaptor ID"
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

uname=`uname`

# assign command line args
ASR_LIBRARY=${@:$OPTIND:1}
INPUT_DIR=${@:$OPTIND+1:1}
INPUT_LIST=${@:$OPTIND+2:1}
OUTPUT_DIR=${@:$OPTIND+3:1}
if [ ! -z "${@:$OPTIND+4:1}" ]
then
    ADAPTER_ID="--adapter-id ${@:$OPTIND+4:1}"
fi

# discern repository root
SLN_VOICE_ROOT=`git rev-parse --show-toplevel`
DIST_HOST="${SLN_VOICE_ROOT}/dist_host"

# determine firmware and data partition file
if [[ ${ASR_LIBRARY} == "Sensory" ]]
then
    ASR_FIRMWARE="dist/test_asr_sensory.xe"
    DATA_PARTITION="dist/test_asr_sensory_data_partition.bin"
elif [[ ${ASR_LIBRARY} == "Wanson" ]]
then
    ASR_FIRMWARE="dist/test_asr_wanson.xe"
    DATA_PARTITION="dist/test_asr_wanson_data_partition.bin"
fi

# flash the data partition file
xflash ${ADAPTER_ID} --quad-spi-clock 50MHz --factory ${ASR_FIRMWARE} --boot-partition-size 0x100000 --data ${DATA_PARTITION}

# read input list
INPUT_ARRAY=()
while IFS= read -r line || [[ "$line" ]]; do
    if [[ ${line:0:1} != "#" ]]; then
        INPUT_ARRAY+=("$line")
    fi
done < ${INPUT_LIST}

DIST_HOST="dist_host"
PIPELINE_FIRMWARE="dist/test_pipeline_ffd.xe"

# create output folder
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

    INPUT_WAV="${INPUT_DIR}/${FILE_NAME}.wav"
    PROCESSED_WAV="${OUTPUT_DIR}/${FILE_NAME}_processed.wav"
    OUTPUT_LOG="${OUTPUT_DIR}/${FILE_NAME}.log"
    
    XSCOPE_FILEIO_INPUT_WAV="${OUTPUT_DIR}/input.wav"
    XSCOPE_FILEIO_OUTPUT_WAV="${OUTPUT_DIR}/output.wav"
    XSCOPE_FILEIO_OUTPUT_LOG="${OUTPUT_DIR}/output.log"

    # ensure input file exists
    if [ ! -f "${INPUT_WAV}" ]; then
        echo "${INPUT_WAV} does not exist."
        exit 1
    fi

    # *********************************
    # Process input wav w/ FFD pipeline
    # *********************************

    # remix and create input wav to the filename expected for xscope_fileio (input.wav)
    #   the input wav files are one channel so we append a silent second mic channel
    sox --no-dither ${INPUT_WAV} ${XSCOPE_FILEIO_INPUT_WAV} remix 1 0

    # call xrun (in background)
    xrun ${ADAPTER_ID} --xscope-realtime --xscope-port localhost:12345 ${PIPELINE_FIRMWARE} &

    # wait for app to load
    sleep 10

    # run xscope host in directory where the XSCOPE_FILEIO_INPUT_WAV resides
    #   xscope_host_endpoint is run in a subshell (inside parentheses) so when 
    #   it exits, the xrun command above will also exit
    (cd ${OUTPUT_DIR} ; ${DIST_HOST}/xscope_host_endpoint 12345)

    # wait for xrun to exit
    sleep 1

    # *****************************************
    # Process FFD pipeline output with ASR test
    # *****************************************

    # save the processed output
    cp ${XSCOPE_FILEIO_OUTPUT_WAV} ${PROCESSED_WAV}

    # xscope_file can not create files so make sure XSCOPE_FILEIO_OUTPUT_LOG exists
    touch ${XSCOPE_FILEIO_OUTPUT_LOG}

    # single out ASR channel from the processed output
    sox --no-dither ${PROCESSED_WAV} ${XSCOPE_FILEIO_INPUT_WAV} remix 1

    # call xrun (in background)
    xrun ${ADAPTER_ID} --xscope-realtime --xscope-port localhost:12345 ${ASR_FIRMWARE} &

    # wait for app to load
    sleep 10
    
    # run xscope host in directory where the XSCOPE_FILEIO_INPUT_WAV resides
    #   xscope_host_endpoint is run in a subshell (inside parentheses) so when 
    #   it exits, the xrun command above will also exit
    (cd ${OUTPUT_DIR} ; ${DIST_HOST}/xscope_host_endpoint 12345)

    # wait for xrun to exit
    sleep 1

    cp ${XSCOPE_FILEIO_OUTPUT_LOG} ${OUTPUT_LOG}

    # ******************
    # Count recognitions
    # ******************

    # count keyword occurrences in the log
    RECOGNITIONS=$(grep -o -I "RECOGNIZED" ${OUTPUT_LOG} | wc -l)
    # trim whitespace
    RECOGNITIONS="${RECOGNITIONS//[[:space:]]/}"
    # log results
    echo "filename=${INPUT_WAV}, recognitions=${RECOGNITIONS}, min=${MIN}, max=${MAX}" >> ${RESULTS}

    # clean up
    rm ${XSCOPE_FILEIO_INPUT_WAV}
    rm ${XSCOPE_FILEIO_OUTPUT_WAV}
    rm ${XSCOPE_FILEIO_OUTPUT_LOG}
done 

# print results
cat ${RESULTS}
