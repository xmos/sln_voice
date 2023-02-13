# FFD Low Power Audio Buffer

## Description

The FFD Low Power Audio Buffer unit test is designed to verify the behavior of:

`void low_power_audio_buffer_enqueue(int32_t *frames, size_t num_frames)`

`uint32_t low_power_audio_buffer_dequeue(uint32_t num_packets)`

## Building the Tests

To build the test application, run the following command from the top of the
repository:

``` console
bash tools/ci/build_tests.sh
```

The `build_test.sh` script will copy the test applications and filesystem files
to the `dist` folder.

## Running Tests

This test runs on `xsim`. Run the test with the following command from the top
of the repository:

``` console
bash test/ffd_low_power_audio_buffer/run_tests.sh
```
