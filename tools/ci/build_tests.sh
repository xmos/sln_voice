#!/bin/bash
set -e

SLN_AVONA_ROOT=`git rev-parse --show-toplevel`

source ${SLN_AVONA_ROOT}/tools/ci/helper_functions.sh

# setup distribution folder
DIST_DIR=${SLN_AVONA_ROOT}/dist
mkdir -p ${DIST_DIR}

# setup configurations
# row format is: "name make_target BOARD toolchain"
applications=(
    "stlp_ua_adec                 application_stlp_ua_adec                 XK_VOICE_L71       xmos_cmake_toolchain/xs3a.cmake"
    "stlp_ua_adec_altarch         application_stlp_ua_adec_altarch         XK_VOICE_L71       xmos_cmake_toolchain/xs3a.cmake"
)

# perform builds
for ((i = 0; i < ${#applications[@]}; i += 1)); do
    read -ra FIELDS <<< ${applications[i]}
    name="${FIELDS[0]}"
    make_target="${FIELDS[1]}"
    board="${FIELDS[2]}"
    toolchain_file="${SLN_AVONA_ROOT}/${FIELDS[3]}"
    path="${SLN_AVONA_ROOT}"
    echo '******************************************************'
    echo '* Building' ${name}, ${make_target} 'for' ${board}
    echo '******************************************************'

    (cd ${path}; rm -rf build_${board})
    (cd ${path}; mkdir -p build_${board})
    (cd ${path}/build_${board}; log_errors cmake ../ -DCMAKE_TOOLCHAIN_FILE=${toolchain_file} -DBOARD=${board} -DDEBUG_STLP_USB_MIC_INPUT=1; log_errors make ${make_target} -j)
    (cd ${path}/build_${board}; cp ${make_target}.xe ${DIST_DIR})
done

if $CI
then
    metadata_filename="${DIST_DIR}/metadata.json"
    template='{"actor":"%s","sha":"%s"}'
    json_string=$(printf "$template" "$GITHUB_ACTOR" "$GITHUB_SHA")
    echo "$json_string" >> metadata_filename
fi