#include <platform.h>
#include <xscope.h>

extern "C"{
  void rx(chanend, chanend);
  void equalise(chanend, chanend);
  void tx(chanend);
}

int main (void){
  chan xscope_chan;
  chan read_chan;
  chan write_chan;
  par {
    xscope_host_data(xscope_chan);
    on tile[0]: rx(xscope_chan, read_chan);
    on tile[0]: equalise(read_chan, write_chan);
    on tile[0]: tx(write_chan);
  }
  return 0;
}
