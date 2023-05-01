# Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
# XMOS Public License: Version 1

def test_results(log):
    errors = []
    with open(log, 'r+') as f:
        lines = f.readlines()
        for line in lines:
            fields = line.split(',')
            fields = [s.strip() for s in fields]
            values = []
            for field in fields:
                values.append(field.split('=', 1)[-1])
            # strip quotes
            filename = values[0].replace('"', '')
            recognitions = int(values[1])
            min_recognitions = int(values[2])
            max_recognitions = int(values[3])
            if not min_recognitions <= recognitions:
                errors.append(filename + " failed with " + str(recognitions) + " recognitions.")
            if not recognitions <= max_recognitions:
                errors.append(filename + " failed with " + str(recognitions) + " recognitions.")

    assert not errors, "Test failed:\n{}".format("\n".join(errors))