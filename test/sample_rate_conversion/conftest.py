# Copyright 2022-2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
# XMOS Public License: Version 1

def pytest_addoption(parser):
    parser.addoption("--wav_file", action="store", default="output.wav")
    parser.addoption("--wav_duration", action="store", type=int, default=30)


def pytest_generate_tests(metafunc):
    option_value = metafunc.config.option.wav_file
    if 'wav_file' in metafunc.fixturenames and option_value is not None:
        metafunc.parametrize("wav_file", [option_value])

    option_value = metafunc.config.option.wav_duration
    if 'wav_duration' in metafunc.fixturenames and option_value is not None:
        metafunc.parametrize("wav_duration", [option_value])        