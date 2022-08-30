#!/bin/bash
set -e

XCORE_VOICE_ROOT=`git rev-parse --show-toplevel`

source ${XCORE_VOICE_ROOT}/tools/ci/helper_functions.sh

# setup distribution folder
DIST_DIR=${XCORE_VOICE_ROOT}/dist_host
mkdir -p ${DIST_DIR}

# perform builds
path="${XCORE_VOICE_ROOT}"
echo '******************************************************'
echo '* Building host applications'
echo '******************************************************'

(cd ${path}; rm -rf build_host)
(cd ${path}; mkdir -p build_host)
(cd ${path}/build_host; log_errors cmake ../ ; log_errors make -j)

# copy fatfs_mkimage to dist
name=fatfs/host
make_target=fatfs_mkimage
(cd ${path}/build_host; cp xcore_sdk/modules/rtos/modules/sw_services/${name}/${make_target} ${DIST_DIR})
