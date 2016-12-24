//
//  sound.c
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-12-18.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//

#include "sound.h"
#include "mockingboard.h"

#include <stdint.h>


// Defines

#define CLEAR_GEM_SOUND_NORMAL  0
#define CLEAR_GEM_SOUND_STAR    1
#define CLEAR_GEM_SOUND_SPECIAL 2
#define CLEAR_GEM_SOUND_EXPLODE 3
#define NUM_CLEAR_GEM_SOUNDS    4


// Globals

static uint8_t gSoundClearGem = CLEAR_GEM_SOUND_NORMAL;

static uint8_t gClearGemSoundFreq[NUM_CLEAR_GEM_SOUNDS][8] = {
    { // CLEAR_GEM_SOUND_NORMAL
        30, 25, 20, 30, 30, 30, 30, 0 },
    { // CLEAR_GEM_SOUND_STAR
        10,  9,  8,  7,  6,  5,  4, 0 },
    { // CLEAR_GEM_SOUND_SPECIAL
        4,  6,  8, 10,  8,  6,  4, 0 },
    { // CLEAR_GEM_SOUND_EXPLODE
        50, 60, 50, 60, 50, 60, 50, 0 }
};

static uint8_t gClearGemSoundDuration[NUM_CLEAR_GEM_SOUNDS][8] = {
    { // CLEAR_GEM_SOUND_NORMAL
        10, 15, 20, 10, 10, 10, 10, 0 },
    { // CLEAR_GEM_SOUND_STAR
        30, 31, 32, 33, 34, 35, 36, 0 },
    { // CLEAR_GEM_SOUND_SPECIAL
        36, 34, 32, 30, 32, 34, 36, 0 },
    { // CLEAR_GEM_SOUND_EXPLODE
        8,  8,  8,  8,  8,  8,  8, 0 },
};

static tMockingSoundRegisters gClearGemMockSounds[NUM_CLEAR_GEM_SOUNDS] = {
    // Normal
    {
        { TONE_PERIOD_C(6), TONE_PERIOD_G(6), 0 },                          // Tone period for the three channels
        0,                                                                  // Noise period
        ENABLE_CHANNEL(TONE_CHANNEL_A | TONE_CHANNEL_B),                    // Enable
        { VARIABLE_AMPLITUDE, VARIABLE_AMPLITUDE, MIN_AMPLITUDE },          // Amplitude for the three channels
        MILLISEC_TO_ENVELOP_PERIOD(960),                                    // Envelope period
        ENVELOPE_SHAPE_ONE_SHOT_DECAY,                                      // Envelope shape
        0,                                                                  // Dummy1
        0                                                                   // Dummy2
    },

    // Star
    {
        { TONE_PERIOD_C(5), TONE_PERIOD_G(5), TONE_PERIOD_E(5) },           // Tone period for the three channels
        10,                                                                 // Noise period
        ENABLE_ALL_CHANNELS,                                                // Enable
        { VARIABLE_AMPLITUDE, VARIABLE_AMPLITUDE, MIN_AMPLITUDE },          // Amplitude for the three channels
        MILLISEC_TO_ENVELOP_PERIOD(960),                                    // Envelope period
        ENVELOPE_SHAPE_ONE_SHOT_DECAY,                                      // Envelope shape
        0,                                                                  // Dummy1
        0                                                                   // Dummy2
    },

    // Special
    {
        { 100, 97, 98 },                                                    // Tone period for the three channels
        4,                                                                  // Noise period
        ENABLE_ALL_CHANNELS,                                                // Enable
        { VARIABLE_AMPLITUDE, VARIABLE_AMPLITUDE, VARIABLE_AMPLITUDE },     // Amplitude for the three channels
        MILLISEC_TO_ENVELOP_PERIOD(1280),                                   // Envelope period
        ENVELOPE_SHAPE_ONE_SHOT_DECAY,                                      // Envelope shape
        0,                                                                  // Dummy1
        0                                                                   // Dummy2
    },
    
    // Explosion
    {
        { 0, 0, 0 },                                            // Tone period for the three channels
        16,                                                     // Noise period
        ENABLE_CHANNEL(NOISE_CHANNEL_A),                        // Enable
        { VARIABLE_AMPLITUDE, MIN_AMPLITUDE, MIN_AMPLITUDE },   // Amplitude for the three channels
        MILLISEC_TO_ENVELOP_PERIOD(3000),                       // Envelope period
        ENVELOPE_SHAPE_ONE_SHOT_DECAY,                          // Envelope shape
        0,                                                      // Dummy1
        0                                                       // Dummy2
    }
};


