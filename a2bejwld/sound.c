//
//  sound.c
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-12-18.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//

#include "sound.h"
#include "ui.h"

#include <stdint.h>


// Defines

#define CLEAR_GEM_SOUND_NORMAL  0
#define CLEAR_GEM_SOUND_STAR    1
#define CLEAR_GEM_SOUND_SPECIAL 2
#define CLEAR_GEM_SOUND_EXPLODE 3
#define NUM_CLEAR_GEM_SOUNDS    4


// Globals

static uint8_t gSoundClearGem = 0;

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




// Implementation


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
    gSoundClearGem = (1 << CLEAR_GEM_SOUND_NORMAL);
}


void playSoundForExplodingGem(void)
{
    gSoundClearGem |= (1 << CLEAR_GEM_SOUND_EXPLODE);
}


void playSoundForStarringGem(void)
{
    gSoundClearGem |= (1 << CLEAR_GEM_SOUND_STAR);
}


void playSoundForSpecialGem(void)
{
    gSoundClearGem |= (1 << CLEAR_GEM_SOUND_SPECIAL);
}


void playClearGemSound(uint8_t frame)
{
    static uint8_t *clearGemSoundFreq;
    static uint8_t *clearGemSoundDuration;
    
    if (!soundEnabled())
        return;
    
    if (mockingBoardEnabled()) {
        // Do something here...
    } else {
        if (frame == 0) {
            uint8_t clearGemSound = CLEAR_GEM_SOUND_NORMAL;
            
            if ((gSoundClearGem & (1 << CLEAR_GEM_SOUND_EXPLODE)) != 0) {
                clearGemSound = CLEAR_GEM_SOUND_EXPLODE;
            } else if ((gSoundClearGem & (1 << CLEAR_GEM_SOUND_SPECIAL)) != 0) {
                clearGemSound = CLEAR_GEM_SOUND_SPECIAL;
            } else if ((gSoundClearGem & (1 << CLEAR_GEM_SOUND_STAR)) != 0) {
                clearGemSound = CLEAR_GEM_SOUND_STAR;
            }
            
            clearGemSoundFreq = &(gClearGemSoundFreq[clearGemSound][0]);
            clearGemSoundDuration = &(gClearGemSoundDuration[clearGemSound][0]);
        }
        
        playSound(*clearGemSoundFreq, *clearGemSoundDuration);
        clearGemSoundFreq++;
        clearGemSoundDuration++;
    }
}


void playLandingSound(void)
{
    if (!soundEnabled())
        return;
    
    if (mockingBoardEnabled()) {
        // Do something here...
    } else {
        playSound(1, 1);
    }
}
