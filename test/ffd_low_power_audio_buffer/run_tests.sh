#!/bin/bash
# Copyright 2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

set -e

REPO_ROOT=$(git rev-parse --show-toplevel)
source ${REPO_ROOT}/tools/ci/helper_functions.sh

APPLICATION=test_ffd_low_power_audio_buffer
REPORT_DIR=testing
REPORT=testing/test.rpt
TIMEOUT_S=60
TIMEOUT_EXE=$(get_timeout)

rm -rf "${REPORT_DIR}"
mkdir testing

echo "****************"
echo "* Run Tests    *"
echo "****************"
$TIMEOUT_EXE ${TIMEOUT_S}s xsim "${REPO_ROOT}/dist/${APPLICATION}.xe" 2>&1 | tee -a "${REPORT}"
