# FFD Low Power Audio Buffer

## Description

The FFD Low Power Audio Buffer unit test is designed to verify the behavior of:

`void low_power_audio_buffer_enqueue(asr_sample_t *frames, size_t num_frames)`

`uint32_t low_power_audio_buffer_dequeue(uint32_t num_packets)`

## Running Tests

This test runs on `xsim`. Run the test with the following command from the top
of the repository:

``` console
bash test/ffd_low_power_audio_buffer/run_tests.sh
```

The output file can be verified via a pytest:

``` console
pytest
```
