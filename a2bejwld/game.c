//
//  game.c
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-07-20.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"


#define MIN_MATCHING 3

#define GEM_TYPE_AT_SQUARE(square) gGameState.squareStates[square].gemType
#define GEM_STARRED_AT_SQUARE(square) gGameState.squareStates[square].isStarred


typedef struct tSquareState {
    tGemType gemType;
    bool isStarred;
} tSquareState;


typedef struct tGameState {
    tSquareState squareStates[NUM_SQUARES];
    uint8_t numStarred;
    uint8_t numSpecial;
    uint8_t level;
    uint8_t score;
} tGameState;


tGameState gGameState;


static tGemType randomGem(void)
{
    return (rand() % (GEM_MAX_NORMAL - GEM_MIN_NORMAL)) + GEM_MIN_NORMAL;
}


static uint8_t numMatchingUpDownAtSquare(tSquare square, tGemType gemType)
{
    tPos x = SQUARE_TO_X(square);
    tPos y;
    tPos startY = SQUARE_TO_Y(square);
    uint8_t result = 1;
    tPos lowerY = (startY < MIN_MATCHING ? 0 : startY - (MIN_MATCHING - 1));
    tPos upperY = (startY > (BOARD_SIZE - MIN_MATCHING) ? (BOARD_SIZE - 1) : startY + (MIN_MATCHING - 1));
    bool isStarred = GEM_STARRED_AT_SQUARE(square);
    
    if (gemType == GEM_NONE)
        gemType = GEM_TYPE_AT_SQUARE(square);
    
    if (startY > 0) {
        for (y = startY - 1; y >= lowerY; y--) {
            square = XY_TO_SQUARE(x, y);
            if (gemType != GEM_TYPE_AT_SQUARE(square))
                break;
            
            result++;
            if (!isStarred)
                isStarred = GEM_STARRED_AT_SQUARE(square);
        }
    }
    
    if (startY < BOARD_SIZE - 1) {
        for (y = startY + 1; y <= upperY; y++) {
            square = XY_TO_SQUARE(x, y);
            if (gemType != GEM_TYPE_AT_SQUARE(square))
                break;
            
            result++;
            if (!isStarred)
                isStarred = GEM_STARRED_AT_SQUARE(square);
        }
    }
    
    if (result < MIN_MATCHING) {
        result = 0;
        isStarred = false;
    }
    
    return result;
}


static uint8_t numMatchingRightLeftAtSquare(tSquare square, tGemType gemType)
{
    tPos x;
    tPos y = SQUARE_TO_Y(square);
    tPos startX = SQUARE_TO_X(square);
    uint8_t result = 1;
    tPos leftX = (startX < MIN_MATCHING ? 0 : startX - (MIN_MATCHING - 1));
    tPos rightX = (startX > (BOARD_SIZE - MIN_MATCHING) ? (BOARD_SIZE - 1) : startX + (MIN_MATCHING - 1));
    bool isStarred = GEM_STARRED_AT_SQUARE(square);
    
    if (gemType == GEM_NONE)
        gemType = GEM_TYPE_AT_SQUARE(square);
    
    if (startX > 0) {
        for (x = startX - 1; x >= leftX; x--) {
            square = XY_TO_SQUARE(x, y);
            if (gemType != GEM_TYPE_AT_SQUARE(square))
                break;
            
            result++;
            if (!isStarred)
                isStarred = GEM_STARRED_AT_SQUARE(square);
        }
    }
    
    if (startX < BOARD_SIZE - 1) {
        for (x = startX + 1; x <= rightX; x++) {
            square = XY_TO_SQUARE(x, y);
            if (gemType != GEM_TYPE_AT_SQUARE(square))
                break;
            
            result++;
            if (!isStarred)
                isStarred = GEM_STARRED_AT_SQUARE(square);
        }
    }
    
    if (result < MIN_MATCHING) {
        result = 0;
        isStarred = false;
    }
    
    return result;
}


static void initSquare(tSquare square)
{
    tGemType gemType;
    
    do {
        gemType = randomGem();
    } while ((numMatchingUpDownAtSquare(square, gemType) != 0) ||
             (numMatchingRightLeftAtSquare(square, gemType) != 0));
    
    gGameState.squareStates[square].gemType = gemType;
}


void initGame(void)
{
    tSquare square;
    
    memset(&gGameState, 0, sizeof(gGameState));
    
    gGameState.level = 1;
    
    for (square = MIN_SQUARE; square <= MAX_SQUARE; square++) {
        initSquare(square);
    }
}


tGemType gemTypeAtSquare(tSquare square)
{
    return GEM_TYPE_AT_SQUARE(square);
}


bool gemIsStarredAtSquare(tSquare square)
{
    return GEM_STARRED_AT_SQUARE(square);
}


tLevel getLevel(void)
{
    return gGameState.level;
}


tScore getScore(void)
{
    return gGameState.score;
}


bool gameIsOver(void)
{
    tPos x, y;
    tSquare square;
    tSquare otherSquare;
    tGemType gemType;
    tGemType otherGemType;
    
    if (gGameState.numSpecial > 0)
        return false;
    
    for (x = 0; x < (BOARD_SIZE - 1); x++) {
        for (y = 0; y < (BOARD_SIZE - 1); y++) {
            square = XY_TO_SQUARE(x, y);
            otherSquare = XY_TO_SQUARE(x + 1, y);
            
            gemType = GEM_TYPE_AT_SQUARE(square);
            otherGemType = GEM_TYPE_AT_SQUARE(otherSquare);
            
            if (gemType != otherGemType) {
                if (numMatchingUpDownAtSquare(otherSquare, gemType) > 0)
                    return false;
            
                if (numMatchingUpDownAtSquare(square, otherGemType) > 0)
                    return false;
            }
            
            otherSquare = XY_TO_SQUARE(x, y + 1);
            otherGemType = GEM_TYPE_AT_SQUARE(otherSquare);
            
            if (gemType != otherGemType) {
                if (numMatchingRightLeftAtSquare(otherSquare, gemType) > 0)
                    return false;
                
                if (numMatchingRightLeftAtSquare(square, otherGemType) > 0)
                    return false;
            }
        }
    }
    
    return true;
}
