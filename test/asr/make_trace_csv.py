#!/usr/bin/env python3
# Copyright 2022-2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
# XMOS Public License: Version 1

import argparse

LINE_START = "TRACE:"

PIPELINE_BRICK_LENGTH_SAMPLES = 240
PIPELINE_BRICK_LENGTH_MS = 15

def fixed_to_float(x, e):
    c = abs(x)
    sign = 1 
    if x < 0:
        # convert back from two's complement
        c = x - 1 
        c = ~c
        sign = -1
    f = (1.0 * c) / (2 ** e)
    f = f * sign
    return f

def control_flag_to_state(f):
    if (f == 2):
        return "HOLD"
    elif (f == 1):
        return "ADAPT"
    elif (f == 0):
        return "ADAPT_SLOW"
    elif (f == -1):
        return "UNSTABLE"
    elif (f == -2):
        return "FORCE_ADAPT"
    elif (f == -3):
        return "FORCE_HOLD"

def process(log, csv):
    recognition_events = []

    # Get recognition events from log

    with open(csv, "w") as csv_fd:
        print("frame_index, frame_sec, input_vnr_pred, control_flag_e", file=csv_fd)
        with open(log, "r") as log_fd:
            for line in log_fd:
                if line.startswith(LINE_START):
                    fields = line[len(LINE_START):].strip().split(",")
                    frame_index = int(fields[0])
                    frame_sec = frame_index * PIPELINE_BRICK_LENGTH_MS / 1000
                    input_vnr_pred = fixed_to_float(int(fields[1]), 31)
                    control_flag_e = control_flag_to_state(int(fields[2]))
                    print(f"{frame_index}, {frame_sec:.3f}, {input_vnr_pred:.3f}, {control_flag_e}", file=csv_fd)

if __name__ == '__main__':
    parser = argparse.ArgumentParser('Trace Maker')
    parser.add_argument('--log_file', help='Log file to parse')
    parser.add_argument('--csv', help='CSV trace file')
    args = parser.parse_args()

    process(args.log_file, args.csv)