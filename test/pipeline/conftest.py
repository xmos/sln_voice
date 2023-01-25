# Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
# XMOS Public License: Version 1

def pytest_addoption(parser):
    parser.addoption("--log", action="store", default="log")

def pytest_generate_tests(metafunc):
    option_value = metafunc.config.option.log
    if 'log' in metafunc.fixturenames and option_value is not None:
        metafunc.parametrize("log", [option_value])

# # generate array of log files
# def pytest_addoption(parser):
#     parser.addoption("--wav_list", action="store", default="list")

# def pytest_generate_tests(metafunc):
#     option_value = metafunc.config.option.wav_list
#     if 'wav_list' in metafunc.fixturenames and option_value is not None:
#         metafunc.parametrize("wav_list", [option_value])