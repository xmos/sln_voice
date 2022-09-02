#!/bin/bash
set -e

help()
{
   echo "XCORE-VOICE wav file processor"
   echo
   echo "Syntax: process_wav.sh [-c|h] to_device.wav from_device.wav"
   echo "options:"
   echo "h     Print this Help."
   echo "c     Number of channels in input wav"
   echo "a     Audio pipeline includes AEC"
   echo
}

# flag arguments
while getopts c:ah option
do
    case "${option}" in
        c) CHANNELS=${OPTARG};;
        a) AEC='true';;
        h) help
           exit;;
    esac
done

# positional arguments
INPUT_FILE=${@:$OPTIND:1}
OUTPUT_FILE=${@:$OPTIND+1:1}

# determine driver & device
uname=`uname`
if [[ "$uname" == 'Linux' ]]; then
    DEVICE_DRIVER="alsa"
    DEVICE_NAME="hw:CARD=XCOREVOICE,DEV=0"
elif [[ "$uname" == 'Darwin' ]]; then
    DEVICE_DRIVER="coreaudio"
    DEVICE_NAME="XCORE-VOICE"
fi

# determine input remix pattern
#  the standard test vector input channel order is: Mic 1, Mic 0, Ref L, Ref R
#
#  XCORE-VOICE's input channel order is: Ref L, Ref R, Mic 0, Mic 1
#  XCORE-VOICE's output channel order is: ASR, Comms, Ref L, Ref R, Mic 0, Mic 1
#  XVF3510 output channel order is: Ref L, Ref R, Mic 1, Mic 0, ASR, Comms
if [[ $CHANNELS == 1 ]]; then # reference-less test vector
    # file only has 1 microphone channel
    if [[ $AEC == true ]]; then
        #   need to insert 2 silent reference channels and repeat microphone channel
        REMIX_PATTERN="remix 0 0 1 1"
    else
        #   need to repeat microphone channel
        REMIX_PATTERN="remix 1 1"
    fi
elif [[ "$CHANNELS" == 2 ]]; then # reference-less test vector
    # file only has microphone channels
    if [[ $AEC == true ]]; then
        # need to insert 2 silent reference channels
        REMIX_PATTERN="remix 0 0 2 1"
    else
        # just include mic channels
        REMIX_PATTERN="remix 2 1"
    fi
elif [[ $CHANNELS == 4 ]]; then # standard test vector, just include mic channels
    if [[ $AEC == true ]]; then
        # reorder mic and reference channels
        REMIX_PATTERN="remix 3 4 2 1"
    else
        # just include mic channels
        REMIX_PATTERN="remix 2 1"
    fi
elif [[ $CHANNELS == 6 ]]; then  # assuming test vector from XCORE-VOICE
    if [[ $AEC == true ]]; then
        REMIX_PATTERN="remix 3 4 5 6"
    else
        # just include mic channels
        REMIX_PATTERN="remix 5 6"
    fi
else
    REMIX_PATTERN=""
fi

# call sox pipelines
SOX_PLAY_OPTS="--buffer=65536 --rate=16000 --bits=16 --encoding=signed-integer --endian=little --no-dither"
SOX_REC_OPTS="--buffer=65536 --channels=6 --rate=16000 --bits=16 --encoding=signed-integer --endian=little --no-dither"

# start recording
sox -t $DEVICE_DRIVER "$DEVICE_NAME" $SOX_REC_OPTS -t wav $OUTPUT_FILE &

# play input
sox $INPUT_FILE $SOX_PLAY_OPTS -t wav - $REMIX_PATTERN | sox -t wav - -t $DEVICE_DRIVER "$DEVICE_NAME"

# kill recording
pkill -P $$
wait #ing around to die
