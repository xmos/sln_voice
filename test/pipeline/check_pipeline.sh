#!/bin/bash
# Copyright (c) 2022, XMOS Ltd, All rights reserved
set -e # exit on first error
set -x # echo on

# help text
help()
{
   echo "XCORE-VOICE pipeline test"
   echo
   echo "Syntax: check_pipeline.sh [-h] firmware input_directory input_list output_directory amazon_wwe_directory adapterID"
   echo
   echo "Arguments:"
   echo "   firmware               Absolute path to .xe file"
   echo "   input_directory        Absolute path to directory with test vectors"
   echo "   input_list             Absolute path test vector input list file"
   echo "   output_directory       Absolute path to output directory"
   echo "   amazon_wwe_directory   Absolute path to amazon_wwe repository"
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

DIST_HOST="${SLN_VOICE_ROOT}/dist_host"

AMAZON_EXE="x86/amazon_ww_filesim"
AMAZON_MODEL="models/common/WR_250k.en-US.alexa.bin"
AMAZON_WAV="amazon_ww_input.wav"
AMAZON_THRESH="500"

# Create output folder
mkdir -p ${OUTPUT_DIR}

# fresh logs
RESULTS="${OUTPUT_DIR}/results.csv"
rm -rf ${RESULTS}

# fresh list.txt for amazon_ww_filesim
rm -f "${OUTPUT_DIR}/list.txt"
echo "${AMAZON_WAV}" >> "${OUTPUT_DIR}/list.txt"

echo "***********************************"
echo "Log file: ${RESULTS}"
echo "***********************************"

for ((j = 0; j < ${#INPUT_ARRAY[@]}; j += 1)); do
    read -ra FIELDS <<< ${INPUT_ARRAY[j]}
    FILE_NAME=${FIELDS[0]}
    AEC=${FIELDS[1]}
    MIN=${FIELDS[2]}
    MAX=${FIELDS[3]}

    # determine input remix pattern
    #  the standard test vector input channel order is: Mic 1, Mic 0, Ref L, Ref R
    #  XCORE-VOICE's input channel order is: Ref L, Ref R, Mic 0, Mic 1
    if [ "${AEC}" == "Y" ] ; then
        # NOTE: we are remixing the mics in [Mic 1, Mic 0] order to be consistent
        #       with the order used in fwk_voice testing
        REMIX_PATTERN="remix 3 4 1 2"
    else
        REMIX_PATTERN="remix 1 2"
    fi


    OUTPUT_LOG="${OUTPUT_DIR}/${FILE_NAME}.log"
    INPUT_WAV="${INPUT_DIR}/${FILE_NAME}.wav"
    OUTPUT_WAV="${OUTPUT_DIR}/processed_${FILE_NAME}.wav"
    XSCOPE_FILEIO_INPUT_WAV="${OUTPUT_DIR}/input.wav"
    XSCOPE_FILEIO_OUTPUT_WAV="${OUTPUT_DIR}/output.wav"

    # ensure input file exists
    if [ ! -f "${INPUT_WAV}" ]; then
        echo "${INPUT_WAV} does not exist."
        exit 1
    fi

    # remix and create input wav to the filename expected for xscope_fileio (input.wav)
    sox ${INPUT_WAV} --no-dither -r 16000 -b 32 ${XSCOPE_FILEIO_INPUT_WAV} ${REMIX_PATTERN}

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

    # the firmware saves output.wav, rename to the desired output name
    cp ${XSCOPE_FILEIO_OUTPUT_WAV} ${OUTPUT_WAV}

    # check wakeword detections
    # amazon_ww_filesim wants a 16bit, single channel input file
    sox ${OUTPUT_WAV} --no-dither -b 16 ${OUTPUT_DIR}/${AMAZON_WAV} remix 1
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

    # clean up
    rm "${OUTPUT_DIR}/${AMAZON_WAV}"
    rm ${XSCOPE_FILEIO_INPUT_WAV}
    rm ${XSCOPE_FILEIO_OUTPUT_WAV}
done 

# clean up
rm "${OUTPUT_DIR}/list.txt"

# print results
cat ${RESULTS}
