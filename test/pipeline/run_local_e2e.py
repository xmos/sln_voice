#!/usr/bin/env python3
# Copyright 2026 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

"""Local FFVA altarch pipeline smoke test.

Generates a synthetic 4-ch WAV, runs test_pipeline_ffva_adec_altarch.xe with
xscope_host_endpoint, and checks that output.wav is valid PCM with energy.
Does not count Alexa detections (that needs amazon_wwe + hydra vectors).
"""

from __future__ import annotations

import argparse
import os
import subprocess
import sys
import time
import wave
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
DEFAULT_FIRMWARE = REPO / "build" / "test_pipeline_ffva_adec_altarch.xe"
DEFAULT_HOST = REPO / "dist_host" / "xscope_host_endpoint"
GEN = Path(__file__).resolve().parent / "generate_synthetic_wav.py"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--firmware", default=str(DEFAULT_FIRMWARE))
    parser.add_argument("--host", default=str(DEFAULT_HOST))
    parser.add_argument("--adapter-id")
    parser.add_argument("--port", type=int, default=12346)
    parser.add_argument("--outdir", default=str(REPO / "test" / "pipeline" / "local_output"))
    parser.add_argument("--seconds", type=float, default=1.5)
    parser.add_argument("--timeout", type=float, default=120.0)
    parser.add_argument("--input", help="Existing 4-ch 32-bit 16 kHz wav; skip synthetic generation")
    return parser.parse_args()


def rms_i32(path: Path, channel: int = 0) -> float:
    with wave.open(str(path), "r") as wf:
        nch = wf.getnchannels()
        nframes = wf.getnframes()
        sampwidth = wf.getsampwidth()
        raw = wf.readframes(nframes)
    if sampwidth != 4:
        raise SystemExit(f"{path} sampwidth={sampwidth}, expected 4")
    import struct

    samples = struct.unpack("<" + "i" * (len(raw) // 4), raw)
    ch = samples[channel::nch]
    if not ch:
        return 0.0
    acc = sum(float(s) * float(s) for s in ch) / len(ch)
    return acc ** 0.5


def main() -> int:
    args = parse_args()
    firmware = Path(args.firmware).resolve()
    host = Path(args.host).resolve()
    if not firmware.is_file():
        print(f"missing firmware {firmware}", file=sys.stderr)
        print("Configure with -DXCORE_VOICE_TESTS=1 -DTEST_PIPELINE=FFVA_ALT_ARCH and build test_pipeline_ffva_adec_altarch", file=sys.stderr)
        return 1
    if not host.is_file():
        print(f"missing host app {host}", file=sys.stderr)
        print("Build xscope_host_endpoint (tools/ci/build_host_apps.sh)", file=sys.stderr)
        return 1

    outdir = Path(args.outdir)
    outdir.mkdir(parents=True, exist_ok=True)
    xtc = os.environ.get("XMOS_TOOL_PATH", "/Applications/XMOS_XTC_15.3.1")
    endpoint_so = Path(xtc) / "lib" / "xscope_endpoint.so"
    if endpoint_so.is_file():
        import shutil
        shutil.copy2(endpoint_so, outdir / "xscope_endpoint.so")
    input_wav = outdir / "input.wav"
    output_wav = outdir / "output.wav"
    for leftover in (output_wav, outdir / "output.log"):
        if leftover.exists():
            leftover.unlink()

    if args.input:
        src = Path(args.input).resolve()
        if not src.is_file():
            print(f"missing input {src}", file=sys.stderr)
            return 1
        if src != input_wav:
            import shutil
            shutil.copy2(src, input_wav)
        print(f"using input {src}")
    else:
        subprocess.run(
            [sys.executable, str(GEN), "-o", str(input_wav), "--seconds", str(args.seconds)],
            check=True,
        )

    xrun_cmd = ["xrun", "--xscope", f"--xscope-port", f"localhost:{args.port}"]
    if args.adapter_id:
        xrun_cmd.extend(["--adapter-id", args.adapter_id])
    xrun_cmd.append(str(firmware))

    print(" ".join(xrun_cmd))
    xrun_live = outdir / "xrun_live.log"
    xrun_fh = open(xrun_live, "w")
    xrun = subprocess.Popen(xrun_cmd, stdout=xrun_fh, stderr=subprocess.STDOUT, text=True)
    time.sleep(8)
    if xrun.poll() is not None:
        xrun_fh.close()
        print(xrun_live.read_text(errors="replace"))
        print("xrun exited before host connected", file=sys.stderr)
        return 1

    host_cmd = [str(host), str(args.port)]
    print(" ".join(host_cmd), "cwd", outdir)
    host_env = os.environ.copy()
    xtc = host_env.get("XMOS_TOOL_PATH", "/Applications/XMOS_XTC_15.3.1")
    libdir = str(Path(xtc) / "lib")
    host_env["DYLD_LIBRARY_PATH"] = libdir + os.pathsep + host_env.get("DYLD_LIBRARY_PATH", "")
    host_env["LD_LIBRARY_PATH"] = libdir + os.pathsep + host_env.get("LD_LIBRARY_PATH", "")
    try:
        host_proc = subprocess.run(
            host_cmd,
            cwd=outdir,
            capture_output=True,
            text=True,
            timeout=args.timeout,
            env=host_env,
        )
    except subprocess.TimeoutExpired:
        xrun.terminate()
        print("xscope_host_endpoint timed out", file=sys.stderr)
        return 1
    finally:
        if xrun.poll() is None:
            xrun.terminate()
            try:
                xrun.wait(timeout=5)
            except subprocess.TimeoutExpired:
                xrun.kill()
        xrun_fh.close()

    xrun_out = xrun_live.read_text(errors="replace") if xrun_live.is_file() else ""
    log = outdir / "xrun.log"
    log.write_text(xrun_out + "\n--- host stdout ---\n" + host_proc.stdout + "\n--- host stderr ---\n" + host_proc.stderr)
    print(log.read_text()[-4000:])

    if "Malloc Failed" in xrun_out:
        print("malloc failed", file=sys.stderr)
        return 1
    if not output_wav.is_file():
        print(f"no output wav at {output_wav}", file=sys.stderr)
        return 1

    with wave.open(str(output_wav), "r") as wf:
        print(
            f"output.wav ch={wf.getnchannels()} rate={wf.getframerate()} "
            f"width={wf.getsampwidth()} frames={wf.getnframes()}"
        )
        if wf.getnchannels() != 2 or wf.getframerate() != 16000 or wf.getsampwidth() != 4:
            print("unexpected output wav format", file=sys.stderr)
            return 1
        if wf.getnframes() < 240:
            print("output wav too short", file=sys.stderr)
            return 1

    energy = rms_i32(output_wav, 0)
    print(f"output ch0 rms={energy:.1f}")
    if energy < 1.0:
        print("output looks silent; pipeline likely did not process audio", file=sys.stderr)
        return 1
    print("PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
