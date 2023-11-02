#!/usr/bin/env python3
# Copyright 2021-2023 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import re

test_results_filename = "testing/test.rpt"
test_regex = r"^TEST:\s+(\w+)"

def test_results():
    with open(test_results_filename, "r") as f:
        cnt = 0
        while 1:
            line = f.readline()

            if len(line) == 0:
                assert cnt == 1
                break

            p = re.match(test_regex, line)

            if p:
                cnt += 1
                assert p.group(1).find("PASS") != -1
