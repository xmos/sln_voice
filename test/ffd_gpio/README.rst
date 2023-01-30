#################
FFD GPIO Test
#################

The FFD GPIO unit test is designed to verify the behavior of void proc_keyword_res(void *args).

This test runs on xsim:

xsim --plugin LoopbackPort.dll "-port tile[0] XS1_PORT_1M 1 0 -port tile[1] XS1_PORT_1M 1 0" --plugin LoopbackPort.dll "-port tile[0] XS1_PORT_1P 1 0 -port tile[1] XS1_PORT_1P 1 0" --xscope "-offline trace.xmt" test_ffd_gpio.xe 
