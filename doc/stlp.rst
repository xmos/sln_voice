.. include:: substitutions.rst

.. _sln_voice_STLP:

#########################
Far-field Voice Assistant
#########################

.. include:: <isonum.txt>

.. toctree::
   :maxdepth: 1
   :hidden:

   stlp/host_integration
   stlp/modifying_software
   stlp/audio_pipeline
   stlp/software_description

Overview
========
This is the XMOS far-field voice assistant reference design.

This application can be used out of the box as a voice processor solution, or expanded to run local wakeword engines.

This application features a full duplex acoustic echo cancellation stage, which can be provided reference audio via I2S or USB audio.  An audio output ASR stream is also available via I2S or USB audio.

Try it
===============

Supported Hardware
------------------

This reference application is supported on the `XK-VOICE-L71 <https://www.digikey.co.uk/en/products/detail/xmos/XK-VOICE-L71/15761172>`_ board.

Setting up the Hardware
-----------------------

This reference design requires an XTAG4 and XK-VOICE-L71 board.

xTAG
^^^^
The xTAG is used to program and debug the device

Connect the xTAG to the debug header, as shown below.

.. image:: images/ffd/xtag_installation.jpg
    :width: 800
    :alt: xtag

Connect the micro USB XTAG4 and micro USB XK-VOICE-L71 to the programming host.

.. image:: images/ffd/host_setup.jpg
    :width: 800
    :alt: programming host setup


Building the Firmware
---------------------

Run the following commands in the root folder to build the I2S firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        make application_stlp_int_adec

.. tab:: Windows

    .. code-block:: console

        cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        nmake application_stlp_int_adec


Run the following commands in the root folder to build the USB firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        make application_stlp_ua_adec

.. tab:: Windows

    .. code-block:: console

        cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        nmake application_stlp_ua_adec


Running the Firmware
--------------------

Before the firmware is run, the filesystem must be loaded.

Inside of the build folder root, after building the firmware, run one of:

.. tab:: Linux and Mac

    .. code-block:: console

        make flash_fs_application_stlp_int_adec
        make flash_fs_application_stlp_ua_adec

.. tab:: Windows

    .. code-block:: console

        nmake flash_fs_application_stlp_int_adec
        nmake flash_fs_application_stlp_ua_adec

Once flashed, the application will run.

After the filesystem has been flashed once, the application can be run without flashing.  If changes are made to the filesystem image, the application must be reflashed.

From the build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        make run_application_stlp_int_adec
        make run_application_stlp_ua_adec

.. tab:: Windows

    .. code-block:: console

        nmake run_application_stlp_int_adec
        nmake run_application_stlp_ua_adec


Debugging the Firmware
----------------------

To debug with xgdb, from the build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        make debug_application_int_adec
        make debug_application_ua_adec

.. tab:: Windows

    .. code-block:: console

        nmake debug_application_int_adec
        nmake debug_application_ua_adec


Host Integration
===================

This reference application can be integrated with existing solutions or modified to be a single controller solution.

Out of the Box Integration
--------------------------

Out of the box integration varies cased on configuration.

INT requires I2S connections to the host.  Refer to the schematic, connecting the host reference audio playback to the ADC I2S and the host input audio to the DAC I2S.

UA requires a USB connection to the host.


Single Controller Solution
--------------------------

In a single controller solution, a user can populate the model runner manager task with the application specific code.

This dummy thread receives only the ASR channel output, which has been downshifted to 16 bits.

The user must ensure that the streambuffer is emptied at at least the rate of the audio pipeline, otherwise samples will be lost.

Populate:

.. code-block:: c
    :caption: Model Runner Dummy (model_runner.c)

    void model_runner_manager(void *args)
    {
        StreamBufferHandle_t input_queue = (StreamBufferHandle_t)args;

        int16_t buf[appconfWW_FRAMES_PER_INFERENCE];

        /* Perform any initialization here */

        while (1)
        {
            /* Receive audio frames */
            uint8_t *buf_ptr = (uint8_t*)buf;
            size_t buf_len = appconfWW_FRAMES_PER_INFERENCE * sizeof(int16_t);
            do {
                size_t bytes_rxed = xStreamBufferReceive(input_queue,
                                                         buf_ptr,
                                                         buf_len,
                                                         portMAX_DELAY);
                buf_len -= bytes_rxed;
                buf_ptr += bytes_rxed;
            } while(buf_len > 0);

            /* Perform inference here */
            // rtos_printf("inference\n");
        }
    }



Design Architecture
===================

The application consists of a PDM microphone input, which is fed through the XMOS-VOICE DSP blocks.  The output ASR channel is then output over I2S or USB.

.. figure:: diagrams/stlp_diagram.drawio.png
   :align: center
   :scale: 80 %
   :alt: ffd diagram


Audio Pipeline
==============

Information on the audio pipeline used by this application can be found here:
:ref:`sln_voice_stlp_ap`
