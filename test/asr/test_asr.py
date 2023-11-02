# Copyright 2022-2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
# XMOS Public License: Version 1

def test_asr_recognition_count(log):
    errors = []
    with open(log, 'r+') as f:
        lines = f.readlines()
        for line in lines[1:]:
            fields = line.split(',')
            filename = fields[0]
            max_allowable_wer = float(fields[1])
            computed_wer = float(fields[2])

            if computed_wer > max_allowable_wer:
                errors.append(filename + " failed with " + str(computed_wer) + " WER")

    assert not errors, "Test failed:\n{}".format("\n".join(errors))
