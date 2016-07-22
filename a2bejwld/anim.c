//
//  anim.c
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-07-22.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//


#include <apple2.h>
#include <stdbool.h>
#include <stdint.h>

#include "anim.h"
#include "dbllores.h"
#include "game.h"
#include "vbl.h"


#define STAR_CYCLES_VISIBLE 3000
#define STAR_CYCLES_INVISIBLE 1000


typedef struct tStarAnimState
{
    uint16_t counter;
    bool starVisible;
} tStarAnimState;

typedef void (*tVblWaitFunction)(void);

tVblWaitFunction gVblWait = vblWait;

tStarAnimState gStarAnimState;


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
