#!/usr/bin/env python3
# Copyright 2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

"""Compare a DUT WAV against a baseline golden. Not bit-exact; reports SNR."""

from __future__ import annotations

import argparse
import math
import sys
import wave
from pathlib import Path

import numpy as np


def read_i32(path: Path) -> tuple[int, int, np.ndarray]:
    with wave.open(str(path), "r") as wf:
        nch = wf.getnchannels()
        rate = wf.getframerate()
        n = wf.getnframes()
        raw = wf.readframes(n)
    data = np.frombuffer(raw, dtype="<i4").reshape(-1, nch)
    return rate, nch, data.astype(np.float64)


def snr_db(ref: np.ndarray, dut: np.ndarray) -> float:
    n = min(len(ref), len(dut))
    ref, dut = ref[:n], dut[:n]
    err = ref - dut
    p_ref = float(np.mean(ref * ref))
    p_err = float(np.mean(err * err))
    if p_err <= 0:
        return math.inf
    if p_ref <= 0:
        return -math.inf
    return 10.0 * math.log10(p_ref / p_err)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--golden", required=True)
    parser.add_argument("--dut", required=True)
    parser.add_argument("--min-snr", type=float, default=25.0)
    args = parser.parse_args()

    g_rate, g_ch, golden = read_i32(Path(args.golden))
    d_rate, d_ch, dut = read_i32(Path(args.dut))
    if g_rate != d_rate or g_ch != d_ch:
        print(f"format mismatch golden {g_rate}/{g_ch} dut {d_rate}/{d_ch}", file=sys.stderr)
        return 1
    n = min(len(golden), len(dut))
    print(f"compare n={n} ch={g_ch} rate={g_rate}")
    worst = math.inf
    for ch in range(g_ch):
        snr = snr_db(golden[:n, ch], dut[:n, ch])
        maxabs = float(np.max(np.abs(golden[:n, ch] - dut[:n, ch])))
        print(f"  ch{ch} SNR={snr:.2f} dB  max_abs={maxabs:.1f}")
        worst = min(worst, snr)
    if worst < args.min_snr:
        print(f"FAIL worst SNR {worst:.2f} < {args.min_snr}", file=sys.stderr)
        return 1
    print(f"PASS worst SNR {worst:.2f} dB")
    return 0


if __name__ == "__main__":
    sys.exit(main())
