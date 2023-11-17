// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_CONF_H
#define APP_CONF_H

#if __XC__
#ifndef THIS_XCORE_TILE
#define ON_TILE(t) (t == t)
#else
#define ON_TILE(t) (THIS_XCORE_TILE == (t))
#endif
#else /* __XC__ */
#define ON_TILE(t) (!defined(THIS_XCORE_TILE) || THIS_XCORE_TILE == (t))
#endif /* __XC__ */

#define AUDIO_PIPELINE_TILE_NO                      1
#define appconfAUDIO_PIPELINE_BUFFER_ENABLED        1
#define appconfAUDIO_PIPELINE_BUFFER_NUM_FRAMES     32
#define appconfAUDIO_PIPELINE_FRAME_ADVANCE         240

#define appconfAUDIO_PIPELINE_LP_BUF_DEQUEUE_FRAMES 2

#endif /* APP_CONF_H */
