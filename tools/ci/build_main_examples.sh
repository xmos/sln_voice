#!/bin/bash
set -e

XCORE_VOICE_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_VOICE_ROOT}/tools/ci/helper_functions.sh

# setup distribution folder
DIST_DIR=${XCORE_VOICE_ROOT}/dist
DIST_HOST_DIR=${XCORE_VOICE_ROOT}/dist_host
mkdir -p ${DIST_DIR}

if [ -d "${DIST_HOST_DIR}" ]; then
    # add DIST_HOST_DIR to path.
    #   This is used in CI for fatfs_mkimage
    PATH="${DIST_HOST_DIR}":$PATH
    find ${DIST_HOST_DIR} -type f -exec chmod a+x {} +
fi

# setup configurations
# row format is: "name app_target run_fs_target BOARD toolchain"
examples=(
    "stlp_int_adec           example_stlp_int_adec           Yes  XK_VOICE_L71        xmos_cmake_toolchain/xs3a.cmake"
    "stlp_int_fixed_delay    example_stlp_int_fixed_delay    Yes  XK_VOICE_L71        xmos_cmake_toolchain/xs3a.cmake"
    "stlp_ua_adec            example_stlp_ua_adec            Yes  XK_VOICE_L71        xmos_cmake_toolchain/xs3a.cmake"
    "ffd                     example_ffd                     Yes  XK_VOICE_L71        xmos_cmake_toolchain/xs3a.cmake"
)

# perform builds
for ((i = 0; i < ${#examples[@]}; i += 1)); do
    read -ra FIELDS <<< ${examples[i]}
    name="${FIELDS[0]}"
    app_target="${FIELDS[1]}"
    run_fs_target="${FIELDS[2]}"
    board="${FIELDS[3]}"
    toolchain_file="${XCORE_VOICE_ROOT}/${FIELDS[4]}"
    path="${XCORE_VOICE_ROOT}"
    echo '******************************************************'
    echo '* Building' ${name}, ${app_target} 'for' ${board}
    echo '******************************************************'

    (cd ${path}; rm -rf build_${board})
    (cd ${path}; mkdir -p build_${board})
    (cd ${path}/build_${board}; log_errors cmake ../ -DCMAKE_TOOLCHAIN_FILE=${toolchain_file} -DBOARD=${board} -DENABLE_ALL_STLP_PIPELINES=1; log_errors make ${app_target} -j)
    (cd ${path}/build_${board}; cp ${app_target}.xe ${DIST_DIR})
    if [ "$run_fs_target" = "Yes" ]; then
        echo '======================================================'
        echo '= Making filesystem for' ${app_target}
        echo '======================================================'
        (cd ${path}/build_${board}; log_errors make make_fs_${app_target} -j)
        (cd ${path}/build_${board}; cp ${app_target}_fat.fs ${DIST_DIR})
    fi    
done
