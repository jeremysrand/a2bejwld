//
//  anim.c
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-07-22.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//


#include <apple2.h>
#include <conio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "anim.h"
#include "dbllores.h"
#include "game.h"
#include "vbl.h"


// Defines

#define STAR_CYCLES_VISIBLE 3000
#define STAR_CYCLES_INVISIBLE 1000


// Typedefs

typedef struct tStarAnimState
{
    uint16_t counter;
    bool starVisible;
} tStarAnimState;


typedef struct tClearGemAnimState
{
    uint8_t squaresToClear[NUM_SQUARES / 8];
    bool gotOne;
} tClearGemAnimState;


typedef void (*tVblWaitFunction)(void);


// Globals

static tVblWaitFunction gVblWait = vblWait;

static tStarAnimState gStarAnimState;
static tClearGemAnimState gClearGemAnimState;


// Implementation

void animInit(void)
{
    switch (get_ostype()) {
        case APPLE_IIC:
        case APPLE_IIC35:
        case APPLE_IICEXP:
        case APPLE_IICREV:
        case APPLE_IICPLUS:
            gVblWait = vblWait2c;
            break;
            
        case APPLE_IIGS:
        case APPLE_IIGS1:
        case APPLE_IIGS3:
            vblInit2gs();
            break;
    }
}


void drawGemAtSquare(tSquare square)
{
    switch (gemTypeAtSquare(square)) {
        case GEM_GREEN:
            drawGreenGem(square);
            break;
            
        case GEM_RED:
            drawRedGem(square);
            break;
            
        case GEM_PURPLE:
            drawPurpleGem(square);
            break;
            
        case GEM_ORANGE:
            drawOrangeGem(square);
            break;
            
        case GEM_GREY:
            drawGreyGem(square);
            break;
            
        case GEM_YELLOW:
            drawYellowGem(square);
            break;
            
        case GEM_BLUE:
            drawBlueGem(square);
            break;
            
        case GEM_SPECIAL:
            drawSpecialGem(square);
            break;
            
        default:
            break;
    }
}


void beginStarAnim(void)
{
    gStarAnimState.starVisible = true;
    gStarAnimState.counter = STAR_CYCLES_VISIBLE;
}


void hideStars(void)
{
    tSquare square;
    
    gVblWait();
    
    for (square = 0; square < NUM_SQUARES; square++) {
        if (gemIsStarredAtSquare(square)) {
            drawGemAtSquare(square);
        }
    }
}


void showStars(void)
{
    tSquare square;
    
    gVblWait();
    
    for (square = 0; square < NUM_SQUARES; square++) {
        if (gemIsStarredAtSquare(square)) {
            starGem(square);
        }
    }
}


void endStarAnim(void)
{
    if (!gStarAnimState.starVisible) {
        showStars();
    }
}


void doStarAnim(void)
{
    gStarAnimState.counter--;
    if (gStarAnimState == 0) {
        if (gStarAnimState.starVisible) {
            gStarAnimState.starVisible = false;
            gStarAnimState.counter = STAR_CYCLES_INVISIBLE;
            hideStars();
        } else {
            gStarAnimState.starVisible = true;
            gStarAnimState.counter = STAR_CYCLES_VISIBLE;
            showStars();
        }
    }
}


void beginClearGemAnim(void)
{
    memset(&gClearGemAnimState, 0, sizeof(gClearGemAnimState));
}


void addClearAtSquare(tSquare square)
{
    uint8_t bit = (1 << (square & 0x7));
    uint8_t offset = (square >> 3);
    
    gClearGemAnimState.squaresToClear[offset] |= bit;
    gClearGemAnimState.gotOne = true;
}


void undoClearAtSquare(tSquare square)
{
    uint8_t bit = (1 << (square & 0x7));
    uint8_t offset = (square >> 3);
    
    gClearGemAnimState.squaresToClear[offset] &= (~bit);
}


#undef DEBUG_CLEAR_ANIM
void endClearGemAnim(void)
{
    tSquare square;
    uint8_t bit;
    uint8_t offset;
    
    if (!gClearGemAnimState.gotOne)
        return;
    
    bit = 1;
    offset = 0;
    gVblWait();
    for (square = 0; square < NUM_SQUARES; square++) {
        if ((gClearGemAnimState.squaresToClear[offset] & bit) != 0) {
            explodeGemFrame1(square);
        }
        bit <<= 1;
        if (bit == 0) {
            bit = 1;
            offset++;
        }
    }
#ifdef DEBUG_CLEAR_ANIM
    cgetc();
#endif
    
    bit = 1;
    offset = 0;
    gVblWait();
    for (square = 0; square < NUM_SQUARES; square++) {
        if ((gClearGemAnimState.squaresToClear[offset] & bit) != 0) {
            explodeGemFrame2(square);
        }
        bit <<= 1;
        if (bit == 0) {
            bit = 1;
            offset++;
        }
    }
#ifdef DEBUG_CLEAR_ANIM
    cgetc();
#endif
    
    bit = 1;
    offset = 0;
    gVblWait();
    for (square = 0; square < NUM_SQUARES; square++) {
        if ((gClearGemAnimState.squaresToClear[offset] & bit) != 0) {
            explodeGemFrame3(square);
        }
        bit <<= 1;
        if (bit == 0) {
            bit = 1;
            offset++;
        }
    }
#ifdef DEBUG_CLEAR_ANIM
    cgetc();
#endif
    
    bit = 1;
    offset = 0;
    gVblWait();
    for (square = 0; square < NUM_SQUARES; square++) {
        if ((gClearGemAnimState.squaresToClear[offset] & bit) != 0) {
            explodeGemFrame4(square);
        }
        bit <<= 1;
        if (bit == 0) {
            bit = 1;
            offset++;
        }
    }
#ifdef DEBUG_CLEAR_ANIM
    cgetc();
#endif
    
    bit = 1;
    offset = 0;
    gVblWait();
    for (square = 0; square < NUM_SQUARES; square++) {
        if ((gClearGemAnimState.squaresToClear[offset] & bit) != 0) {
            explodeGemFrame5(square);
        }
        bit <<= 1;
        if (bit == 0) {
            bit = 1;
            offset++;
        }
    }
#ifdef DEBUG_CLEAR_ANIM
    cgetc();
#endif
    
    bit = 1;
    offset = 0;
    gVblWait();
    for (square = 0; square < NUM_SQUARES; square++) {
        if ((gClearGemAnimState.squaresToClear[offset] & bit) != 0) {
            explodeGemFrame6(square);
        }
        bit <<= 1;
        if (bit == 0) {
            bit = 1;
            offset++;
        }
    }
#ifdef DEBUG_CLEAR_ANIM
    cgetc();
#endif
    
    bit = 1;
    offset = 0;
    gVblWait();
    for (square = 0; square < NUM_SQUARES; square++) {
        if ((gClearGemAnimState.squaresToClear[offset] & bit) != 0) {
            drawBgSquare(square);
        }
        bit <<= 1;
        if (bit == 0) {
            bit = 1;
            offset++;
        }
    }
#ifdef DEBUG_CLEAR_ANIM
    cgetc();
#endif
}
