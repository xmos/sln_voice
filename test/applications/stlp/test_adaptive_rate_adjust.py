import random
import pytest
from fxpmath import Fxp

from build_adaptive_rate_adjust import build_ffi, clean_ffi

TICKS_PER_SECOND = 100000000
TICKS_PER_MILLISECOND = 100000
EXPECTED_OUT_BYTES_PER_SAMPLE = 128
EXPECTED_IN_BYTES_PER_SAMPLE = 192
EXPECTED_OUT_BYTES_PER_SECOND = EXPECTED_OUT_BYTES_PER_SAMPLE * 1000
EXPECTED_IN_BYTES_PER_SECOND = EXPECTED_IN_BYTES_PER_SAMPLE * 1000
INTMAX_32 = 4294967295
DIR_OUT = 0
DIR_IN = 1

fxp_frac_gen = lambda val, frac : Fxp(val, signed=False, n_word=32, n_frac = frac)
fxp_gen = lambda val : fxp_frac_gen(val, 31)
parts_per_million = lambda val, ppm : (ppm/1000000) * val

NOMINAL_RATE_NUMBER = 1
NOMINAL_RATE = fxp_gen(NOMINAL_RATE_NUMBER)

def convert_uut(timestamp, data_length, direction, update):
    debug = ffi.new("uint32_t[4]")
    result = determine_USB_audio_rate(timestamp, data_length, direction, update, debug)

    dbg_res = fxp_gen(f"0b{debug[0]:032b}")
    dbg_dps = fxp_frac_gen(f"0b{debug[1]:032b}", 19)
    dbg_tdi = debug[2]
    dbg_tt  = debug[3]

    varstring = f"0b{result:032b}"
    fxp = fxp_gen(varstring)
    return fxp


@pytest.fixture(scope="module")
def build_uut():
    # These are declared global so they may be used in the subsequent tests - bit of a hack
    global adaptive_rate_adjust_api
    global adaptive_rate_adjust_lib
    global determine_USB_audio_rate
    global ffi
    global uut
    global reset

    build_ffi()

    # Import the things we just built
    from build import adaptive_rate_adjust_api
    from adaptive_rate_adjust_api import ffi
    import adaptive_rate_adjust_api.lib as adaptive_rate_adjust_lib
    determine_USB_audio_rate = adaptive_rate_adjust_lib.determine_USB_audio_rate
    reset = adaptive_rate_adjust_lib.reset_state

    # The UUT returns values in UQ31 format. Cast this automagically
    uut = convert_uut

    yield

    clean_ffi()

# Test first call on OUT endpoint returns nominal rate no matter the actual input
# and that reset actually resets the state
def test_first_call(build_uut):
    retval = uut(1, 10, DIR_OUT, True)
    reset()
    assert retval == NOMINAL_RATE

    retval = uut(1, 10, DIR_OUT, True)
    reset()
    assert retval == NOMINAL_RATE

# Test call on OUT endpoint does not impact IN endpoint and vice-versa
def test_crosstalk(build_uut):
    retval_out = uut(1, EXPECTED_OUT_BYTES_PER_SECOND, DIR_OUT, True)
    retval_in = uut(1, EXPECTED_IN_BYTES_PER_SECOND, DIR_IN, True)
    reset()

    assert retval_in  == NOMINAL_RATE
    assert retval_out == NOMINAL_RATE

# Test two OUT data transactions at 1s and 2s return the nominal rate
def test_two_call(build_uut):

    _ = uut(TICKS_PER_SECOND, EXPECTED_OUT_BYTES_PER_SECOND, DIR_OUT, True)
    retval = uut(2*TICKS_PER_SECOND, EXPECTED_OUT_BYTES_PER_SECOND, DIR_OUT, True)
    reset()

    assert retval == NOMINAL_RATE

