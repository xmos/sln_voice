#!/bin/bash
set -e

XCORE_VOICE_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_VOICE_ROOT}/tools/ci/helper_functions.sh

# setup distribution folder
DIST_DIR=${XCORE_VOICE_ROOT}/dist
mkdir -p ${DIST_DIR}

# setup configurations
# row format is: "name make_target flag BOARD toolchain"
examples=(
    "ffd_usb_audio            example_ffd_usb_audio_test           NONE                       XK_VOICE_L71   xmos_cmake_toolchain/xs3a.cmake"
    "ffd_usb_audio_bypass_ap  example_ffd_usb_audio_test_bypass_ap NONE                       XK_VOICE_L71   xmos_cmake_toolchain/xs3a.cmake"
    "stlp_ua_adec             example_stlp_ua_adec                 DEBUG_STLP_USB_MIC_INPUT   XK_VOICE_L71   xmos_cmake_toolchain/xs3a.cmake"
    "stlp_ua_adec_altarch     example_stlp_ua_adec_altarch         DEBUG_STLP_USB_MIC_INPUT   XK_VOICE_L71   xmos_cmake_toolchain/xs3a.cmake"
    "stlp_sample_rate_conv  example_stlp_ua_adec   DEBUG_STLP_USB_MIC_INPUT_PIPELINE_BYPASS  XK_VOICE_L71  xmos_cmake_toolchain/xs3a.cmake"
)

# perform builds
for ((i = 0; i < ${#examples[@]}; i += 1)); do
    read -ra FIELDS <<< ${examples[i]}
    name="${FIELDS[0]}"
    make_target="${FIELDS[1]}"
    flag="${FIELDS[2]}"
    board="${FIELDS[3]}"
    toolchain_file="${XCORE_VOICE_ROOT}/${FIELDS[4]}"
    path="${XCORE_VOICE_ROOT}"
    echo '******************************************************'
    echo '* Building' ${name}, ${make_target} 'for' ${board}
    echo '******************************************************'

    if [ "${flag}" = "NONE" ]; then
        optional_cache_entry=""
    else
        optional_cache_entry="-D${flag}=1"
    fi

    (cd ${path}; rm -rf build_${board})
    (cd ${path}; mkdir -p build_${board})
    (cd ${path}/build_${board}; log_errors cmake ../ -DCMAKE_TOOLCHAIN_FILE=${toolchain_file} -DBOARD=${board} -DENABLE_ALL_STLP_PIPELINES=1 ${optional_cache_entry}; log_errors make ${make_target} -j)
    (cd ${path}/build_${board}; cp ${make_target}.xe ${DIST_DIR}/example_${name}_test.xe)
done
