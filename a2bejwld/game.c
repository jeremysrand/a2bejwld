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
#include <conio.h>

#include "game.h"


#define MIN_MATCHING 3
#define STAR_MATCH 4
#define SPECIAL_MATCH 5

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
tSquareRefreshCallback gSquareRefresh = NULL;


static tGemType randomGem(void)
{
    return (rand() % (GEM_MAX_NORMAL - GEM_MIN_NORMAL)) + GEM_MIN_NORMAL;
}


static void explodeStarAtSquare(tSquare square)
{
    tPos x = SQUARE_TO_X(square);
    tPos y = SQUARE_TO_Y(square);
    tPos minX = (x == 0 ? 0 : x - 1);
    tPos maxX = (x == (BOARD_SIZE - 1) ? (BOARD_SIZE - 1) : x + 1);
    tPos minY = (y == 0 ? 0 : y - 1);
    tPos maxY = (y == (BOARD_SIZE - 1) ? (BOARD_SIZE - 1) : y + 1);
    
    GEM_STARRED_AT_SQUARE(square) = false;
    
    for (x = minX; x <= maxX; x++) {
        for (y = minY; y <= maxY; y++) {
            square = XY_TO_SQUARE(x, y);
            GEM_TYPE_AT_SQUARE(square) = GEM_NONE;
            gSquareRefresh(square);
        }
    }
    // DEBUG
    cgetc();
    // DBEUG
}


static uint8_t numMatchingUpDownAtSquare(tSquare square, tGemType gemType, bool update)
{
    tPos x = SQUARE_TO_X(square);
    tPos y;
    tPos startY = SQUARE_TO_Y(square);
    uint8_t result = 1;
    tPos lowerY = (startY < MIN_MATCHING ? 0 : startY - (MIN_MATCHING - 1));
    tPos upperY = (startY > (BOARD_SIZE - MIN_MATCHING) ? (BOARD_SIZE - 1) : startY + (MIN_MATCHING - 1));
    
    if (gemType == GEM_NONE)
        gemType = GEM_TYPE_AT_SQUARE(square);
    
    if (startY > 0) {
        for (y = startY - 1; y >= lowerY; y--) {
            square = XY_TO_SQUARE(x, y);
            if (gemType != GEM_TYPE_AT_SQUARE(square)) {
                lowerY = y + 1;
                break;
            }
            
            result++;
        }
    }
    
    if (startY < BOARD_SIZE - 1) {
        for (y = startY + 1; y <= upperY; y++) {
            square = XY_TO_SQUARE(x, y);
            if (gemType != GEM_TYPE_AT_SQUARE(square)) {
                upperY = y - 1;
                break;
            }
            
            result++;
        }
    }
    
    if (result < MIN_MATCHING) {
        result = 0;
    } else if (update) {
        for (y = lowerY; y <= upperY; y++) {
            square = XY_TO_SQUARE(x, y);
            GEM_TYPE_AT_SQUARE(square) = GEM_NONE;
            gSquareRefresh(square);
        }
    }
    
    return result;
}


static uint8_t numMatchingRightLeftAtSquare(tSquare square, tGemType gemType, bool update)
{
    
    tPos x;
    tPos y = SQUARE_TO_Y(square);
    tPos startX = SQUARE_TO_X(square);
    uint8_t result = 1;
    tPos leftX = (startX < MIN_MATCHING ? 0 : startX - (MIN_MATCHING - 1));
    tPos rightX = (startX > (BOARD_SIZE - MIN_MATCHING) ? (BOARD_SIZE - 1) : startX + (MIN_MATCHING - 1));
    
    if (gemType == GEM_NONE)
        gemType = GEM_TYPE_AT_SQUARE(square);
    
    if (startX > 0) {
        for (x = startX - 1; x >= leftX; x--) {
            square = XY_TO_SQUARE(x, y);
            if (gemType != GEM_TYPE_AT_SQUARE(square)) {
                leftX = x + 1;
                break;
            }
            
            result++;
        }
    }
    
    if (startX < BOARD_SIZE - 1) {
        for (x = startX + 1; x <= rightX; x++) {
            square = XY_TO_SQUARE(x, y);
            if (gemType != GEM_TYPE_AT_SQUARE(square)) {
                rightX = x - 1;
                break;
            }
            
            result++;
        }
    }
    
    if (result < MIN_MATCHING) {
        result = 0;
    } else if (update) {
        for (x = leftX; x <= rightX; x++) {
            square = XY_TO_SQUARE(x, y);
            GEM_TYPE_AT_SQUARE(square) = GEM_NONE;
            gSquareRefresh(square);
        }
    }
    
    return result;
}


