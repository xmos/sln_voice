#!/bin/bash
# Copyright 2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

set -e

# Get unix name for determining OS
UNAME=$(uname)

REPO_ROOT=$(git rev-parse --show-toplevel)
APPLICATION=test_ffd_low_power_audio_buffer
REPORT_DIR=testing
REPORT=testing/test.rpt
TIMEOUT_S=60

rm -rf "${REPORT_DIR}"
mkdir testing

echo "****************"
echo "* Run Tests    *"
echo "****************"
if [ "$UNAME" == "Linux" ] ; then
    timeout ${TIMEOUT_S}s xsim "${REPO_ROOT}/dist/example_${APPLICATION}_test.xe" 2>&1 | tee -a "${REPORT}"
elif [ "$UNAME" == "Darwin" ] ; then
    gtimeout ${TIMEOUT_S}s xsim "${REPO_ROOT}/dist/example_${APPLICATION}_test.xe" 2>&1 | tee -a "${REPORT}"
fi

echo "****************"
echo "* Parse Result *"
echo "****************"
pytest
