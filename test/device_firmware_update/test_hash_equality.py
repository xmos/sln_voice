# Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
# XMOS Public License: Version 1

import hashlib as hl

BUF_SIZE = 65536

def test_hashfile(file_1, file_2):
    digests = []
    for bin_file in [file_1, file_2]:
        a = hl.sha1()
        with open(bin_file, 'rb') as f:
            while True:
                data = f.read(BUF_SIZE)
                if not data:
                    break
                a.update(data)
        digests.append(a.hexdigest())

    assert (digests[0] == digests[1])