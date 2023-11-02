# Copyright 2022-2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
# XMOS Public License: Version 1

def test_results(log):
    errors = []
    with open(log, 'r+') as f:
        lines = f.readlines()
        for line in lines:
            fields = line.split(',')
            fields = [s.strip() for s in fields]
            # expected format:
            # ['filename="wav_name.wav"', 'keyword=alexa', 'detected=25', 'min=20', 'max=25']
            values = []
            for field in fields:
                values.append(field.split('=', 1)[-1])
            # strip quotes
            filename = values[0].replace('"', '')
            detections = int(values[2])
            min_detect = int(values[3])
            max_detect = int(values[4])
            if not min_detect <= detections:
                errors.append(filename + " failed with " + str(detections) + " detections.")
            if not detections <= max_detect:
                errors.append(filename + " failed with " + str(detections) + " detections.")

    assert not errors, "Test failed:\n{}".format("\n".join(errors))