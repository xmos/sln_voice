/* Speech Recognition HMM Grammar Vocabulary Description file
 * Copyright (C) 2003-2022 Sensory, Inc. All Rights Reserved.
 * 
 *
 *            source: /tmp/tmp.rIp0tcjsKu/trecs-en_US_12-13-2-0_61f32b5746c2714729fbda433ea035796031c553.snsr
 *           created: Tue May  2 19:15:29 2023
 *   min lib version: 6.1.0
 *   operating point: 10
 *  production ready: yes
 *       license key: no
 *
 * Created by VoiceHub 2.3.1
 * Project: Hello_XMOS_wakeword
 */

//extern u32 gs_wakeword_grammarLabel;
#ifndef NETLABEL
#define NETLABEL
//extern u32 dnn_wakeword_netLabel;
#endif

/* The following phrases (Hex format) correspond to the word IDs emitted by the recognizer. */
#define WAKEWORD_PHRASE_COUNT 3
#define WAKEWORD_PHRASE_0 "SILENCE"	/* Legacy system phrase */
#define WAKEWORD_PHRASE_1 "\x68\x65\x6C\x6C\x6F\x5F\x78\x5F\x6D\x6F\x73\x73"	/* Phrase: hello_x_moss */
#define WAKEWORD_PHRASE_2 "nota"	/* Legacy system phrase */