static void initSquare(tSquare square)
{
    tGemType gemType;
    
    do {
        gemType = randomGem();
    } while ((numMatchingUpDownAtSquare(square, gemType, false) != 0) ||
             (numMatchingRightLeftAtSquare(square, gemType, false) != 0));
    
    gGameState.squareStates[square].gemType = gemType;
}


void initGame(tSquareRefreshCallback callback)
{
    tSquare square;
    
    gSquareRefresh = callback;
    
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
                if (numMatchingUpDownAtSquare(otherSquare, gemType, false) > 0)
                    return false;
            
                if (numMatchingUpDownAtSquare(square, otherGemType, false) > 0)
                    return false;
            }
            
            otherSquare = XY_TO_SQUARE(x, y + 1);
            otherGemType = GEM_TYPE_AT_SQUARE(otherSquare);
            
            if (gemType != otherGemType) {
                if (numMatchingRightLeftAtSquare(otherSquare, gemType, false) > 0)
                    return false;
                
                if (numMatchingRightLeftAtSquare(square, otherGemType, false) > 0)
                    return false;
            }
        }
    }
    
    return true;
}


static bool explodeGems(void)
{
    tSquare square;
    bool result = false;
    
    for (square = MIN_SQUARE; square <= MAX_SQUARE; square++) {
        if (GEM_TYPE_AT_SQUARE(square) != GEM_NONE)
            continue;
        if (!GEM_STARRED_AT_SQUARE(square))
            continue;
        
        explodeStarAtSquare(square);
        result = true;
    }
    
    return result;
}


static bool actOnMatchAtSquare(tSquare square)
{
    tGemType gemType = GEM_TYPE_AT_SQUARE(square);
    bool starred = GEM_STARRED_AT_SQUARE(square);
    bool result = false;
    uint8_t matchesUD;
    uint8_t matchesRL;
    
    if (gemType == GEM_NONE)
        return result;
    
    matchesUD = numMatchingUpDownAtSquare(square, gemType, true);
    matchesRL = numMatchingRightLeftAtSquare(square, gemType, true);
    
    if (matchesUD > 0)
        result = true;
    
    if (matchesRL > 0)
        result = true;
    
    if (!starred) {
        if ((matchesUD == SPECIAL_MATCH) ||
            (matchesRL == SPECIAL_MATCH)) {
            GEM_TYPE_AT_SQUARE(square) = GEM_SPECIAL;
            gSquareRefresh(square);
        } else if ((matchesUD == STAR_MATCH) ||
                   (matchesRL == STAR_MATCH)) {
            GEM_TYPE_AT_SQUARE(square) = gemType;
            GEM_STARRED_AT_SQUARE(square) = true;
            gSquareRefresh(square);
            
        } else if ((matchesUD == MIN_MATCHING) &&
                   (matchesRL == MIN_MATCHING)) {
            GEM_TYPE_AT_SQUARE(square) = gemType;
            GEM_STARRED_AT_SQUARE(square) = true;
            gSquareRefresh(square);
        }
    }
    
    // DEBUG
    if (result)
        cgetc();
    // DBEUG
    
    return result;
}


