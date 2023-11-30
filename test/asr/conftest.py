# Copyright 2022-2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.
# XMOS Public License: Version 1

def pytest_addoption(parser):
    parser.addoption("--log", action="store", default="log")

def pytest_generate_tests(metafunc):
    option_value = metafunc.config.option.log
    if 'log' in metafunc.fixturenames and option_value is not None:
        metafunc.parametrize("log", [option_value])