static tMockingSoundRegisters gGemLandSound = {
    { 0, 0, 0 },                                                        // Tone period for the three channels
    30,                                                                 // Noise period
    ENABLE_CHANNEL(NOISE_CHANNEL_A),                                    // Enable
    { VARIABLE_AMPLITUDE, VARIABLE_AMPLITUDE, VARIABLE_AMPLITUDE },     // Amplitude for the three channels
    MILLISEC_TO_ENVELOP_PERIOD(20),                                     // Envelope period
    ENVELOPE_SHAPE_ONE_SHOT_DECAY,                                      // Envelope shape
    0,                                                                  // Dummy1
    0                                                                   // Dummy2
};


static uint8_t gSpeakGood[] = {
    0xE8, 0x7B, 0x88, 0x3A, 0xE9, 0xE8, 0x7B, 0x88,
    0x3A, 0xE6, 0xE8, 0x7B, 0x88, 0x3A, 0x52, 0xE8,
    0x7B, 0x88, 0x3A, 0x52, 0xEA, 0x7F, 0x68, 0x4A,
    0x65, 0xEA, 0x7F, 0x68, 0x4A, 0xC0
};


static uint8_t gSpeakGo[] = {
    0xE8, 0x7B, 0x88, 0x3A, 0xE9, 0xE8, 0x7B, 0x88,
    0x3A, 0xE6, 0xE8, 0x7B, 0x88, 0x3A, 0x11, 0xE8,
    0x7B, 0x88, 0x3A, 0x63
};


static uint8_t gSpeakLevelComplete[] = {
    0xE8, 0x7B, 0x88, 0x3A, 0x60, 0xE8, 0x7B, 0x88,
    0x3A, 0x0A, 0xE8, 0x7B, 0x88, 0x3A, 0x33, 0xE8,
    0x7B, 0x88, 0x3A, 0xEC, 0xE8, 0x7B, 0x88, 0x3A,
    0x4A, 0xE8, 0x7B, 0x88, 0x3A, 0x4A, 0xE8, 0x7B,
    0x88, 0x3A, 0x60, 0xE8, 0x7B, 0x88, 0x3A, 0x29,
    0xE8, 0x7B, 0x88, 0x3A, 0xAD, 0xE8, 0x7B, 0x88,
    0x3A, 0x5A, 0xE8, 0x7B, 0x88, 0x3A, 0x5A, 0xE8,
    0x7B, 0x88, 0x3A, 0x37, 0xE8, 0x7B, 0x88, 0x3A,
    0x27, 0xE8, 0x7B, 0x88, 0x3A, 0x60, 0xE8, 0x7B,
    0x88, 0x3A, 0x01, 0xE8, 0x7B, 0x88, 0x3A, 0x68,
    0xE8, 0x7B, 0x88, 0x3A, 0xC0
};


static uint8_t gSpeakGetReady[] = {
    0xE8, 0x7B, 0x88, 0x3A, 0xE6, 0xE8, 0x7B, 0x88,
    0x3A, 0xC0, 0xE8, 0x7B, 0x88, 0x3A, 0x0A, 0xE8,
    0x7B, 0x88, 0x3A, 0x68, 0xE8, 0x7B, 0x88, 0x3A,
    0xC0, 0xE8, 0x7B, 0x88, 0x3A, 0x1D, 0xE8, 0x7B,
    0x88, 0x3A, 0x4A, 0xE8, 0x7B, 0x88, 0x3A, 0x4A,
    0xE8, 0x7B, 0x88, 0x3A, 0x25, 0xE8, 0x7B, 0x88,
    0x3A, 0x01
};


