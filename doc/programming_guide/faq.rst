.. _sln_voice_faq:

#############################
Frequently Asked Questions
#############################

************************
fatfs_mkimage: not found
************************

This issue occurs when the `fatfs_mkimage` host utility cannot be found.  The most common cause for these issues are an incomplete installation of XCORE-VOICE.

Ensure that the host applications build and install has been completed.  Verify that the `fatfs_mkimage`` binary is installed to a location on PATH, or that the default application installation folder is added to PATH.

******************
FFD Crash At Start
******************

One potential issue with the FFD application is a crash when trying to run:

.. code-block:: console

    Wanson init
    xrun: Program received signal ET_ECALL, Application exception.
        [Switching to tile[0] core[3]]
        0x0008d308 in __xcore_ecallf ()

This generally occurs when the model was not properly loaded into flash.  To flash the model and filesystem, see :ref:`sln_voice_ffd_deloying_linux_mac`
or :ref:`sln_voice_ffd_deloying_native_windows` based on host platform.


**********************
FFD pdm_rx_isr() Crash
**********************

One potential issue with the low power FFD application is a crash after adding new code:

.. code-block:: console

    xrun: Program received signal ET_ECALL, Application exception.
        [Switching to tile[1] core[1]]
        0x0008a182 in pdm_rx_isr ()

This generally occurs when there is not enough processing time available on tile 1, or when interrupts were disabled for too long, causing the mic array driver to fail to meet timing.  To resolve reduce the processing time, minimize context switching and other actions that require kernel locks, and/or increase the tile 1 core clock frequency.

*******************
Debugging low-power
*******************

The clock dividers are set high to minimize core power consumption.  This can make debugging a challenge or impossible.  Even adding a simple `printf` can case critical timing to be missed.  In order to debug with the low-power features enabled, temporarily modify the clock dividers in `app_conf.h`.

.. code-block:: c

    #define appconfLOW_POWER_SWITCH_CLK_DIV         1   // Resulting clock freq 600MHz.
    #define appconfLOW_POWER_OTHER_TILE_CLK_DIV     1   // Resulting clock freq 600MHz.
    #define appconfLOW_POWER_CONTROL_TILE_CLK_DIV   1   // Resulting clock freq 600MHz.

***********************************************
xcc2clang.exe: error: no such file or directory
***********************************************

Those strange characters at the beginning of the path are known as a byte-order mark (BOM). CMake adds them to the beginning of the response files it generates during the configure step. Why does it add them? Because the MSVC compiler toolchain requires them. However, some compiler toolchains, like `gcc` and `xcc`, do not ignore the BOM. Why did CMake think the compiler toolchain was MSVC and not the XTC toolchain? Because of a bug in which certain versions of CMake and certain versions of Visual Studio do not play nice together. The good news is that this appears to have been addressed in CMake version 3.22.3. Update to CMake version 3.22.2 or newer.