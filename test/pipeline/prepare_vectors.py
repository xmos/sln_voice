#!/usr/bin/env python3
# Copyright 2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

"""Build 16 kHz 32-bit 4-ch pipeline inputs from in-tree WAV files.

Channel order: Ref L, Ref R, Mic 0, Mic 1.
Far-end is a delayed copy mixed into the mics so AEC has echo to cancel.
"""

from __future__ import annotations

import argparse
import wave
from pathlib import Path

import numpy as np
from scipy.io import wavfile

REPO = Path(__file__).resolve().parents[2]
RATE = 16000
FRAME = 240
DELAY = 32
ECHO = 0.35
OUTDIR = Path(__file__).resolve().parent / "vectors"


def brick_trim(n: int) -> int:
    return max(FRAME, (n // FRAME) * FRAME)


def to_i32_mono(rate: int, data: np.ndarray) -> np.ndarray:
    if data.ndim > 1:
        data = data[:, 0]
    if rate != RATE:
        # integer-only resample via linear interpolation
        n_out = int(round(len(data) * RATE / rate))
        x = np.linspace(0, len(data) - 1, n_out)
        data = np.interp(x, np.arange(len(data)), data.astype(np.float64))
    data = data.astype(np.float64)
    peak = np.max(np.abs(data)) or 1.0
    if data.dtype != np.int32 and peak <= 1.0 + 1e-6:
        data = data * 0x7FFFFFFF * 0.5
    elif peak < 2**15:
        data = data * (2**16)
    return np.clip(data, -2**31, 2**31 - 1).astype(np.int32)


def write_4ch(path: Path, frames: np.ndarray) -> None:
    """frames shape (n, 4) int32."""
    n = brick_trim(frames.shape[0])
    frames = frames[:n]
    path.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(path), "w") as wf:
        wf.setnchannels(4)
        wf.setsampwidth(4)
        wf.setframerate(RATE)
        wf.writeframes(frames.astype("<i4").tobytes())
    print(f"wrote {path} frames={n} dur={n / RATE:.2f}s")


def add_echo(mic0: np.ndarray, mic1: np.ndarray) -> np.ndarray:
    n = min(len(mic0), len(mic1))
    mic0, mic1 = mic0[:n].astype(np.int64), mic1[:n].astype(np.int64)
    ref0 = np.zeros_like(mic0)
    ref1 = np.zeros_like(mic1)
    ref0[DELAY:] = (mic0[:-DELAY] * ECHO).astype(np.int64)
    ref1[DELAY:] = (mic1[:-DELAY] * ECHO).astype(np.int64)
    y0 = np.clip(mic0 + ref0, -2**31, 2**31 - 1)
    y1 = np.clip(mic1 + ref1, -2**31, 2**31 - 1)
    return np.stack([ref0, ref1, y0, y1], axis=1).astype(np.int32)


def load_wav(path: Path) -> tuple[int, np.ndarray]:
    rate, data = wavfile.read(path)
    return rate, data


def make_ic_vector(seconds: float) -> None:
    src = REPO / "modules/voice/tests/lib_ic/py_c_frame_compare/input.wav"
    rate, data = load_wav(src)
    n = brick_trim(min(int(seconds * RATE), data.shape[0]))
    mic0 = data[:n, 0].astype(np.int32)
    mic1 = data[:n, 1].astype(np.int32)
    write_4ch(OUTDIR / "ic_room_4ch.wav", add_echo(mic0, mic1))


def make_commands_vector() -> None:
    clips = [
        REPO / "examples/ffva/filesystem_support/english_usa" / f"{i}.wav"
        for i in (1, 3, 10, 11, 12, 13)
    ]
    parts = []
    for p in clips:
        rate, data = load_wav(p)
        parts.append(to_i32_mono(rate, data))
        parts.append(np.zeros(RATE // 5, dtype=np.int32))  # 200 ms gap
    near = np.concatenate(parts)
    write_4ch(OUTDIR / "en_commands_4ch.wav", add_echo(near, near))


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--seconds", type=float, default=3.0)
    args = parser.parse_args()
    make_ic_vector(args.seconds)
    make_commands_vector()


if __name__ == "__main__":
    main()
