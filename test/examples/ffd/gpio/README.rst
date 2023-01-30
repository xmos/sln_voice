#################
FFD GPIO Test
#################

The FFD GPIO unit test is designed to verify the behavior of void proc_keyword_res(void *args).

This test runs on an XCORE-AI-EXPLORER board using the same hardware setup as the RTOS driver tests in fwk_rtos.

**************
Hardware Setup
**************

The target hardware for these tests is the XCORE-AI-EXPLORER board.

To setup the board for testing, follow the Hardware Setup section in fwk_rtos tests.

xsim --plugin LoopbackPort.dll "-port tile[0] XS1_PORT_1M 1 0 -port tile[1] XS1_PORT_1M 1 0" --plugin LoopbackPort.dll "-port tile[0] XS1_PORT_1P 1 0 -port tile[1] XS1_PORT_1P 1 0" --plugin LoopbackPort.dll "-port tile[0] XS1_PORT_1E 1 0 -port tile[1] XS1_PORT_1E 1 0" --plugin LoopbackPort.dll "-port tile[0] XS1_PORT_1O 1 0 -port tile[1] XS1_PORT_1O 1 0" --plugin LoopbackPort.dll "-port tile[0] XS1_PORT_1N 1 0 -port tile[1] XS1_PORT_1N 1 0" --weak-external-drive --disable-port-warnings --xscope "-offline trace.xmt" test_ffd_gpio.xe 
