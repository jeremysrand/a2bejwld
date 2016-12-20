//
//  mockingboard.c
//  mocktest
//
//  Created by Jeremy Rand on 2016-09-10.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "mockingboard.h"
#include "mockingboard_speech.h"


// Defines

#define LATCH_COMMAND 0x7
#define WRITE_COMMAND 0x6
#define RESET_COMMAND 0x0
#define THROUGH_PORT_B 0x4

#define MOCKINGBOARD_LATCH(soundChip) writeCommand((soundChip), LATCH_COMMAND)
#define MOCKINGBOARD_WRITE(soundChip) writeCommand((soundChip), WRITE_COMMAND)
#define MOCKINGBOARD_RESET(soundChip) writeCommand((soundChip), RESET_COMMAND)


// Typedefs

typedef enum {
    SOUND_CHIP_1 = 0,
    SOUND_CHIP_2 = 1,
    NUM_SOUND_CHIPS = 2
} tMockingBoardSoundChip;


// Globals

// Addresses for the two 6522's (assuming slot 4 for now)
static uint8_t *gMockPortB[NUM_SOUND_CHIPS]    = { (uint8_t *)0xc000, (uint8_t *)0xc080 };
static uint8_t *gMockPortA[NUM_SOUND_CHIPS]    = { (uint8_t *)0xc001, (uint8_t *)0xc081 };
static uint8_t *gMockDataDirB[NUM_SOUND_CHIPS] = { (uint8_t *)0xc002, (uint8_t *)0xc082 };
static uint8_t *gMockDataDirA[NUM_SOUND_CHIPS] = { (uint8_t *)0xc003, (uint8_t *)0xc083 };

static uint8_t gMockingBoardInitialized = false;
static uint8_t gMockingBoardSpeechInitialized = false;


// Implementation

static uint8_t *mapIOPointer(tSlot slot, uint8_t *ptr)
{
    uint16_t temp1 = (uint16_t)ptr;
    uint16_t temp2 = slot;
    
    temp2 <<= 8;
    temp1 &= 0xf0ff;
    
    temp1 |= temp2;
    ptr = (uint8_t *)temp1;
    return ptr;
}


void mockingBoardInit(tSlot slot, bool hasSpeechChip)
{
    tMockingBoardSoundChip soundChip;
    
    if (sizeof(tMockingSoundRegisters) != 16) {
        printf("The sound registers must be 16 bytes long!\n");
    }
    
    for (soundChip = SOUND_CHIP_1; soundChip < NUM_SOUND_CHIPS; soundChip++) {
        gMockPortB[soundChip] = mapIOPointer(slot, gMockPortB[soundChip]);
        gMockPortA[soundChip] = mapIOPointer(slot, gMockPortA[soundChip]);
        gMockDataDirB[soundChip] = mapIOPointer(slot, gMockDataDirB[soundChip]);
        gMockDataDirA[soundChip] = mapIOPointer(slot, gMockDataDirA[soundChip]);
        
        *(gMockDataDirA[soundChip]) = 0xff;     // Set port A for output
        *(gMockDataDirB[soundChip]) = 0x7;      // Set port B for output
    }
    
    if (hasSpeechChip) {
        if (gMockingBoardSpeechInitialized) {
            mockingBoardSpeechShutdown();
        }
        mockingBoardSpeechInit(slot);
        gMockingBoardSpeechInitialized = true;
    } else if (gMockingBoardSpeechInitialized) {
        mockingBoardSpeechShutdown();
        gMockingBoardSpeechInitialized = false;
    }
    
    gMockingBoardInitialized = true;
}


void mockingBoardShutdown(void)
{
    if (gMockingBoardSpeechInitialized) {
        mockingBoardSpeechShutdown();
        gMockingBoardSpeechInitialized = false;
    }
    
    gMockingBoardInitialized = false;
}


static void writeCommand(tMockingBoardSoundChip soundChip, uint8_t command)
{
    volatile uint8_t *ptr = gMockPortB[soundChip];
    
    *ptr = command;
    *ptr = THROUGH_PORT_B;
}


static void mockingBoardTableAccess(tMockingBoardSoundChip soundChip, tMockingSoundRegisters *registers)
{
    uint8_t *data = (uint8_t *)registers;
    volatile uint8_t *ptr = gMockPortA[soundChip];
    uint8_t index;
    
    if (!gMockingBoardInitialized)
        return;
    
    MOCKINGBOARD_RESET(soundChip);
    for (index = 0; index < 16; index++) {
        *ptr = index;
        MOCKINGBOARD_LATCH(soundChip);
        *ptr = *data;
        MOCKINGBOARD_WRITE(soundChip);
        data++;
    }
}


void mockingBoardPlaySound(tMockingBoardSpeaker speaker, tMockingSoundRegisters *registers)
{
    if ((speaker & SPEAKER_LEFT) != 0) {
        mockingBoardTableAccess(SOUND_CHIP_1, registers);
    }
    
    if ((speaker & SPEAKER_RIGHT) != 0) {
        mockingBoardTableAccess(SOUND_CHIP_2, registers);
    }
}


void mockingBoardStopSound(tMockingBoardSpeaker speaker)
{
    if ((speaker & SPEAKER_LEFT) != 0) {
        MOCKINGBOARD_RESET(SOUND_CHIP_1);
    }
    
    if ((speaker & SPEAKER_RIGHT) != 0) {
        MOCKINGBOARD_RESET(SOUND_CHIP_2);
    }
}


bool mockingBoardSpeechIsBusy(void)
{
    return (mockingBoardSpeechBusy != 0);
}


bool mockingBoardSpeechIsPlaying(void)
{
    return (mockingBoardSpeechPlaying != 0);
}


bool mockingBoardSpeak(uint8_t *data, uint16_t dataLen)
{
    if (!gMockingBoardSpeechInitialized)
        return false;
    
    if (mockingBoardSpeechIsBusy())
        return false;
    
    mockingBoardSpeechData = data;
    mockingBoardSpeechLen = dataLen + 1;
    mockingBoardSpeakPriv();
    
    return true;
}
