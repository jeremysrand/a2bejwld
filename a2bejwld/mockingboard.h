//
//  mockingboard.h
//  mocktest
//
//  Created by Jeremy Rand on 2016-09-10.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//

#ifndef __mocktest__mockingboard__
#define __mocktest__mockingboard__


#include <stdbool.h>
#include <stdint.h>


// Defines

#define MOCK_NUM_CHANNELS 3


#define TONE_PERIOD_C(octave)       (0x7a3 >> (octave - 1))
#define TONE_PERIOD_C_SHARP(octave) (0x735 >> (octave - 1))
#define TONE_PERIOD_D(octave)       (0x6cd >> (octave - 1))
#define TONE_PERIOD_D_SHARP(octave) (0x66c >> (octave - 1))
#define TONE_PERIOD_E(octave)       (0x60f >> (octave - 1))
#define TONE_PERIOD_F(octave)       (0x5b8 >> (octave - 1))
#define TONE_PERIOD_F_SHARP(octave) (0x566 >> (octave - 1))
#define TONE_PERIOD_G(octave)       (0x518 >> (octave - 1))
#define TONE_PERIOD_G_SHARP(octave) (0x4cf >> (octave - 1))
#define TONE_PERIOD_A(octave)       (0x48a >> (octave - 1))
#define TONE_PERIOD_A_SHARP(octave) (0x449 >> (octave - 1))
#define TONE_PERIOD_B(octave)       (0x40b >> (octave - 1))

#define MIN_NOISE_PERIOD 0
#define MAX_NOISE_PERIOD 31

#define TONE_CHANNEL_A    1
#define TONE_CHANNEL_B    2
#define TONE_CHANNEL_C    4
#define NOISE_CHANNEL_A   8
#define NOISE_CHANNLE_B  16
#define NOISE_CHANNEL_C  32

#define ENABLE_CHANNEL(channels) (0x3f ^ (channels))

#define MIN_AMPLITUDE       0
#define MAX_AMPLITUDE      15
#define VARIABLE_AMPLITUDE 16

#define MIN_ENVELOPE_PERIOD 0
#define MAX_ENVELOPE_PERIOD 65535u
#define MILLISEC_TO_ENVELOP_PERIOD(ms) ((ms) * 4)


// Here is a table of the envelope shapes and how they look:
//
//  ENVELOPE_SHAPE_ONE_SHOT_DECAY         \__________...
//
//  ENVELOPE_SHAPE_ONE_SHOT_ATTACK        /__________...
//
//  ENVELOPE_SHAPE_CONT_DECAY             \|\|\|\|\|\...
//
//  ENVELOPE_SHAPE_CONT_DECAY_HOLD        \__________...
//
//  ENVELOPE_SHAPE_CONT_DECAY_ALT         \/\/\/\/\/\...
//                                          _________
//  ENVELOPE_SHAPE_CONT_DECAY_ALT_HOLD    \|         ...
//
//  ENVELOPE_SHAPE_CONT_ATTACK            /|/|/|/|/|/...
//                                         __________
//  ENVELOPE_SHAPE_CONT_ATTACK_HOLD       /          ...
//  ENVELOPE_SHAPE_CONT_ATTACK_ALT        /\/\/\/\/\/...
//
//  ENVELOPE_SHAPE_CONT_ATTACK_ALT_HOLD   /|_________...

#define ENVELOPE_SHAPE_ONE_SHOT_DECAY        0
#define ENVELOPE_SHAPE_ONE_SHOT_ATTACK       4
#define ENVELOPE_SHAPE_CONT_DECAY            8
#define ENVELOPE_SHAPE_CONT_DECAY_HOLD       9
#define ENVELOPE_SHAPE_CONT_DECAY_ALT       10
#define ENVELOPE_SHAPE_CONT_DECAY_ALT_HOLD  11
#define ENVELOPE_SHAPE_CONT_ATTACK          12
#define ENVELOPE_SHAPE_CONT_ATTACK_HOLD     13
#define ENVELOPE_SHAPE_CONT_ATTACK_ALT      14
#define ENVELOPE_SHAPE_CONT_ATTACK_ALT_HOLD 15



// Typedefs

typedef uint8_t tSlot;


typedef enum {
    SPEAKER_NONE = 0,
    SPEAKER_LEFT = (1 << 0),
    SPEAKER_RIGHT = (1 << 1),
    SPEAKER_BOTH = (1 << 0) | (1 << 1)
} tMockingBoardSpeaker;


typedef struct tMockingSoundRegisters {
    uint16_t tonePeriod[MOCK_NUM_CHANNELS];
    uint8_t  noisePeriod;
    uint8_t  enable;
    uint8_t  amplitude[MOCK_NUM_CHANNELS];
    uint16_t envelopePeriod;
    uint8_t  envelopeShape;
    uint8_t  dummy1;
    uint8_t  dummy2;
} tMockingSoundRegisters;


// API

extern void mockingBoardInit(tSlot slot, bool hasSpeechChip);
extern void mockingBoardShutdown(void);

extern void mockingBoardPlaySound(tMockingBoardSpeaker speaker, tMockingSoundRegisters *registers);
extern void mockingBoardStopSound(tMockingBoardSpeaker speaker);

extern bool mockingBoardSpeechIsBusy(void);
extern bool mockingBoardSpeechIsPlaying(void);
extern bool mockingBoardSpeak(uint8_t *data, uint16_t dataLen);


#endif /* defined(__mocktest__mockingboard__) */
