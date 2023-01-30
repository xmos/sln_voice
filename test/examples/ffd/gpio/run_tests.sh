#!/bin/bash
# Copyright 2021 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

set -e

# Get unix name for determining OS
UNAME=$(uname)

rm -rf testing
mkdir testing
REPORT=testing/test.rpt
FIRMWARE=example_test_ffd_gpio_test.xe
TIMEOUT_S=60

rm -f ${REPORT}

REPO_ROOT=`git rev-parse --show-toplevel`

echo "****************"
echo "* Run Tests    *"
echo "****************"
xsim --plugin LoopbackPort.dll "-port tile[0] XS1_PORT_1M 1 0 -port tile[1] XS1_PORT_1M 1 0" --plugin LoopbackPort.dll "-port tile[0] XS1_PORT_1P 1 0 -port tile[1] XS1_PORT_1P 1 0" --xscope "-offline trace.xmt" ${REPO_ROOT}/dist/${FIRMWARE} 2>&1 | tee -a ${REPORT}

echo "****************"
echo "* Parse Result *"
echo "****************"
python ${REPO_ROOT}/modules/rtos/test/rtos_drivers/python/parse_test_output.py testing/test.rpt -outfile="testing/test_results" --print_test_results --verbose

pytest