# Test three OUT data transactions at 1s, 2s, and 3s return the nominal rate
def test_three_call(build_uut):
    lobound = NOMINAL_RATE - parts_per_million(NOMINAL_RATE, 1)
    hibound = NOMINAL_RATE + parts_per_million(NOMINAL_RATE, 1)

    _ = uut(TICKS_PER_SECOND, EXPECTED_OUT_BYTES_PER_SECOND, DIR_OUT, True)
    _ = uut(2*TICKS_PER_SECOND, EXPECTED_OUT_BYTES_PER_SECOND, DIR_OUT, True)
    retval = uut(3*TICKS_PER_SECOND, EXPECTED_OUT_BYTES_PER_SECOND, DIR_OUT, True)

    reset()

    assert lobound <= retval
    assert retval <= hibound

# Test 1 second of normal operation (1 transaction per millisecond) returns nominal rate
def test_one_second(build_uut):
    lobound = NOMINAL_RATE - parts_per_million(NOMINAL_RATE, 1)
    hibound = NOMINAL_RATE + parts_per_million(NOMINAL_RATE, 1)

    for millis in range(1, 1001):
        retval = uut(millis*TICKS_PER_MILLISECOND, EXPECTED_OUT_BYTES_PER_SAMPLE, DIR_OUT, True)

    reset()

    assert lobound <= retval
    assert retval <= hibound

# Test that time can loop without jump in data
def test_one_second_with_loop(build_uut):
    lobound = NOMINAL_RATE - parts_per_million(NOMINAL_RATE, 1)
    hibound = NOMINAL_RATE + parts_per_million(NOMINAL_RATE, 1)
    
    offset = INTMAX_32 - (500 * TICKS_PER_MILLISECOND) # Should overflow halfway thru
    for millis in range(1, 1001):
        t = (offset + (millis*TICKS_PER_MILLISECOND)) % INTMAX_32
        retval = uut(t, EXPECTED_OUT_BYTES_PER_SAMPLE, DIR_OUT, True)
    
    reset()

    assert lobound <= retval
    assert retval <= hibound

# Test that there can be >42.95 seconds of operation - causing the internal timer to overflow
def test_forty_three_seconds(build_uut):
    lobound = NOMINAL_RATE - parts_per_million(NOMINAL_RATE, 1)
    hibound = NOMINAL_RATE + parts_per_million(NOMINAL_RATE, 1)

    for millis in range(1, 43001):
        t = (millis*TICKS_PER_MILLISECOND) % INTMAX_32
        retval = uut(t, EXPECTED_OUT_BYTES_PER_SAMPLE, DIR_OUT, True)

    reset()

    assert lobound <= retval
    assert retval <= hibound

# Test that there can be 240 seconds of operation - causing the internal timer to overflow multiple times
def test_two_hundred_forty_seconds(build_uut):
    lobound = NOMINAL_RATE - parts_per_million(NOMINAL_RATE, 1)
    hibound = NOMINAL_RATE + parts_per_million(NOMINAL_RATE, 1)

    for millis in range(1, 240001):
        t = (millis*TICKS_PER_MILLISECOND) % INTMAX_32
        retval = uut(t, EXPECTED_OUT_BYTES_PER_SAMPLE, DIR_OUT, True)

    reset()

    assert lobound <= retval
    assert retval <= hibound

# Test what happens when the internal timer overflows while the average is changing
def test_unstable_average_overflow(build_uut):
    lobound = NOMINAL_RATE - parts_per_million(NOMINAL_RATE, 1)
    hibound = NOMINAL_RATE + parts_per_million(NOMINAL_RATE, 1)

    for millis in range(1, 41001):
        t = (millis*TICKS_PER_MILLISECOND) % INTMAX_32
        retval = uut(t, EXPECTED_OUT_BYTES_PER_SAMPLE, DIR_OUT, True)
        assert lobound <= retval
        assert retval <= hibound

    for millis in range(41001, 50001):
        t = (millis*TICKS_PER_MILLISECOND) % INTMAX_32
        retval = uut(t, EXPECTED_OUT_BYTES_PER_SAMPLE+1, DIR_OUT, True)
        assert lobound <= retval
        # For 7s of nominal and 9s of +1 per sample, should expect
        assert retval <= fxp_gen(1.00439453125)

    reset()

