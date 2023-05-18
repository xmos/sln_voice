/* Speech Recognition HMM Grammar Vocabulary Description file
 * Copyright (C) 2003-2022 Sensory, Inc. All Rights Reserved.
 * 
 *
 *            source: /tmp/tmp.0RquHiTx7w/trecs-en_US_12-13-2-0_2dd0bf4a31d42deebee621607cd1c8d075d29a4b.snsr
 *           created: Wed May  3 11:49:34 2023
 *   min lib version: 6.1.0
 *   operating point: 10
 *  production ready: yes
 *       license key: no
 *
 * Created by VoiceHub 2.3.1
 * Project: Simple_FFD_command_set_US_English_command
 */

//extern u32 gs_command_grammarLabel;
#ifndef NETLABEL
#define NETLABEL
//extern u32 dnn_command_netLabel;
#endif

/* The following phrases (Hex format) correspond to the word IDs emitted by the recognizer. */
#define COMMAND_PHRASE_COUNT 18
#define COMMAND_PHRASE_0 "SILENCE"	/* Legacy system phrase */
#define COMMAND_PHRASE_1 "\x73\x77\x69\x74\x63\x68\x5F\x6F\x6E\x5F\x74\x68\x65\x5F\x74\x5F\x76"	/* Phrase: switch_on_the_t_v */
#define COMMAND_PHRASE_2 "\x63\x68\x61\x6E\x6E\x65\x6C\x5F\x75\x70"	/* Phrase: channel_up */
#define COMMAND_PHRASE_3 "\x63\x68\x61\x6E\x6E\x65\x6C\x5F\x64\x6F\x77\x6E"	/* Phrase: channel_down */
#define COMMAND_PHRASE_4 "\x76\x6F\x6C\x75\x6D\x65\x5F\x75\x70"	/* Phrase: volume_up */
#define COMMAND_PHRASE_5 "\x76\x6F\x6C\x75\x6D\x65\x5F\x64\x6F\x77\x6E"	/* Phrase: volume_down */
#define COMMAND_PHRASE_6 "\x73\x77\x69\x74\x63\x68\x5F\x6F\x66\x66\x5F\x74\x68\x65\x5F\x74\x5F\x76"	/* Phrase: switch_off_the_t_v */
#define COMMAND_PHRASE_7 "\x73\x77\x69\x74\x63\x68\x5F\x6F\x6E\x5F\x74\x68\x65\x5F\x6C\x69\x67\x68\x74\x73"	/* Phrase: switch_on_the_lights */
#define COMMAND_PHRASE_8 "\x62\x72\x69\x67\x68\x74\x6E\x65\x73\x73\x5F\x75\x70"	/* Phrase: brightness_up */
#define COMMAND_PHRASE_9 "\x62\x72\x69\x67\x68\x74\x6E\x65\x73\x73\x5F\x64\x6F\x77\x6E"	/* Phrase: brightness_down */
#define COMMAND_PHRASE_10 "\x73\x77\x69\x74\x63\x68\x5F\x6F\x66\x66\x5F\x74\x68\x65\x5F\x6C\x69\x67\x68\x74\x73"	/* Phrase: switch_off_the_lights */
#define COMMAND_PHRASE_11 "\x73\x77\x69\x74\x63\x68\x5F\x6F\x6E\x5F\x74\x68\x65\x5F\x66\x61\x6E"	/* Phrase: switch_on_the_fan */
#define COMMAND_PHRASE_12 "\x73\x70\x65\x65\x64\x5F\x75\x70\x5F\x74\x68\x65\x5F\x66\x61\x6E"	/* Phrase: speed_up_the_fan */
#define COMMAND_PHRASE_13 "\x73\x6C\x6F\x77\x5F\x64\x6F\x77\x6E\x5F\x74\x68\x65\x5F\x66\x61\x6E"	/* Phrase: slow_down_the_fan */
#define COMMAND_PHRASE_14 "\x73\x65\x74\x5F\x68\x69\x67\x68\x65\x72\x5F\x74\x65\x6D\x70\x65\x72\x61\x74\x75\x72\x65"	/* Phrase: set_higher_temperature */
#define COMMAND_PHRASE_15 "\x73\x65\x74\x5F\x6C\x6F\x77\x65\x72\x5F\x74\x65\x6D\x70\x65\x72\x61\x74\x75\x72\x65"	/* Phrase: set_lower_temperature */
#define COMMAND_PHRASE_16 "\x73\x77\x69\x74\x63\x68\x5F\x6F\x66\x66\x5F\x74\x68\x65\x5F\x66\x61\x6E"	/* Phrase: switch_off_the_fan */
#define COMMAND_PHRASE_17 "nota"	/* Legacy system phrase */
