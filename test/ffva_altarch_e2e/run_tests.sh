#!/bin/bash
# Copyright 2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

set -e

REPO_ROOT=$(git rev-parse --show-toplevel)
FIRMWARE="${REPO_ROOT}/build/example_ffva_ua_adec_altarch.xe"
REPORT_DIR="${REPO_ROOT}/test/ffva_altarch_e2e/testing"
REPORT="${REPORT_DIR}/test.rpt"
TIMEOUT_S="${FFVA_E2E_TIMEOUT_S:-45}"

if [ ! -f "${FIRMWARE}" ]; then
    echo "Firmware not found: ${FIRMWARE}"
    echo "Build example_ffva_ua_adec_altarch first."
    exit 1
fi

ADAPTER_ARGS=()
if [ -n "${1}" ]; then
    ADAPTER_ARGS=(--adapter-id "${1}")
elif [ -n "${XTAG_ADAPTER_ID}" ]; then
    ADAPTER_ARGS=(--adapter-id "${XTAG_ADAPTER_ID}")
fi

rm -rf "${REPORT_DIR}"
mkdir -p "${REPORT_DIR}"

echo "****************"
echo "* FFVA e2e     *"
echo "****************"
python3 "${REPO_ROOT}/test/ffva_altarch_e2e/capture_xrun.py" \
    --firmware "${FIRMWARE}" \
    --report "${REPORT}" \
    --timeout "${TIMEOUT_S}" \
    "${ADAPTER_ARGS[@]}"
