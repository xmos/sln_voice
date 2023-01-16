# Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
# XMOS Public License: Version 1

from math import isclose
import soundfile as sf

from thdncalculator import THDN_and_freq

TEST_CHAN = 0    # Note, the mic channels are swapped on the output
TEST_FREQ = [2000, 1000]  # Note, the mic channels are swapped on the output
TEST_SAMPLE_RATE = 48000

def test_48k_output(wav_file, wav_duration):
    wave_file = sf.SoundFile(wav_file)
    signal = wave_file.read()
    for ch in range(2):
        THDN, freq = THDN_and_freq(signal[:, ch], TEST_SAMPLE_RATE)
        assert isclose(TEST_FREQ[ch], freq, rel_tol=(1 / (2 * wav_duration)))

        THDN_max = (
            -60.0
        )  # in decibels. Actual THD should be -160 or better but capturing artefacts sometimes lower it to -60
        assert THDN_max > THDN
