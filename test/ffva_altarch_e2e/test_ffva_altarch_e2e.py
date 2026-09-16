#!/usr/bin/env python3
# Copyright 2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

"""Verify example_ffva_ua_adec_altarch SRAM/s16 changes still boot.

Parses the xrun/xscope report produced by run_tests.sh. Checks:
- both tiles start
- UA heaps are the reduced 96/80 KB budgets
- heaps are not exhausted
- AEC_COEFF_S16 pipeline stages construct (sizeof prints)
- no malloc failure
"""

from __future__ import annotations

import os
import re
from pathlib import Path

import pytest

REPORT = Path(__file__).resolve().parent / "testing" / "test.rpt"

TILE0_HEAP = 96 * 1024
TILE1_HEAP = 80 * 1024

PRE_PLATFORM_RE = re.compile(
    r"MEM tile (?P<tile>\d+) pre_platform heap_cfg=(?P<heap>\d+) cur_free=(?P<free>-?\d+)"
)
POST_PLATFORM_RE = re.compile(
    r"MEM tile (?P<tile>\d+) post_platform cur_free=(?P<free>-?\d+) min_free=(?P<minf>-?\d+)"
)
HEAP_CFG_RE = re.compile(
    r"MEM tile (?P<tile>\d+) heap_cfg=(?P<heap>\d+) min_free=(?P<minf>-?\d+) "
    r"cur_free=(?P<free>-?\d+) used=(?P<used>-?\d+)"
)
SIZEOF_T0_RE = re.compile(
    r"MEM tile 0 sizeof ic=(?P<ic>\d+) ns=(?P<ns>\d+) agc=(?P<agc>\d+) frame_data_t=(?P<frame>\d+)"
)
SIZEOF_T1_RE = re.compile(
    r"MEM tile 1 sizeof stage1_t=(?P<stage1>\d+) frame_data_t=(?P<frame>\d+)"
)


@pytest.fixture(scope="module")
def log_text() -> str:
    if not REPORT.is_file():
        pytest.skip(f"missing report {REPORT}; run test/ffva_altarch_e2e/run_tests.sh first")
    text = REPORT.read_text(errors="replace")
    if not text.strip():
        pytest.fail(f"empty report {REPORT}")
    return text


def test_no_malloc_failure(log_text: str) -> None:
    assert "Malloc Failed" not in log_text


def test_no_exception(log_text: str) -> None:
    upper = log_text.upper()
    assert "UNCAUGHT EXCEPTION" not in upper
    assert "ECALL" not in log_text
    assert "Aborted at" not in log_text


def test_both_tiles_started(log_text: str) -> None:
    assert "Startup task running from tile 0" in log_text
    assert "Startup task running from tile 1" in log_text


def test_reduced_ua_heap_config(log_text: str) -> None:
    pre = {int(m.group("tile")): int(m.group("heap")) for m in PRE_PLATFORM_RE.finditer(log_text)}
    assert 0 in pre, "missing tile 0 pre_platform heap_cfg"
    assert 1 in pre, "missing tile 1 pre_platform heap_cfg"
    assert pre[0] == TILE0_HEAP
    assert pre[1] == TILE1_HEAP


def test_heaps_survive_platform_start(log_text: str) -> None:
    posts = list(POST_PLATFORM_RE.finditer(log_text))
    tiles = {int(m.group("tile")) for m in posts}
    assert tiles >= {0, 1}, f"post_platform tiles={tiles}"
    for m in posts:
        assert int(m.group("free")) > 0
        assert int(m.group("minf")) > 0


def test_s16_pipeline_stages_constructed(log_text: str) -> None:
    t0 = SIZEOF_T0_RE.search(log_text)
    t1 = SIZEOF_T1_RE.search(log_text)
    assert t0, "tile 0 IC/NS/AGC sizeof print missing (audio_pipeline_init did not run)"
    assert t1, "tile 1 stage1 sizeof print missing (audio_pipeline_init did not run)"
    assert int(t0.group("ic")) > 0
    assert int(t0.group("ns")) > 0
    assert int(t0.group("agc")) > 0
    assert int(t1.group("stage1")) > 0


def test_pipeline_reached_mem_analysis(log_text: str) -> None:
    cfgs = {int(m.group("tile")): m for m in HEAP_CFG_RE.finditer(log_text)}
    if os.environ.get("FFVA_E2E_REQUIRE_PIPELINE", "0") != "1" and not cfgs:
        pytest.skip("pipeline did not reach mem_analysis (often no PDM/I2S clock)")
    assert 0 in cfgs, "tile 0 mem_analysis heap_cfg missing"
    assert 1 in cfgs, "tile 1 mem_analysis heap_cfg missing"
    assert int(cfgs[0].group("heap")) == TILE0_HEAP
    assert int(cfgs[1].group("heap")) == TILE1_HEAP
    for tile, m in cfgs.items():
        minf = int(m.group("minf"))
        used = int(m.group("used"))
        heap = int(m.group("heap"))
        assert minf > 0, f"tile {tile} heap exhausted min_free={minf}"
        assert used < heap, f"tile {tile} used={used} heap={heap}"
        assert used > 0
