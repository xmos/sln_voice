# Copyright 2022 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

from cffi import FFI
from shutil import rmtree

def build_ffi():
    # One more ../ than necessary - builds in the 'build' subdirectory in this folder
    APPLICATION_ROOT = "../../../../applications/stlp"

    FLAGS = [
        '-std=c99',
        '-fPIC'
        ]

    # Source file
    SRCS = [f"{APPLICATION_ROOT}/src/usb/adaptive_rate_callback.c"]
    INCLUDES = [f"{APPLICATION_ROOT}/src/usb/",
                f"{APPLICATION_ROOT}/src/"]

    # Units under test
    ffibuilder = FFI()
    ffibuilder.cdef(
        """
        uint32_t determine_USB_audio_rate(uint32_t timestamp,
                                    uint32_t data_length,
                                    uint32_t direction,
                                    bool update,
                                    uint32_t * debug);
        void reset_state();
        """
    )

    ffibuilder.set_source("adaptive_rate_adjust_api", 
    """
        #include <stdbool.h>
        uint32_t determine_USB_audio_rate(uint32_t timestamp,
                                    uint32_t data_length,
                                    uint32_t direction,
                                    bool update,
                                    uint32_t * debug); 
        void reset_state();
    """,
        sources=SRCS,
        include_dirs=INCLUDES,
        libraries=['m'],
        extra_compile_args=FLAGS)

    ffibuilder.compile(tmpdir="build", target="adaptive_rate_adjust_api.*", verbose=True)

def clean_ffi():
    rmtree("./build")


if __name__ == "__main__":
    build_ffi()
