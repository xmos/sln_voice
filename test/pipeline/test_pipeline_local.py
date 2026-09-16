#!/usr/bin/env python3
# Copyright 2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

"""Checks the synthetic-vector pipeline smoke output from run_local_e2e.py."""

from __future__ import annotations

import struct
import wave
from pathlib import Path

import pytest

OUTPUT = Path(__file__).resolve().parent / "local_output" / "output.wav"


def test_local_pipeline_output_exists() -> None:
    if not OUTPUT.is_file():
        pytest.skip(f"missing {OUTPUT}; run test/pipeline/run_local_e2e.py first")


def test_local_pipeline_output_format() -> None:
    if not OUTPUT.is_file():
        pytest.skip(f"missing {OUTPUT}")
    with wave.open(str(OUTPUT), "r") as wf:
        assert wf.getnchannels() == 2
        assert wf.getframerate() == 16000
        assert wf.getsampwidth() == 4
        assert wf.getnframes() >= 240


def test_local_pipeline_output_has_energy() -> None:
    if not OUTPUT.is_file():
        pytest.skip(f"missing {OUTPUT}")
    with wave.open(str(OUTPUT), "r") as wf:
        raw = wf.readframes(wf.getnframes())
        nch = wf.getnchannels()
    samples = struct.unpack("<" + "i" * (len(raw) // 4), raw)
    ch0 = samples[0::nch]
    rms = (sum(float(s) * float(s) for s in ch0) / len(ch0)) ** 0.5
    assert rms > 1.0