static uint8_t gSpeakNoMoreMoves[] = {
    0xE8, 0x7B, 0x88, 0x3A, 0x78, 0xE8, 0x7B, 0x88,
    0x3A, 0x11, 0xE8, 0x7B, 0x88, 0x3A, 0x63, 0xE8,
    0x7B, 0x88, 0x3A, 0x37, 0xE8, 0x7B, 0x88, 0x3A,
    0x11, 0xE8, 0x7B, 0x88, 0x3A, 0x5C, 0xE8, 0x7B,
    0x88, 0x3A, 0x37, 0xE8, 0x7B, 0x88, 0x3A, 0x16,
    0xE8, 0x7B, 0x88, 0x3A, 0x33, 0xE8, 0x7B, 0x88,
    0x3A, 0x2F
};


static uint8_t gSpeakExcellent[] = {
    0xE8, 0x7B, 0x88, 0x3A, 0x4A, 0xE8, 0x7B, 0x88,
    0x3A, 0x29, 0xE8, 0x7B, 0x88, 0x3A, 0x30, 0xE8,
    0x7B, 0x88, 0x3A, 0x0A, 0xE8, 0x7B, 0x88, 0x3A,
    0x4A, 0xE8, 0x7B, 0x88, 0x3A, 0x60, 0xE8, 0x7B,
    0x88, 0x3A, 0x60, 0xE8, 0x7B, 0x88, 0x3A, 0x4A,
    0xE8, 0x7B, 0x88, 0x3A, 0x4A, 0xE8, 0x7B, 0x88,
    0x3A, 0x78, 0xEA, 0x7F, 0x68, 0x4A, 0x68, 0xEA,
    0x7F, 0x68, 0x4A, 0xC0
};


static uint8_t gSpeakIncredible[] = {
    0xE8, 0x7B, 0x88, 0x3A, 0x47, 0xE8, 0x7B, 0x88,
    0x3A, 0x78, 0xE8, 0x7B, 0x88, 0x3A, 0x29, 0xE8,
    0x7B, 0x88, 0x3A, 0x1D, 0xE8, 0x7B, 0x88, 0x3A,
    0x0A, 0xE8, 0x7B, 0x88, 0x3A, 0x65, 0xE8, 0x7B,
    0x88, 0x3A, 0x1B, 0xE8, 0x7B, 0x88, 0x3A, 0x64,
    0xE8, 0x7B, 0x88, 0x3A, 0x60
};


static bool gSoundEnabled = false;
static bool gMockingBoardEnabled = false;
static bool gMockingBoardSpeechEnabled = false;

static tMockingBoardSpeaker gMockingBoardSoundSpeaker = SPEAKER_BOTH;


// Implementation


void soundInit(tSlot mockingBoardSlot, bool enableSpeechChip)
{
    gSoundEnabled = true;
    if (mockingBoardSlot > 0) {
        mockingBoardInit(mockingBoardSlot, enableSpeechChip);
        gMockingBoardEnabled = true;
        gMockingBoardSpeechEnabled = enableSpeechChip;
        // When the speech chip is on, sound effects go out the right speaker
        // only and the left speaker is used for speech.  If the speech chip is
        // off, then sound effects go to both speakers.
        if (enableSpeechChip) {
            gMockingBoardSoundSpeaker = SPEAKER_RIGHT;
        } else {
            gMockingBoardSoundSpeaker = SPEAKER_BOTH;
        }
    } else {
        if (gMockingBoardEnabled) {
            mockingBoardShutdown();
        }
        gMockingBoardEnabled = false;
        gMockingBoardSpeechEnabled = false;
    }
}


void soundShutdown(void)
{
    if (gMockingBoardEnabled) {
        mockingBoardShutdown();
        gMockingBoardEnabled = false;
        gMockingBoardSpeechEnabled = false;
    }
    gSoundEnabled = false;
}


static void playSound(int8_t startFreq, int8_t duration)
{
    int8_t freq;
    
    while (duration > 0) {
        asm ("STA %w", 0xc030);
        freq = startFreq;
        while (freq > 0) {
            freq--;
        }
        duration--;
    }
}


