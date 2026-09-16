#!/usr/bin/env python3
# Copyright 2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

"""Write a 16 kHz 32-bit PCM WAV the pipeline fileio firmware will accept.

Channel order matches XCORE-VOICE after check_pipeline.sh remix:
  0: Ref L, 1: Ref R, 2: Mic 0, 3: Mic 1

Mics contain a delayed copy of the reference plus a near-end tone so AEC/IC
have something to do. This is not a speech corpus and cannot replace hydra
Alexa vectors.
"""

from __future__ import annotations

import argparse
import math
import struct
import wave
from pathlib import Path

RATE = 16000
BITS = 32
CHANNELS = 4
FRAME_ADVANCE = 240
AMP = 0.15 * 0x7FFFFFFF


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output", required=True)
    parser.add_argument("--seconds", type=float, default=1.5)
    return parser.parse_args()


def brick_aligned_frames(seconds: float) -> int:
    frames = int(RATE * seconds)
    return max(FRAME_ADVANCE, (frames // FRAME_ADVANCE) * FRAME_ADVANCE)


def sample(ch: int, n: int) -> int:
    t = n / RATE
    ref = math.sin(2 * math.pi * 500.0 * t)
    delayed = math.sin(2 * math.pi * 500.0 * ((n - 24) / RATE)) if n >= 24 else 0.0
    near = math.sin(2 * math.pi * 1200.0 * t)
    if ch == 0:
        val = ref
    elif ch == 1:
        val = 0.9 * ref
    elif ch == 2:
        val = 0.6 * delayed + 0.4 * near
    else:
        val = 0.55 * delayed + 0.45 * near
    return int(max(-1.0, min(1.0, val)) * AMP)


def main() -> None:
    args = parse_args()
    path = Path(args.output)
    path.parent.mkdir(parents=True, exist_ok=True)
    nframes = brick_aligned_frames(args.seconds)
    with wave.open(str(path), "w") as wf:
        wf.setnchannels(CHANNELS)
        wf.setsampwidth(BITS // 8)
        wf.setframerate(RATE)
        frames = bytearray()
        for n in range(nframes):
            frames.extend(
                struct.pack("<4i", sample(0, n), sample(1, n), sample(2, n), sample(3, n))
            )
        wf.writeframes(frames)
    print(f"wrote {path} frames={nframes} duration={nframes / RATE:.3f}s")


if __name__ == "__main__":
    main()
