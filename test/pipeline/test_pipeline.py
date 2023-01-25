# Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
# XMOS Public License: Version 1

def test_results(log):

    # TODO for each .log in folder
    # read logfile, parse fields
    with open(log, 'r+') as f:
        fields = f.read().split(',')
        fields = [s.strip() for s in fields]
        # expected format:
        # ['filename="wav_name.wav"', 'keyword=alexa', 'detected=25', 'min=20', 'max=25']
        values = []
        for field in fields:
            print(field.split('=', 1)[-1])
            values.append(field.split('=', 1)[-1])
        # strip quotes
        filename = values[0].replace('"', '')
        detections = int(values[2])
        min_detect = int(values[3])
        max_detect = int(values[4])
        print(filename + " test results:")

        assert min_detect <= detections & detections <= max_detect