#################
Graphic Equaliser
#################

This example application implements the `Graphic Equaliser <https://www.dsprelated.com/showcode/169.php>` model.  
The application will take a wav file as input, perform a signal processing and output wav file using `xscope` channel.

*********************
Building the firmware
*********************

Run the following commands in the xcore_sdk root folder to build the firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ make example_graphic_equaliser

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ nmake example_graphic_equaliser

********************
Running the firmware
********************

Running with hardware

.. tab:: Linux, Mac and Windows

    .. code-block:: console

        $ xrun --xscope-port localhost:10234 example_graphic_equaliser.xe
        $ python host_app.py path/to/input_wav path/to/output_wav

After runnig an xcore app with `xrun`, xcore will wait until a data is sent from a host application. `host_app.py` can be used as the host application for this example. It will send the audio data to the xcore using the `xscope` channel.
The `host_app.py` script requires Python. Ensure you have installed Python 3 and `numpy` python library. Only 1 channel wav format is supported in this example.


