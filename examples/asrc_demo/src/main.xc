// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <platform.h>
#include <xs1.h>
#include <xscope.h>
#include <stdlib.h>
#ifdef __XC__
#define chanend_t chanend
#else
#include <xcore/chanend.h>
#endif

extern "C" {
void asrc_task(chanend_t c_file_to_asrc, chanend_t c_asrc_to_file);
void fileio_task(const char *input_file_name, const char *output_file_name, chanend_t c_file_to_asrc, chanend_t c_asrc_to_file);
void i2s_task();
#if TEST_WAV_XSCOPE
    #include "xscope_io_device.h"
#endif
}

#define IN_WAV_FILE_NAME    "input.wav"
#define OUT_WAV_FILE_NAME   "output.wav"

int main (void)
{
  chan xscope_chan;
  chan c_file_to_asrc, c_asrc_to_file;
  par
  {
#if TEST_WAV_XSCOPE
    xscope_host_data(xscope_chan);
#endif
    on tile[0]: {
#if TEST_WAV_XSCOPE
        xscope_io_init(xscope_chan);
#endif
        par{
            {
                fileio_task("input.wav", "output.wav", c_file_to_asrc, c_asrc_to_file);
                _Exit(1);
            }
            asrc_task(c_file_to_asrc, c_asrc_to_file);
            i2s_task();
        }
    }
  }
  return 0;
}
