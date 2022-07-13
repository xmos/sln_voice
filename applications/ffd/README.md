Wanson Local Speech Recognition Application
===========================================

This is the XMOS far-field local dictionary (FFD) reference design with
Wanson speech recognition.

This software is an evaluation version only. It includes a mechanism
that limits the maximum number of recognitions to 50. You can reset the
counter to 0 by restarting or rebooting the application.

The Wanson speech recognition library is [Copyright 2022. Shanghai
Wanson Electronic Technology Co.Ltd (&quot;WANSON&quot;)]{.title-ref}
and is subject to the [Wanson Restrictive
License](https://github.com/xmos/sln_voice/tree/develop/applications/ffd/inference/wanson/lib/LICENSE.md).

Speech Recognition
------------------

The application will recognize the following utterances:

  -----------------------------------------------------------------------
  Wakeword Utterances
  -----------------------------------------------------------------------
  Hello XMOS

  Hello Wanson
  -----------------------------------------------------------------------

  -----------------------------------------------------------------------
  Command Utterances
  -----------------------------------------------------------------------
  Switch on the TV

  Channel up

  Channel down

  Volume up

  Volume down

  Switch off the TV

  Switch on the lights

  Brightness up

  Brightness down

  Switch off the lights

  Switch on the fan

  Speed up the fan

  Slow down the fan

  Set higher temperature

  Set lower temperature

  Switch off the fan
  -----------------------------------------------------------------------

Supported Hardware
------------------

This example is supported on the XK\_VOICE\_L71 board.















## Results {.tabset}

### Plots

We show a scatter plot in this section.


### Tables

We show the data in this tab.
























Building the Firmware
---------------------

Run the following commands in the root folder to build the firmware:

::: {.tab}
Linux and Mac

``` {.console}
cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
cd build
make application_ffd
```
:::

::: {.tab}
Windows

``` {.console}
cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
cd build
nmake application_ffd
```
:::

Running the Firmware
--------------------

Before the firmware is run, the swmem must be loaded. This application
currently uses a nonstandard swmem access, so the steps to setup the
flash are dictated below.

Inside of the build folder root, after building the firmware:

``` {.console}
xobjdump --strip application_ffd.xe
xobjdump --split application_ffd.xb
xflash --boot-partition-size 0x100000 --data image_n0c0.swmem --factory application_ffd.xe --target-file platform_def.xn
```

From the build folder run:

::: {.tab}
Linux and Mac

``` {.console}
make run_application_ffd
```
:::

::: {.tab}
Windows

``` {.console}
nmake run_application_ffd
```
:::

Debugging the firmware with xgdb
--------------------------------

From the build folder run:

::: {.tab}
Linux and Mac

``` {.console}
make debug_application_ffd
```
:::

::: {.tab}
Windows

``` {.console}
nmake debug_application_ffd
```
:::
