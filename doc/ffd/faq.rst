.. _sln_voice_ffd_faq:

#############################
Frequently Asked Questions
#############################

Overview
========

This section provides information on common issues and frequently asked questions.


Building
========

If any issues are encountered with building, be sure to verify that CMake is the version specified in the XCORE-SDK installation.

fatfs_mkimage: not found
^^^^^^^^^^^^^^^^^^^^^^^^

This issue occurs when the XCORE-SDK fatfs_mkimage utility cannot be found.  The most common cause for these issues are an incomplete installation of the XCORE-SDK.

Ensure that the host applications setup has been completed.  Verify that the fatfs_mkimage binary is installed to a location on PATH, or that the default application installation folder is added to PATH.

XCORE-SDK installation information can be found here:

:ref:`sdk-installation`