static bool dropGems(void)
{
    bool result = false;
    tPos x;
    tPos y;
    tPos destY;
    tSquare square;
    tSquare destSquare;
    tGemType gemType;
    
    for (x = 0; x < BOARD_SIZE; x++) {
        destSquare = NUM_SQUARES;
        for (y = BOARD_SIZE - 1; y >= 0; y--) {
            square = XY_TO_SQUARE(x, y);
            gemType = GEM_TYPE_AT_SQUARE(square);
            
            if (destSquare == NUM_SQUARES) {
                if (gemType == GEM_NONE) {
                    destSquare = square;
                    destY = y;
                }
            } else {
                if (gemType != GEM_NONE) {
                    GEM_TYPE_AT_SQUARE(destSquare) = gemType;
                    GEM_STARRED_AT_SQUARE(destSquare) = GEM_STARRED_AT_SQUARE(square);
                    gSquareRefresh(destSquare);
                    destY--;
                    destSquare = XY_TO_SQUARE(x, destY);
                }
            }
        }
        if (destSquare != NUM_SQUARES) {
            // DEBUG
            if (result)
                cgetc();
            // DBEUG
            for (y = destY; y >= 0; y--) {
                square = XY_TO_SQUARE(x, y);
                GEM_TYPE_AT_SQUARE(square) = randomGem();
                GEM_STARRED_AT_SQUARE(square) = false;
                gSquareRefresh(destSquare);
            }
            // DEBUG
            if (result)
                cgetc();
            // DBEUG
        }
    }
    
    for (x = 0; x < BOARD_SIZE; x++) {
        for (y = 0; y < BOARD_SIZE; y++) {
            square = XY_TO_SQUARE(x, y);
            if (actOnMatchAtSquare(square))
                result = true;
        }
    }
    
    if (result) {
        while (explodeGems())
            ;
    }
    
    return result;
}


void moveSquareInDir(tSquare square, tDirection dir)
{
    tPos x = SQUARE_TO_X(square);
    tPos y = SQUARE_TO_Y(square);
    tSquare otherSquare;
    tGemType gemType = GEM_TYPE_AT_SQUARE(square);
    tGemType otherGemType;
    bool starred = GEM_STARRED_AT_SQUARE(square);
    bool otherStarred;
    bool goodMove = false;
    
    switch (dir) {
        case DIR_UP:
            y--;
            break;
            
        case DIR_DOWN:
            y++;
            break;
            
        case DIR_LEFT:
            x--;
            break;
        
        case DIR_RIGHT:
            x++;
            break;
    }
    
    otherSquare = XY_TO_SQUARE(x, y);
    otherGemType = GEM_TYPE_AT_SQUARE(otherSquare);
    otherStarred = GEM_STARRED_AT_SQUARE(otherSquare);
    
    // Actually do the fun stuff here...
    
    GEM_TYPE_AT_SQUARE(square) = otherGemType;
    GEM_STARRED_AT_SQUARE(square) = otherStarred;
    
    GEM_TYPE_AT_SQUARE(otherSquare) = gemType;
    GEM_STARRED_AT_SQUARE(otherStarred) = starred;
    
    gSquareRefresh(square);
    gSquareRefresh(otherSquare);
    
    if (actOnMatchAtSquare(square))
        goodMove = true;
    if (actOnMatchAtSquare(otherSquare))
        goodMove = true;
    
    if (!goodMove) {
        GEM_TYPE_AT_SQUARE(square) = gemType;
        GEM_STARRED_AT_SQUARE(square) = starred;
        
        GEM_TYPE_AT_SQUARE(otherSquare) = otherGemType;
        GEM_STARRED_AT_SQUARE(otherStarred) = otherStarred;
        
        gSquareRefresh(square);
        gSquareRefresh(otherSquare);
    } else {
        while (explodeGems())
            ;
        while (dropGems())
            ;
    }
}
