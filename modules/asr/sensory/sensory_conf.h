// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef SENSORY_CONF_H_
#define SENSORY_CONF_H_

#include <sensorytypes.h>
#include "device_memory.h"

#include <sensorytypes.h>
#include <sensorylib.h>

// configuration variables

// configuration variables
#ifndef SENSORY_ASR_ADDITIONAL_ROM_CACHE
#define SENSORY_ASR_ADDITIONAL_ROM_CACHE     (0)  // OPTIONAL larger ROM cache to speed up processing
#endif


#ifndef SENSORY_ASR_KEEP_GOING
#define SENSORY_ASR_KEEP_GOING     (1)
#endif

#ifndef SENSORY_ASR_MAX_TOKENS
#define SENSORY_ASR_MAX_TOKENS     (500)   // > 0 : override MAX_TOKENS
#endif

#ifndef SENSORY_ASR_SDET_TYPE
#define SENSORY_ASR_SDET_TYPE      (SDET_NONE)   // use SDET_LPSD for Low Power Sound Detect
#endif

#endif /* SENSORY_CONF_H_ */
