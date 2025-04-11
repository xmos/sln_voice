# The following patterns are to be excluded from the documentation build

# core 
modules/core/modules/legacy_compat
modules/core/modules/otpinfo
modules/core/modules/random
modules/core/modules/trycatch
modules/rtos/modules/FreeRTOS

# io
modules/io/modules/xud
modules/io/doc/substitutions.rst

# others
modules/sample_rate_conversion
modules/lib_qspi_fast_read
modules/xscope_fileio
modules/sw_pll
modules/xua

# venv and other docs
sln_voice_venv
**/.venv
test
xmos_cmake_toolchain
build_*
modules/*/build
modules/*/build_*
modules/*/index*
modules/*/doc/index*
modules/*/modules/*/index*
**/*.md
**/*.ipynb
**CHANGELOG*
**LICENSE*
**README*
