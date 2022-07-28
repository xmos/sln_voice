.. _sln_voice_ffd_faq:

#############################
Frequently Asked Questions
#############################

Overview
========

This section provides information on common issues and frequently asked questions.


Building
========

If any issues are encountered with building, be sure to verify that CMake is the version specified in the XCORE SDK installation.

fatfs_mkimage: not found
^^^^^^^^^^^^^^^^^^^^^^^^

This issue occurs when the XCORE SDK `fatfs_mkimage` utility cannot be found.  The most common cause for these issues are an incomplete installation of the XCORE SDK.

Ensure that the host applications setup has been completed.  Verify that the `fatfs_mkimage`` binary is installed to a location on PATH, or that the default application installation folder is added to PATH.  See the `XCORE SDK documentation <https://www.xmos.ai/documentation/XM-014660-PC-LATEST/html/>`__ for more information on installing.

xcc2clang.exe: error: no such file or directory
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Those strange characters at the beginning of the path are known as a byte-order mark (BOM). CMake adds them to the beginning of the response files it generates during the configure step. Why does it add them? Because the MSVC compiler toolchain requires them. However, some compiler toolchains, like `gcc` and `xcc`, do not ignore the BOM. Why did CMake think the compiler toolchain was MSVC and not the XTC toolchain? Because of a bug in which certain versions of CMake and certain versions of Visual Studio do not play nice together. The good news is that this appears to have been addressed in CMake version 3.22.3. Update to CMake version 3.22.2 or newer.