void beginClearGemSound(void)
{
    gSoundClearGem = CLEAR_GEM_SOUND_NORMAL;
}


void playSoundForExplodingGem(void)
{
    if (gSoundClearGem < CLEAR_GEM_SOUND_EXPLODE)
        gSoundClearGem = CLEAR_GEM_SOUND_EXPLODE;
}


void playSoundForStarringGem(void)
{
    if (gSoundClearGem < CLEAR_GEM_SOUND_STAR)
        gSoundClearGem = CLEAR_GEM_SOUND_STAR;
}


void playSoundForSpecialGem(void)
{
    if (gSoundClearGem < CLEAR_GEM_SOUND_SPECIAL)
        gSoundClearGem = CLEAR_GEM_SOUND_SPECIAL;
}


void playClearGemSound(uint8_t frame)
{
    static uint8_t *clearGemSoundFreq;
    static uint8_t *clearGemSoundDuration;
    
    if (!gSoundEnabled)
        return;
    
    if (gMockingBoardEnabled) {
        if (frame == 0) {
            mockingBoardPlaySound(gMockingBoardSoundSpeaker, &(gClearGemMockSounds[gSoundClearGem]));
        }
    } else {
        if (frame == 0) {
            clearGemSoundFreq = &(gClearGemSoundFreq[gSoundClearGem][0]);
            clearGemSoundDuration = &(gClearGemSoundDuration[gSoundClearGem][0]);
        }
        
        playSound(*clearGemSoundFreq, *clearGemSoundDuration);
        clearGemSoundFreq++;
        clearGemSoundDuration++;
    }
}


void playLandingSound(uint8_t numLanded)
{
    if (!gSoundEnabled)
        return;
    
    if (gMockingBoardEnabled) {
        if (numLanded == 0) {
            mockingBoardPlaySound(gMockingBoardSoundSpeaker, &gGemLandSound);
        }
    } else {
        playSound(1, 1);
    }
}


void speakGo(void)
{
    if (!gMockingBoardSpeechEnabled)
        return;
    
    if (mockingBoardSpeechIsBusy()) {
        while (mockingBoardSpeechIsPlaying())
            ;
    }
    
    mockingBoardSpeak(gSpeakGo, sizeof(gSpeakGo));
}


void speakLevelComplete(void)
{
    if (!gMockingBoardSpeechEnabled)
        return;
    
    if (mockingBoardSpeechIsBusy()) {
        while (mockingBoardSpeechIsPlaying())
            ;
    }
    
    mockingBoardSpeak(gSpeakLevelComplete, sizeof(gSpeakLevelComplete));
}


void speakGetReady(void)
{
    if (!gMockingBoardSpeechEnabled)
        return;
    
    if (mockingBoardSpeechIsBusy()) {
        while (mockingBoardSpeechIsPlaying())
            ;
    }
    
    mockingBoardSpeak(gSpeakGetReady, sizeof(gSpeakGetReady));
}


void speakNoMoreMoves(void)
{
    if (!gMockingBoardSpeechEnabled)
        return;
    
    if (mockingBoardSpeechIsBusy()) {
        while (mockingBoardSpeechIsPlaying())
            ;
    }
    
    mockingBoardSpeak(gSpeakNoMoreMoves, sizeof(gSpeakNoMoreMoves));
}

bool speakGood(void)
{
    if (!gMockingBoardSpeechEnabled)
        return true;
    
    if (mockingBoardSpeechIsBusy())
        return false;
    
    mockingBoardSpeak(gSpeakGood, sizeof(gSpeakGood));
    return true;
}


bool speakExcellent(void)
{
    if (!gMockingBoardSpeechEnabled)
        return true;
    
    if (mockingBoardSpeechIsBusy())
        return false;
    
    mockingBoardSpeak(gSpeakExcellent, sizeof(gSpeakExcellent));
    return true;
}

bool speakIncredible(void)
{
    if (!gMockingBoardSpeechEnabled)
        return true;
    
    if (mockingBoardSpeechIsBusy())
        return false;
    
    mockingBoardSpeak(gSpeakIncredible, sizeof(gSpeakIncredible));
    return true;
}
