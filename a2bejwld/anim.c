//
//  anim.c
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-07-22.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//


#include <conio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "anim.h"
#include "dbllores.h"
#include "game.h"
#include "machine.h"
#include "sound.h"
#include "ui.h"


// Defines

#define STAR_CYCLES_VISIBLE 3000
#define STAR_CYCLES_INVISIBLE 1000

#define DROP_ACCELERATION 1

#define VERTICAL_PIXELS 48
#define HORIZONTAL_PIXELS 64

#define VERTICAL_PIXELS_PER_SQUARE (VERTICAL_PIXELS / BOARD_SIZE)
#define HORIZONTAL_PIXELS_PER_SQUARE (HORIZONTAL_PIXELS / BOARD_SIZE)

#define DRAW_GEM_AT_XY(x, y, gemType, starred)           \
    gTempY = y;                                          \
    gTempX = x;                                          \
    gTempGemType = gemType;                              \
    if (starred) {                                       \
        __asm__ volatile("lda %v", gTempY);              \
        __asm__ volatile("ldx %v", gTempX);              \
        __asm__ volatile("ldy %v", gTempGemType);        \
        __asm__ volatile("jsr _drawAndStarGemAtXY");     \
    } else {                                             \
        __asm__ volatile("lda %v", gTempY);              \
        __asm__ volatile("ldx %v", gTempX);              \
        __asm__ volatile("ldy %v", gTempGemType);        \
        __asm__ volatile("jsr _drawGemAtXY");            \
    }


// Typedefs

typedef struct tStarAnimState
{
    uint16_t counter;
    bool starVisible;
} tStarAnimState;


typedef struct tClearGemAnimState
{
    uint8_t squaresToClear[NUM_SQUARES / (sizeof(uint8_t))];
    bool gotOne;
} tClearGemAnimState;


