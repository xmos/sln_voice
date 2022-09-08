// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef XTFLM_CONF_h_
#define XTFLM_CONF_h_

// The currenly used model lib_vnr/python/model/model_output/model_qaware_xcore.tflite uses 4 operators; AddConv2D, Reshape, Logistic and Conv2D_V2_OpCode
// When adding a new model, open the tflite file in netron.app, check the number of operators and change XTFLM_OPERATORS if needed
#define XTFLM_OPERATORS        (4)

#define NUM_OUTPUT_TENSORS     (1)
#define NUM_INPUT_TENSORS      (1)

#define NETWORK_NUM_THREADS    (1)
#define AISRV_GPIO_LENGTH      (4)
#define MAX_DEBUG_LOG_LENGTH   (256)

#endif // XTFLM_CONF_h_
