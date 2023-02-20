.. include:: ../../substitutions.rst

**********************
Modifying the Software
**********************

Implementing the ASR API
========================

Begin your ASR port by creating a new folder under ``example/speech_recognition/asr/port``.  Be sure to include ``asr/api/asr.h`` in your port's main source file.  The ``asr.h`` file includes comments detailing the public API methods and parameters.  ASR ports that implement the public API 
defined can easily be added to current and future XCORE-VOICE example designs that support speech recognition.

Pay close attention to the ``ASR_MALLOC``, ``ASR_FREE``, ``ASR_PRINTF``, and ``ASR_READ_EXT`` macros.  ASR libraries must not call ``malloc``
directly to allocate dynamic memory. Instead call the ``ASR_MALLOC`` and ``ASR_FREE`` macros which use the same function signatures as 
``malloc`` and ``free``.  This allows the application to provide alternative implementations of these functions - like ``pvPortMalloc``
and ``vPortFree`` in a FreeRTOS application.  Similarly, applications should call ``ASR_PRINTF`` instead of ``printf`` or xcore's ``debug_printf``.

The ``ASR_READ_EXT`` macro is provided to load data directly from external memory (QSPI flash or LPDDR) into SRAM. This is the recommended 
way to load coefficients or blocks of data from a model.  It is far more efficient to load the data into SRAM and perform any math on the 
data while it is in SRAM.  The The ``ASR_READ_EXT`` macro has the same function signature as ``memcpy``.  The caller is responsible for 
allocating the destination buffer.  

.. note::

  XMOS provides an arithmetic and DSP library which leverages the XS3 Vector Processing Unit (VPU) to accelerate costly operations on vectors of 16- or 32-bit data. Included are functions for block floating-point arithmetic, fast Fourier transforms, discrete cosine transforms, linear filtering and more.  See the XMath Programming Guide for more information.

.. note::

  To minimize SRAM scratch space usage, some ASR ports load coefficients into SRAM in chunks.  This is useful when performing a routine
  such as a vector matrix multiply as this operation can be performed on a portion of the matrix at a time.

.. note::

  You may also need to modify ``BRICK_SIZE_SAMPLES`` in ``app_conf.h`` to match the number of audio samples expected per process for your ASR port.  In other example designs, this is defined by ``appconfINTENT_SAMPLE_BLOCK_LENGTH``.  This is set to 240 in the existing example designs.  

In the current source code, the model data (and optional grammar data) are set in ``src/process_file.c``.  Modify these variables to reflect your data.  The remainder of the API should be familiar to ASR developers.  The API can be extended if necessary.


Flashing Models
===============

To flash your model, modify the ``--data`` argument passed to ``xflash`` command in the :ref:`sln_voice_asr_programming_guide_flash_model` section.

See ``asr/port/simple/simple_asr_model.h`` to see how the model's flash address is defined.

Placing Models in SRAM
======================

Small models (near or under 100kB in size) may be placed in SRAM.  See `asr/port/simple/simple_asr_model.h`` and ``asr/port/simple/simple_asr_model.c`` for more information on placing your model in SRAM.  

*******
ASR API
*******

.. doxygengroup:: asr_api
   :content-only:
