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
   echo "   firmware               Absolute path to .xe file"
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
FIRMWARE=${@:$OPTIND:1}
INPUT_DIR=${@:$OPTIND+1:1}
INPUT_LIST=${@:$OPTIND+2:1}
OUTPUT_DIR=${@:$OPTIND+3:1}
if [ ! -z "${@:$OPTIND+4:1}" ]
then
    ADAPTER_ID="--adapter-id ${@:$OPTIND+5:1}"
fi

# TODO: need to flash the corerct data partition file

# read input list
INPUT_ARRAY=()
while IFS= read -r line || [[ "$line" ]]; do
    if [[ ${line:0:1} != "#" ]]; then
        INPUT_ARRAY+=("$line")
    fi
done < ${INPUT_LIST}

# discern repository root
SLN_VOICE_ROOT=`git rev-parse --show-toplevel`

DIST_HOST="${SLN_VOICE_ROOT}/dist_host"

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
    MIN=${FIELDS[2]}
    MAX=${FIELDS[3]}

    REMIX_PATTERN="remix 1"

    OUTPUT_LOG="${OUTPUT_DIR}/${FILE_NAME}.log"
    INPUT_WAV="${INPUT_DIR}/${FILE_NAME}.wav"
    XSCOPE_FILEIO_INPUT_WAV="${OUTPUT_DIR}/input.wav"
    XSCOPE_FILEIO_OUTPUT_LOG="${OUTPUT_DIR}/output.log"

    # ensure input file exists
    if [ ! -f "${INPUT_WAV}" ]; then
        echo "${INPUT_WAV} does not exist."
        exit 1
    fi

    # TODO: process INPUT_WAV with FFD pipeline test

    # remix and create input wav to the filename expected for xscope_fileio (input.wav)
    sox --no-dither ${INPUT_WAV} ${XSCOPE_FILEIO_INPUT_WAV} ${REMIX_PATTERN}

    # xscope_file can not create files so make sure XSCOPE_FILEIO_OUTPUT_LOG exists
    touch ${XSCOPE_FILEIO_OUTPUT_LOG}

    # call xrun (in background)
    xrun ${ADAPTER_ID} --xscope-realtime --xscope-port localhost:12345 ${FIRMWARE} &

    # wait for app to load
    sleep 10
    
    # run xscope host in directory where the XSCOPE_FILEIO_INPUT_WAV resides
    #   xscope_host_endpoint is run in a subshell (inside parentheses) so when 
    #   it exits, the xrun command above will also exit
    (cd ${OUTPUT_DIR} ; ${DIST_HOST}/xscope_host_endpoint 12345)

    # wait for xrun to exit
    sleep 1

    cp ${XSCOPE_FILEIO_OUTPUT_LOG} ${OUTPUT_LOG}

    # TODO: count results from OUTPUT_LOG and append to RESULTS

    # clean up
    rm ${XSCOPE_FILEIO_INPUT_WAV}
    rm ${XSCOPE_FILEIO_OUTPUT_WAV}
done 

# print results
cat ${RESULTS}
