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
   echo "   asr_library            Sensory"
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
PIPELINE_FIRMWARE="dist/test_pipeline_ffd.xe"

# determine firmware and data partition file
if [[ ${ASR_LIBRARY} == "Sensory" ]]
then
    ASR_FIRMWARE="dist/test_asr_sensory.xe"
    DATA_PARTITION="dist/test_asr_sensory_data_partition.bin"
    TRIM_COMMAND="" # trim is not needed
    TRUTH_TRACK="${INPUT_DIR}/truth_labels.txt"
# elif [[ ${ASR_LIBRARY} == "Other" ]]
# then
#     ASR_FIRMWARE="dist/test_asr_other.xe"
#     DATA_PARTITION="dist/test_asr_other_data_partition.bin"
#     TRIM_COMMAND="trim 0 01:45" # need to trim input to account for 50 command limit  
#     TRUTH_TRACK="${INPUT_DIR}/truth_labels_1_45.txt"
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

# create output folder
mkdir -p ${OUTPUT_DIR}

# fresh logs
RESULTS="${OUTPUT_DIR}/results.csv"
rm -rf ${RESULTS}

echo "Log file: ${RESULTS}"
echo "Filename, Max_Allowable_WER, Computed_WER" >> ${RESULTS}

for ((j = 0; j < ${#INPUT_ARRAY[@]}; j += 1)); do
    read -ra FIELDS <<< ${INPUT_ARRAY[j]}
    FILE_NAME=${FIELDS[0]}
    MAX_ALLOWABLE_WER=${FIELDS[1]}

    INPUT_WAV="${INPUT_DIR}/${FILE_NAME}.wav"
    PROCESSED_WAV="${OUTPUT_DIR}/${FILE_NAME}_processed.wav"
    LABEL_TRACK="${OUTPUT_DIR}/${FILE_NAME}_labels.txt"
    PIPELINE_OUTPUT_LOG="${OUTPUT_DIR}/${FILE_NAME}_pipeline.log"
    PIPELINE_OUTPUT_CSV="${OUTPUT_DIR}/${FILE_NAME}_pipeline.csv"
    ASR_OUTPUT_LOG="${OUTPUT_DIR}/${FILE_NAME}_asr.log"
    SCORING_OUTPUT_LOG="${OUTPUT_DIR}/${FILE_NAME}_scoring.log"
    
    TEMP_XSCOPE_FILEIO_INPUT_WAV="${OUTPUT_DIR}/input.wav"
    TEMP_XSCOPE_FILEIO_OUTPUT_WAV="${OUTPUT_DIR}/output.wav"
    TEMP_XSCOPE_FILEIO_OUTPUT_LOG="${OUTPUT_DIR}/output.log"

    # ensure input file exists
    if [ ! -f "${INPUT_WAV}" ]; then
        echo "${INPUT_WAV} does not exist."
        exit 1
    fi

    # xscope_fileio can not create files so make sure TEMP_XSCOPE_FILEIO_OUTPUT_LOG exists
    touch ${TEMP_XSCOPE_FILEIO_OUTPUT_LOG}

    # *********************************
    # Process input wav w/ FFD pipeline
    # *********************************

    # remix and create input wav to the filename expected for xscope_fileio (input.wav)
    #   the input wav files are one channel so we append a silent second mic channel
    TEMP_WAV="${OUTPUT_DIR}/temp.wav"
    sox ${INPUT_WAV} --no-dither -r 16000 -b 32 ${TEMP_WAV} remix 1 2
    sox ${TEMP_WAV} ${TEMP_XSCOPE_FILEIO_INPUT_WAV} ${TRIM_COMMAND}
    rm ${TEMP_WAV}

    # call xrun (in background)
    (xrun ${ADAPTER_ID} --xscope-realtime --xscope-port localhost:12345 ${PIPELINE_FIRMWARE}) &

    # wait for app to load
    sleep 15

    # run xscope host in directory where the TEMP_XSCOPE_FILEIO_INPUT_WAV resides
    #   xscope_host_endpoint is run in a subshell (inside parentheses) so when 
    #   it exits, the xrun command above will also exit
    (cd ${OUTPUT_DIR} ; ${DIST_HOST}/xscope_host_endpoint 12345)

    # wait for xrun to exit
    sleep 1

    cp ${TEMP_XSCOPE_FILEIO_OUTPUT_LOG} ${PIPELINE_OUTPUT_LOG}

    # make pipeline csv
    python3 test/asr/make_trace_csv.py --log ${PIPELINE_OUTPUT_LOG} --csv ${PIPELINE_OUTPUT_CSV}

    # single out ASR channel from the processed output
    sox ${TEMP_XSCOPE_FILEIO_OUTPUT_WAV} ${PROCESSED_WAV} remix 1

    # *****************************************
    # Process FFD pipeline output with ASR test
    # *****************************************

    # save the processed output
    cp ${PROCESSED_WAV} ${TEMP_XSCOPE_FILEIO_INPUT_WAV}

    # call xrun (in background)
    (xrun ${ADAPTER_ID} --xscope-realtime --xscope-port localhost:12345 ${ASR_FIRMWARE}) &

    # wait for app to load
    sleep 15
    
    # run xscope host in directory where the TEMP_XSCOPE_FILEIO_INPUT_WAV resides
    #   xscope_host_endpoint is run in a subshell (inside parentheses) so when 
    #   it exits, the xrun command above will also exit
    (cd ${OUTPUT_DIR} ; ${DIST_HOST}/xscope_host_endpoint 12345)

    # wait for xrun to exit
    sleep 1

    cp ${TEMP_XSCOPE_FILEIO_OUTPUT_LOG} ${ASR_OUTPUT_LOG}

    # ******************
    # Compute WER
    # ******************

    # make label track
    python3 test/asr/make_label_track.py --log ${ASR_OUTPUT_LOG} --lut ${ASR_LIBRARY} --label_track ${LABEL_TRACK}

    # score label track
    python3 test/asr/score_label_track.py --label_track ${LABEL_TRACK} --truth_track ${TRUTH_TRACK} --log ${SCORING_OUTPUT_LOG}

    # extract WER from scoring log
    WER=$(grep "WER:" ${SCORING_OUTPUT_LOG} | cut -d' ' -f 2)
    # log results
    echo "${INPUT_WAV}, ${MAX_ALLOWABLE_WER}, ${WER}" >> ${RESULTS}

    # clean up temp 
    rm ${TEMP_XSCOPE_FILEIO_INPUT_WAV}
    rm ${TEMP_XSCOPE_FILEIO_OUTPUT_WAV}
    rm ${TEMP_XSCOPE_FILEIO_OUTPUT_LOG}
done 

# print results
cat ${RESULTS}