typedef struct tDropGemInfo {
    uint8_t x;
    int8_t y;
    int8_t endY;
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

typedef void __fastcall__ (*tClearGemHandler)(tSquare square);


// Globals

static tStarAnimState gStarAnimState;
static tClearGemAnimState gClearGemAnimState;
static tDropGemAnimState gDropGemAnimState;

static uint8_t gTempX;
static uint8_t gTempY;
static uint8_t gTempGemType;

static tClearGemHandler gClearGemHandler[] = {
    explodeGemFrame1,
    explodeGemFrame2,
    explodeGemFrame3,
    explodeGemFrame4,
    explodeGemFrame5,
    explodeGemFrame6,
    drawBgSquare
};


// Implementation


void drawGemAtSquare(tSquare square)
{
    static uint8_t tempSquare;
    static uint8_t tempGemType;
    
    tempGemType = gemTypeAtSquare(square);
    tempSquare = square;
    
    __asm__ volatile("lda %v", tempSquare);
    __asm__ volatile("ldy %v", tempGemType);
    __asm__ volatile("jsr _drawGem");
}


static void hideStars(void)
{
    tSquare square;
    
    gVblWait();
    
    for (square = 0; square < NUM_SQUARES; square++) {
        if (gemIsStarredAtSquare(square)) {
            drawGemAtSquare(square);
        }
    }
}


static void showStars(void)
{
    tSquare square;
    
    gVblWait();
    
    for (square = 0; square < NUM_SQUARES; square++) {
        if (gemIsStarredAtSquare(square)) {
            starGem(square);
        }
    }
}


void resetStarAnim(void)
{
    if (!gStarAnimState.starVisible) {
        showStars();
    }
    gStarAnimState.starVisible = true;
    gStarAnimState.counter = STAR_CYCLES_VISIBLE;
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
    beginClearGemSound();
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
    uint8_t frame;
    
    if (!gClearGemAnimState.gotOne)
        return;
    
    for (frame = 0; frame < (sizeof(gClearGemHandler) / sizeof(gClearGemHandler[0])); frame++) {
        bit = 1;
        offset = 0;
        
        playClearGemSound(frame);
        
        gVblWait();
        for (square = 0; square < NUM_SQUARES; square++) {
            if ((gClearGemAnimState.squaresToClear[offset] & bit) != 0) {
                (gClearGemHandler[frame])(square);
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
}


#undef DEBUG_SWAP_ANIM
void swapSquares(tSquare square1, tGemType gemType1, bool starred1,
                 tSquare square2, tGemType gemType2, bool starred2)
{
    
    uint8_t x1 = (SQUARE_TO_X(square1) * HORIZONTAL_PIXELS_PER_SQUARE);
    uint8_t x2 = (SQUARE_TO_X(square2) * HORIZONTAL_PIXELS_PER_SQUARE);
    uint8_t y1 = (SQUARE_TO_Y(square1) * VERTICAL_PIXELS_PER_SQUARE);
    uint8_t y2 = (SQUARE_TO_Y(square2) * VERTICAL_PIXELS_PER_SQUARE);
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
        // some time and don't swap.
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
            DRAW_GEM_AT_XY(x1, y1, gemType1, starred1);
            DRAW_GEM_AT_XY(x2, y2, gemType2, starred2);
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
            DRAW_GEM_AT_XY(x1, y1, gemType1, starred1);
            DRAW_GEM_AT_XY(x2, y2, gemType2, starred2);
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
    
    gemInfo->x = (x * HORIZONTAL_PIXELS_PER_SQUARE);
    
    // The y positions are multiplied by 6 to get a number from 0 to 42
    gemInfo->y = (SQUARE_TO_Y(srcSquare) * VERTICAL_PIXELS_PER_SQUARE);
    gemInfo->endY = (y * VERTICAL_PIXELS_PER_SQUARE);
    
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
    gemInfo->x = (x * HORIZONTAL_PIXELS_PER_SQUARE);
    
    // The y positions are multiplied by 6 to get a number from 0 to 42
    // We skip setting the current y position.  For offscreen gems,
    // we determine the starting y position just before we run the
    // animation.
    gemInfo->endY = (y * VERTICAL_PIXELS_PER_SQUARE);
    
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
    tDropGemInfo *neighbourInfo;
    bool done;
    int8_t limit;
    uint8_t numLanded;
    
    if (!gDropGemAnimState.gotOne)
        return;
    
#ifdef DEBUG_DROP_ANIM
    cgetc();
#endif
    
    square = NUM_SQUARES;
    while (square > 0) {
        square--;
        
        gemInfo = &(gDropGemAnimState.gemState[square]);
        if (gemInfo->landed)
            continue;
        
        if (!gemInfo->visible) {
            neighbourInfo = gemInfo->lowerNeighbour;
            if ((neighbourInfo == NULL) ||
                (neighbourInfo->landed) ||
                (neighbourInfo->visible)) {
                // If I am not visible and I have no lower neighbour, then start right at the top
                // of the screen.  Also, if I am not visible, I have a lower neighbout and my
                // lower neighbour is landed or visible, I also start right at the top of the screen.
                gemInfo->y = 1 - VERTICAL_PIXELS_PER_SQUARE;
                gemInfo->y -= (rand() % VERTICAL_PIXELS_PER_SQUARE);
            } else {
                // My lower neighbour is also off the screen.  I start six lines above my
                // lower neighbour.
                gemInfo->y = neighbourInfo->y - VERTICAL_PIXELS_PER_SQUARE;
            }
        }
    }
    
    while (true) {
        done = true;
        numLanded = 0;
        
        for (square = 0; square < NUM_SQUARES; square++) {
            gemInfo = &(gDropGemAnimState.gemState[square]);
            
            if (gemInfo->landed)
                continue;
            
            if (gemInfo->y == gemInfo->endY) {
                gemInfo->landed = true;
                playLandingSound(numLanded);
                numLanded++;
                continue;
            }
            
            done = false;
            
            gemInfo->y += gemInfo->speed;
            if (gemInfo->y > gemInfo->endY) {
                gemInfo->y = gemInfo->endY;
            }
            
            neighbourInfo = gemInfo->lowerNeighbour;
            if ((neighbourInfo != NULL) &&
                (!neighbourInfo->landed) &&
                ((limit = gemInfo->lowerNeighbour->y - VERTICAL_PIXELS_PER_SQUARE) <= gemInfo->y)) {
                gemInfo->y = limit;
                gemInfo->speed = 0;
            }
            
            gemInfo->speed += DROP_ACCELERATION;
            
            if (gemInfo->y > (1 - VERTICAL_PIXELS_PER_SQUARE)) {
                gemInfo->visible = true;
            }
        }
        
        if (done)
            break;
        
        gVblWait();

        // Completely unroll the actual drawing to make it faster.
#define DRAW_SQUARE(square)                                                 \
        if (!gDropGemAnimState.gemState[square].landed) {                   \
            drawBgSquare(square);                                           \
            if (gDropGemAnimState.gemState[square].visible) {               \
                DRAW_GEM_AT_XY(gDropGemAnimState.gemState[square].x,        \
                               gDropGemAnimState.gemState[square].y,        \
                               gDropGemAnimState.gemState[square].gemType,  \
                               gDropGemAnimState.gemState[square].starred); \
            }                                                               \
        }
        
        DRAW_SQUARE(0);
        DRAW_SQUARE(1);
        DRAW_SQUARE(2);
        DRAW_SQUARE(3);
        DRAW_SQUARE(4);
        DRAW_SQUARE(5);
        DRAW_SQUARE(6);
        DRAW_SQUARE(7);
        DRAW_SQUARE(8);
        DRAW_SQUARE(9);
        DRAW_SQUARE(10);
        DRAW_SQUARE(11);
        DRAW_SQUARE(12);
        DRAW_SQUARE(13);
        DRAW_SQUARE(14);
        DRAW_SQUARE(15);
        DRAW_SQUARE(16);
        DRAW_SQUARE(17);
        DRAW_SQUARE(18);
        DRAW_SQUARE(19);
        DRAW_SQUARE(20);
        DRAW_SQUARE(21);
        DRAW_SQUARE(22);
        DRAW_SQUARE(23);
        DRAW_SQUARE(24);
        DRAW_SQUARE(25);
        DRAW_SQUARE(26);
        DRAW_SQUARE(27);
        DRAW_SQUARE(28);
        DRAW_SQUARE(29);
        DRAW_SQUARE(30);
        DRAW_SQUARE(31);
        DRAW_SQUARE(32);
        DRAW_SQUARE(33);
        DRAW_SQUARE(34);
        DRAW_SQUARE(35);
        DRAW_SQUARE(36);
        DRAW_SQUARE(37);
        DRAW_SQUARE(38);
        DRAW_SQUARE(39);
        DRAW_SQUARE(40);
        DRAW_SQUARE(41);
        DRAW_SQUARE(42);
        DRAW_SQUARE(43);
        DRAW_SQUARE(44);
        DRAW_SQUARE(45);
        DRAW_SQUARE(46);
        DRAW_SQUARE(47);
        DRAW_SQUARE(48);
        DRAW_SQUARE(49);
        DRAW_SQUARE(50);
        DRAW_SQUARE(51);
        DRAW_SQUARE(52);
        DRAW_SQUARE(53);
        DRAW_SQUARE(54);
        DRAW_SQUARE(55);
        DRAW_SQUARE(56);
        DRAW_SQUARE(57);
        DRAW_SQUARE(58);
        DRAW_SQUARE(59);
        DRAW_SQUARE(60);
        DRAW_SQUARE(61);
        DRAW_SQUARE(62);
        DRAW_SQUARE(63);
        
#ifdef DEBUG_DROP_ANIM
        cgetc();
#endif
    }
}