# Test what a minute of an extra byte per second looks like
def test_slightly_fast(build_uut):
    lobound = fxp_gen(1.0000078125) - parts_per_million(fxp_gen(1.0000078125), 1)
    hibound = fxp_gen(1.0000078125) + parts_per_million(fxp_gen(1.0000078125), 1)

    for seconds in range(1, 60):
        # This is 1 second
        for millis in range(1, 1000):
            t = ((millis*TICKS_PER_MILLISECOND) + (seconds*TICKS_PER_SECOND)) % INTMAX_32
            retval = uut(t, EXPECTED_OUT_BYTES_PER_SAMPLE, DIR_OUT, True)
            
        millis += 1
        t = ((millis*TICKS_PER_MILLISECOND) + (seconds*TICKS_PER_SECOND)) % INTMAX_32
        retval = uut(t, EXPECTED_OUT_BYTES_PER_SAMPLE+1, DIR_OUT, True)

    reset()

    assert lobound <= retval
    assert retval <= hibound

# Test the same as previous, but with realistic jitter
def test_slightly_fast_jitter(build_uut):
    lobound = fxp_gen(1.0000078125) - parts_per_million(fxp_gen(1.0000078125), 1)
    hibound = fxp_gen(1.0000078125) + parts_per_million(fxp_gen(1.0000078125), 1)
    jitter = 100

    for seconds in range(1, 60):
        # This is 1 second
        for millis in range(1, 1000):
            t = (random.randint(-jitter, jitter) + (millis*TICKS_PER_MILLISECOND) + (seconds*TICKS_PER_SECOND)) % INTMAX_32
            retval = uut(t, EXPECTED_OUT_BYTES_PER_SAMPLE, DIR_OUT, True)
            
        millis += 1
        t = (random.randint(-jitter, jitter) + (millis*TICKS_PER_MILLISECOND) + (seconds*TICKS_PER_SECOND)) % INTMAX_32
        retval = uut(t, EXPECTED_OUT_BYTES_PER_SAMPLE+1, DIR_OUT, True)

    reset()

    assert lobound <= retval
    assert retval <= hibound

# Test that there can be 300 seconds of operation with realistic jitter
def test_three_hundred_seconds_jitter(build_uut):
    lobound = NOMINAL_RATE - parts_per_million(NOMINAL_RATE, 1)
    hibound = NOMINAL_RATE + parts_per_million(NOMINAL_RATE, 1)
    jitter = 100

    for millis in range(1, 300001):
        t = (random.randint(-jitter, jitter) + (millis*TICKS_PER_MILLISECOND)) % INTMAX_32
        retval = uut(t, EXPECTED_OUT_BYTES_PER_SAMPLE, DIR_OUT, True)

    reset()

    assert lobound <= retval
    assert retval <= hibound

# Test 1ppm detection with jitter - simulates 128 seconds
def test_one_part_per_million_jitter(build_uut):
    lobound = NOMINAL_RATE + parts_per_million(NOMINAL_RATE, 1)
    hibound = NOMINAL_RATE + parts_per_million(NOMINAL_RATE, 2)
    jitter = 100

    for seconds in range(0, 8):
        # This is 16 seconds
        for millis in range(1, 16000):
            t = (random.randint(-jitter, jitter) + (millis*TICKS_PER_MILLISECOND) + (16*seconds*TICKS_PER_SECOND)) % INTMAX_32
            retval = uut(t, EXPECTED_OUT_BYTES_PER_SAMPLE, DIR_OUT, True)
            
        millis += 1
        t = (random.randint(-jitter, jitter) + (millis*TICKS_PER_MILLISECOND) + (16*seconds*TICKS_PER_SECOND)) % INTMAX_32
        retval = uut(t, EXPECTED_OUT_BYTES_PER_SAMPLE+2, DIR_OUT, True)

    assert retval > lobound
    assert retval < hibound

    reset()