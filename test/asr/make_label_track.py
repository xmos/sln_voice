#!/usr/bin/env python3
# Copyright 2022-2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
# XMOS Public License: Version 1

import argparse

LINE_START = "RECOGNIZED:"

PIPELINE_BRICK_LENGTH_SAMPLES = 240
PIPELINE_BRICK_LENGTH_MS = 15

SENSORY_LUT = {
    1: "Switch on the TV",
    2: "Channel up",
    3: "Channel down",
    4: "Volume up",
    5: "Volume down",
    6: "Switch off the TV",
    7: "Switch on the lights",
    8: "Brightness up",
    9: "Brightness down",
    10: "Switch off the lights",
    11: "Switch on the fan",
    12: "Speed up the fan",
    13: "Slow down the fan",
    14: "Set higher temperature",
    15: "Set lower temperature",
    16: "Switch off the fan",
    17: "Hello XMOS"
}

def convert(val):
    constructors = [int, float, str]
    for c in constructors:
        try:
            return c(val)
        except ValueError:
            pass

def process(log, label_track, lut):
    recognition_events = []

    # Get recognition events from log
    with open(log, "r") as fd:
        for line in fd:
            if line.startswith(LINE_START):
                recognition_event = {}

                fields = line[len(LINE_START):].strip().split(",")
                for field in fields:
                    key_value = field.split("=")
                    recognition_event[key_value[0].strip()] = convert(key_value[1])

                recognition_events.append(recognition_event)

    # Process events and make label track
    with open(label_track, "w") as fd:
        for recognition_event in recognition_events:
            if lut == "Sensory":
                event_str = SENSORY_LUT[recognition_event["id"]]
            else:
                event_str = str(recognition_event["id"])

            start_sec = recognition_event["start"] * (PIPELINE_BRICK_LENGTH_MS / PIPELINE_BRICK_LENGTH_SAMPLES) / 1000.0
            end_sec = recognition_event["end"] * (PIPELINE_BRICK_LENGTH_MS / PIPELINE_BRICK_LENGTH_SAMPLES) / 1000.0
            print(f"{start_sec}\t{end_sec}\t{event_str}\t{recognition_event['id']}", file=fd)

if __name__ == '__main__':
    parser = argparse.ArgumentParser('Label Track Maker')
    parser.add_argument('--log_file', help='Log file to parse')
    parser.add_argument('--label_track', help='Label track file')
    parser.add_argument('--lut', choices={"Sensory"}, help='Lookup')
    args = parser.parse_args()

    process(args.log_file, args.label_track, args.lut)