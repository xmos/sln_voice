# Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
# XMOS Public License: Version 1

def pytest_addoption(parser):
    parser.addoption("--file_1", action="store", default="file1")
    parser.addoption("--file_2", action="store", default="file2")

def pytest_generate_tests(metafunc):
    option_value = metafunc.config.option.file_1
    if 'file_1' in metafunc.fixturenames and option_value is not None:
        metafunc.parametrize("file_1", [option_value])
        
    option_value = metafunc.config.option.file_2
    if 'file_2' in metafunc.fixturenames and option_value is not None:
        metafunc.parametrize("file_2", [option_value])