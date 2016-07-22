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

#define DROP_ACCELERATION 1


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


typedef struct tDropGemInfo {
    uint8_t x;
    uint8_t y;
    uint8_t endY;
    uint8_t speed;
    tGemType gemType;
    bool starred;
    bool visible;
    bool landed;
    struct tDropGemInfo *lowerNeighbour;
} tDropGemInfo;

typedef struct tDropGemAnimState {
    tDropGemInfo gemState[NUM_SQUARES];
    bool gotOne;
} tDropGemAnimState;


typedef void (*tVblWaitFunction)(void);


// Globals

static tVblWaitFunction gVblWait = vblWait;

static tStarAnimState gStarAnimState;
static tClearGemAnimState gClearGemAnimState;
static tDropGemAnimState gDropGemAnimState;


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


void drawGemAtXY(uint8_t x, uint8_t y, tGemType gemType, bool starred)
{
    static uint8_t tempX;
    
    tempX = x;
    switch (gemType) {
        case GEM_GREEN:
            __asm__("ldx %v", tempX);
            drawGreenGemAtXY(y);
            break;
            
        case GEM_RED:
            __asm__("ldx %v", tempX);
            drawRedGemAtXY(y);
            break;
            
        case GEM_PURPLE:
            __asm__("ldx %v", tempX);
            drawPurpleGemAtXY(y);
            break;
            
        case GEM_ORANGE:
            __asm__("ldx %v", tempX);
            drawOrangeGemAtXY(y);
            break;
            
        case GEM_GREY:
            __asm__("ldx %v", tempX);
            drawGreyGemAtXY(y);
            break;
            
        case GEM_YELLOW:
            __asm__("ldx %v", tempX);
            drawYellowGemAtXY(y);
            break;
            
        case GEM_BLUE:
            __asm__("ldx %v", tempX);
            drawBlueGemAtXY(y);
            break;
            
        case GEM_SPECIAL:
            __asm__("ldx %v", tempX);
            drawSpecialGemAtXY(y);
            break;
            
        default:
            break;
    }
    if (starred) {
        __asm__("ldx %v", tempX);
        starGemAtXY(y);
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


#undef DEBUG_SWAP_ANIM
void swapSquares(tSquare square1, tGemType gemType1, bool starred1,
                 tSquare square2, tGemType gemType2, bool starred2)
{
    // The x positions are multiplied by 4 to get a number from 0 to 28
    uint8_t x1 = (SQUARE_TO_X(square1) << 2);
    uint8_t x2 = (SQUARE_TO_X(square2) << 2);
    // The y positions are multiplied by 3 to get a number from 0 to 21
    uint8_t y1 = (SQUARE_TO_Y(square1) * 3);
    uint8_t y2 = (SQUARE_TO_Y(square2) * 3);
    uint8_t temp;
    
    // If x1 is bigger than x2, then swap.  We want x1 to go up and x2
    // to go down.  Easier to make that assumption.  Same for y1 and
    // y2.
    if ((x1 > x2) ||
        (y1 > y2)) {
        temp = x2;
        x2 = x1;
        x1 = temp;
        
        temp = y2;
        y2 = y1;
        y1 = temp;

#if 0
        // We don't need to swap square numbers.  The code from here on
        // doesn't distinguish between the two square numbers.  So, save
        // some time and don't sway.
        //
        // Be careful if this assumption is no longer true.
        temp = square2;
        square2 = square1;
        square1 = temp;
#endif
        
        temp = gemType2;
        gemType2 = gemType1;
        gemType1 = temp;
        
        temp = starred2;
        starred2 = starred1;
        starred1 = temp;
    }
    
    if (x1 < x2) {
        temp = x2;
        while (x1 < temp) {
            x1++;
            x2--;
            gVblWait();
            drawBgSquare(square1);
            drawBgSquare(square2);
            drawGemAtXY(x1, y1, gemType1, starred1);
            drawGemAtXY(x2, y2, gemType2, starred2);
#ifdef DEBUG_SWAP_ANIM
            cgetc();
#endif
        }
    } else {
        temp = y2;
        while (y1 < temp) {
            y1++;
            y2--;
            gVblWait();
            drawBgSquare(square1);
            drawBgSquare(square2);
            drawGemAtXY(x1, y1, gemType1, starred1);
            drawGemAtXY(x2, y2, gemType2, starred2);
#ifdef DEBUG_SWAP_ANIM
            cgetc();
#endif
        }
    }
}


void beginDropAnim(void)
{
    tSquare square;
    
    memset(&gDropGemAnimState, 0, sizeof(gDropGemAnimState));
    for (square = 0; square < NUM_SQUARES; square++) {
        gDropGemAnimState.gemState[square].landed = true;
    }
}


void dropSquareFromTo(tSquare srcSquare, tSquare tgtSquare, tGemType gemType, bool starred)
{
    tPos x = SQUARE_TO_X(tgtSquare);
    tPos y = SQUARE_TO_Y(tgtSquare);
    tDropGemInfo *gemInfo = &(gDropGemAnimState.gemState[tgtSquare]);
    
    gDropGemAnimState.gotOne = true;
    
    // The x positions are multiplied by 4 to get a number from 0 to 28
    gemInfo->x = (x << 2);
    
    // The y positions are multiplied by 3 to get a number from 0 to 21
    // We also add 24.  We have 24 lines invisible above the screen.
    gemInfo->y = (SQUARE_TO_Y(srcSquare) * 3) + 24;
    gemInfo->endY = (y * 3) + 24;
    
    gemInfo->gemType = gemType;
    gemInfo->starred = starred;
    gemInfo->visible = true;
    gemInfo->landed = false;
    
    if (y < BOARD_SIZE - 1) {
        gemInfo->lowerNeighbour = &(gDropGemAnimState.gemState[XY_TO_SQUARE(x, y + 1)]);
    }
}


void dropSquareFromOffscreen(tSquare tgtSquare, tGemType gemType, bool starred)
{
    tPos x = SQUARE_TO_X(tgtSquare);
    tPos y = SQUARE_TO_Y(tgtSquare);
    tDropGemInfo *gemInfo = &(gDropGemAnimState.gemState[tgtSquare]);
    
    gDropGemAnimState.gotOne = true;
    
    // The x positions are multiplied by 4 to get a number from 0 to 28
    gemInfo->x = (x << 2);
    
    // The y positions are multiplied by 3 to get a number from 0 to 21
    // We also add 24.  We have 24 lines invisible above the screen.
    // We skip setting the current y position.  For offscreen gems,
    // we determine the starting y position just before we run the
    // animation.
    gemInfo->endY = (y * 3) + 24;
    
    gemInfo->gemType = gemType;
    gemInfo->starred = starred;
    gemInfo->visible = false;
    gemInfo->landed = false;
    
    if (y < BOARD_SIZE - 1) {
        gemInfo->lowerNeighbour = &(gDropGemAnimState.gemState[XY_TO_SQUARE(x, y + 1)]);
    }
}


#undef DEBUG_DROP_ANIM
void endDropAnim(void)
{
    tSquare square;
    tDropGemInfo *gemInfo;
    bool done;
    
    
    if (!gDropGemAnimState.gotOne)
        return;
    
    square = NUM_SQUARES;
    while (square > 0) {
        square--;
        
        gemInfo = &(gDropGemAnimState.gemState[square]);
        if (gemInfo->landed)
            continue;
        
        if (!gemInfo->visible) {
            if ((gemInfo->lowerNeighbour == NULL) ||
                (gemInfo->lowerNeighbour->visible)) {
                // If I am not visible and I have no lower neighbour, then start right at the top
                // of the screen.  Also, if I am not visible, I have a lower neighbout and my
                // lower neighbour is visible, I also start right at the top of the screen.
                gemInfo->y = 21;
            } else {
                // My lower neighbour is also off the screen.  I start three lines above my
                // lower neighbour.
                gemInfo->y = gemInfo->lowerNeighbour->y - 3;
            }
        }
    }
    
    while (true) {
        done = true;
        
        square = NUM_SQUARES;
        while (square > 0) {
            square--;
            
            gemInfo = &(gDropGemAnimState.gemState[square]);
            
            if (gemInfo->landed)
                continue;
            
            if (gemInfo->y == gemInfo->endY) {
                gemInfo->landed = true;
                continue;
            }
            
            done = false;
            
            if (gemInfo->lowerNeighbour != NULL) {
                if (gemInfo->y == gemInfo->lowerNeighbour->y - 3) {
                    gemInfo->speed = 0;
                    continue;
                }
            }
            
            gemInfo->y += gemInfo->speed;
            if (gemInfo->y > gemInfo->endY) {
                gemInfo->y = gemInfo->endY;
            }
            
            if (gemInfo->y > 21) {
                gemInfo->visible = true;
            }
            
            gemInfo->speed += DROP_ACCELERATION;
        }
        
        if (done)
            break;
        
        gVblWait();
        
        for (square = 0; square < NUM_SQUARES; square++) {
            gemInfo = &(gDropGemAnimState.gemState[square]);
            if (gemInfo->landed)
                continue;
            
            drawBgSquare(square);
            
            if (!gemInfo->visible)
                continue;
            
            drawGemAtXY(gemInfo->x, gemInfo->y - 24, gemInfo->gemType, gemInfo->starred);
        }
#ifdef DEBUG_DROP_ANIM
        cgetc();
#endif
    }
}