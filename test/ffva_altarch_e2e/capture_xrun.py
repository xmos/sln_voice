#!/usr/bin/env python3
# Copyright 2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

"""Run example_ffva_ua_adec_altarch under xrun --xscope and capture boot logs.

The app's mem_analysis() task never exits, so capture stops when both tiles
have printed post-init heap watermarks, or when --timeout elapses.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
import time
from pathlib import Path


DONE_MARKERS = (
    "MEM tile 0 heap_cfg=",
    "MEM tile 1 heap_cfg=",
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--firmware", required=True)
    parser.add_argument("--report", required=True)
    parser.add_argument("--timeout", type=float, default=45.0)
    parser.add_argument("--adapter-id")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    cmd = ["xrun", "--xscope"]
    if args.adapter_id:
        cmd.extend(["--adapter-id", args.adapter_id])
    cmd.append(args.firmware)

    report = Path(args.report)
    report.parent.mkdir(parents=True, exist_ok=True)

    proc = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
    )
    lines: list[str] = []
    seen: set[str] = set()
    start = time.time()
    try:
        assert proc.stdout is not None
        while True:
            line = proc.stdout.readline()
            if line:
                sys.stdout.write(line)
                sys.stdout.flush()
                lines.append(line)
                for marker in DONE_MARKERS:
                    if marker in line:
                        seen.add(marker)
                if seen == set(DONE_MARKERS):
                    time.sleep(0.4)
                    break
            elif proc.poll() is not None:
                break
            if time.time() - start > args.timeout:
                break
    finally:
        if proc.poll() is not None:
            pass
        else:
            proc.terminate()
            try:
                proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait(timeout=5)
        report.write_text("".join(lines))

    if not lines:
        print("No xrun output captured", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
