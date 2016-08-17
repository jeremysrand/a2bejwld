//
//  game.c
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-07-20.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include "game.h"


// Defines

#undef DEBUG_MOVES

#define MIN_MATCHING 3
#define STAR_MATCH 4
#define SPECIAL_MATCH 5

#define STARTING_GEMS_PER_POINT 3
#define MAX_GEMS_PER_POINT 20

#define GEM_TYPE_AT_SQUARE(square) gGameState.squareStates[square].gemType
#define GEM_STARRED_AT_SQUARE(square) gGameState.squareStates[square].isStarred

#define SAVE_GAME_FILE "a2bejwld.game"


// Typedefs

typedef struct tSquareState {
    tGemType gemType;
    bool isStarred;
} tSquareState;


typedef struct tGameState {
    tSquareState squareStates[NUM_SQUARES];
    tLevel       level;
    uint16_t     numGemsCleared;
    uint16_t     targetGemsToClear;
    uint8_t      gemsPerPoint;
    tScore       score;
    tSquare      hintSquare;
} tGameState;


// Globals

static tGameState gGameState;
static tGameCallbacks *gGameCallbacks = NULL;


// Implementation

static void incrementScore(void)
{
    tScore oldScore;
    
    if (gGameState.numGemsCleared < gGameState.targetGemsToClear) {
        gGameState.numGemsCleared++;
        oldScore = gGameState.score;
        gGameState.score = gGameState.numGemsCleared / gGameState.gemsPerPoint;
        
        if (gGameState.score != oldScore) {
            gGameCallbacks->scoreCallback(gGameState.score);
        }
    }
}


static tGemType randomGem(void)
{
    return (rand() % (GEM_MAX_NORMAL - GEM_MIN_NORMAL + 1)) + GEM_MIN_NORMAL;
}


static void clearSquare(tSquare square)
{
    if (GEM_TYPE_AT_SQUARE(square) != GEM_NONE)
        incrementScore();
    GEM_TYPE_AT_SQUARE(square) = GEM_NONE;
    gGameCallbacks->addClearAtSquare(square);
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
            clearSquare(square);
        }
    }
    gGameCallbacks->playSoundForExplodingGem();
    
#ifdef DEBUG_MOVES
    cgetc();
#endif
}


static uint8_t numMatchingUpDownAtSquare(tSquare square, tGemType gemType, bool update)
{
    tPos x = SQUARE_TO_X(square);
    tPos y;
    tPos startY = SQUARE_TO_Y(square);
    uint8_t result = 1;
    tPos lowerY = 0;
    tPos upperY = BOARD_SIZE - 1;
    
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
            clearSquare(square);
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
    tPos leftX = 0;
    tPos rightX = BOARD_SIZE - 1;
    
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
            clearSquare(square);
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
    gGameCallbacks->dropSquareFromOffscreen(square, gemType, false);
}


void initGameEngine(tGameCallbacks *gameCallbacks)
{
    gGameCallbacks = gameCallbacks;
}


void startNewGame(void)
{
    tSquare square;
    
    memset(&gGameState, 0, sizeof(gGameState));
    
    gGameState.level = 1;
    gGameState.numGemsCleared = 0;
    gGameState.gemsPerPoint = STARTING_GEMS_PER_POINT;
    gGameState.targetGemsToClear = STARTING_GEMS_PER_POINT * SCORE_PER_LEVEL;
    gGameState.score = 0;
    
    gGameCallbacks->beginDropAnim();
    for (square = MIN_SQUARE; square <= MAX_SQUARE; square++) {
        initSquare(square);
    }
    gGameCallbacks->endDropAnim();
}


tGemType gemTypeAtSquare(tSquare square)
{
    return GEM_TYPE_AT_SQUARE(square);
}


bool gemIsStarredAtSquare(tSquare square)
{
    bool result = GEM_STARRED_AT_SQUARE(square);
    tGemType gemType;
    
    if (result) {
        gemType = GEM_TYPE_AT_SQUARE(square);
        if ((gemType < GEM_MIN_NORMAL) ||
            (gemType > GEM_MAX_NORMAL)) {
            result = false;
        }
    }
    
    return result;
}


tLevel getLevel(void)
{
    return gGameState.level;
}


tScore getScore(void)
{
    return gGameState.score;
}


tSquare getHintSquare(void)
{
    return gGameState.hintSquare;
}


static void doSwapSquares(tSquare square, tSquare otherSquare, bool update)
{
    tSquareState tempState;
    
    if (update) {
        gGameCallbacks->swapSquares(square, GEM_TYPE_AT_SQUARE(square), GEM_STARRED_AT_SQUARE(square),
                                    otherSquare, GEM_TYPE_AT_SQUARE(otherSquare), GEM_STARRED_AT_SQUARE(otherSquare));
    }
    
    tempState = gGameState.squareStates[square];
    gGameState.squareStates[square] = gGameState.squareStates[otherSquare];
    gGameState.squareStates[otherSquare] = tempState;
}


bool gameIsOver(void)
{
    tPos x, y;
    tSquare square;
    tSquare otherSquare;
    tGemType gemType;
    tGemType otherGemType;
    
    for (y = 0; y < BOARD_SIZE; y++) {
        for (x = 0; x < BOARD_SIZE; x++) {
            square = XY_TO_SQUARE(x, y);
            gemType = GEM_TYPE_AT_SQUARE(square);
            if (gemType == GEM_SPECIAL) {
                gGameState.hintSquare = square;
                return false;
            }
            
            if (x < BOARD_SIZE - 1) {
                otherSquare = XY_TO_SQUARE(x + 1, y);
                otherGemType = GEM_TYPE_AT_SQUARE(otherSquare);
                if (otherGemType == GEM_SPECIAL) {
                    gGameState.hintSquare = otherSquare;
                    return false;
                }
                
                if (gemType != otherGemType) {
                    doSwapSquares(square, otherSquare, false);
                    
                    if ((numMatchingUpDownAtSquare(otherSquare, gemType, false) > 0) ||
                        (numMatchingRightLeftAtSquare(otherSquare, gemType, false) > 0)) {
                        gGameState.hintSquare = square;
                        doSwapSquares(square, otherSquare, false);
                        return false;
                    }
                    
                    if ((numMatchingUpDownAtSquare(square, otherGemType, false) > 0) ||
                        (numMatchingRightLeftAtSquare(square, otherGemType, false)  > 0)) {
                        gGameState.hintSquare = otherSquare;
                        doSwapSquares(square, otherSquare, false);
                        return false;
                    }
                    
                    doSwapSquares(square, otherSquare, false);
                }
            }
            
            if (y < BOARD_SIZE - 1) {
                otherSquare = XY_TO_SQUARE(x, y + 1);
                otherGemType = GEM_TYPE_AT_SQUARE(otherSquare);
                if (otherGemType == GEM_SPECIAL) {
                    gGameState.hintSquare = otherSquare;
                    return false;
                }
                
                if (gemType != otherGemType) {
                    doSwapSquares(square, otherSquare, false);
                    
                    if ((numMatchingUpDownAtSquare(otherSquare, gemType, false) > 0) ||
                        (numMatchingRightLeftAtSquare(otherSquare, gemType, false) > 0)) {
                        gGameState.hintSquare = square;
                        doSwapSquares(square, otherSquare, false);
                        return false;
                    }
                    
                    if ((numMatchingUpDownAtSquare(square, otherGemType, false) > 0) ||
                        (numMatchingRightLeftAtSquare(square, otherGemType, false) > 0)) {
                        gGameState.hintSquare = otherSquare;
                        doSwapSquares(square, otherSquare, false);
                        return false;
                    }
                    
                    doSwapSquares(square, otherSquare, false);
                }
            }
        }
    }
    
    return true;
}


static bool explodeGems(void)
{
    tSquare square;
    bool result = false;
    
    gGameCallbacks->beginClearGemAnim();
    for (square = MIN_SQUARE; square <= MAX_SQUARE; square++) {
        if (GEM_TYPE_AT_SQUARE(square) != GEM_NONE)
            continue;
        if (!GEM_STARRED_AT_SQUARE(square))
            continue;
        
        explodeStarAtSquare(square);
        result = true;
    }
    gGameCallbacks->endClearGemAnim();
    
    return result;
}


static bool actOnMatchAtSquare(tSquare square, bool specialsOnly)
{
    tGemType gemType = GEM_TYPE_AT_SQUARE(square);
    bool starred = GEM_STARRED_AT_SQUARE(square);
    bool result = false;
    uint8_t matchesUD;
    uint8_t matchesRL;
    
    if (gemType == GEM_NONE)
        return result;
    
    matchesUD = numMatchingUpDownAtSquare(square, gemType, !specialsOnly);
    matchesRL = numMatchingRightLeftAtSquare(square, gemType, !specialsOnly);
    
    if (matchesUD > 0)
        result = true;
    
    if (matchesRL > 0)
        result = true;
    
    if (!starred) {
        if ((matchesUD == SPECIAL_MATCH) ||
            (matchesRL == SPECIAL_MATCH)) {
            if (specialsOnly) {
                numMatchingUpDownAtSquare(square, gemType, true);
                numMatchingRightLeftAtSquare(square, gemType, true);
            }
            GEM_TYPE_AT_SQUARE(square) = GEM_SPECIAL;
            gGameCallbacks->playSoundForSpecialGem();
            gGameCallbacks->undoClearAtSquare(square);
            gGameCallbacks->squareCallback(square);
        } else if ((matchesUD == STAR_MATCH) ||
                   (matchesRL == STAR_MATCH) ||
                   ((matchesUD == MIN_MATCHING) &&
                    (matchesRL == MIN_MATCHING))) {
            if (specialsOnly) {
                numMatchingUpDownAtSquare(square, gemType, true);
                numMatchingRightLeftAtSquare(square, gemType, true);
            }
            GEM_TYPE_AT_SQUARE(square) = gemType;
            GEM_STARRED_AT_SQUARE(square) = true;
            gGameCallbacks->playSoundForStarringGem();
            gGameCallbacks->undoClearAtSquare(square);
            gGameCallbacks->squareCallback(square);
        }
    }
    
#ifdef DEBUG_MOVES
    if (result)
        cgetc();
#endif
    
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
    bool starred;
    
    gGameCallbacks->beginDropAnim();
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
                    starred = GEM_STARRED_AT_SQUARE(square);
                    GEM_STARRED_AT_SQUARE(destSquare) = starred;
                    GEM_TYPE_AT_SQUARE(square) = GEM_NONE;
                    GEM_STARRED_AT_SQUARE(square) = false;
                    
                    gGameCallbacks->dropSquareFromTo(square, destSquare, gemType, starred);
                    
                    destY--;
                    destSquare = XY_TO_SQUARE(x, destY);
                }
            }
        }
        if (destSquare != NUM_SQUARES) {
#ifdef DEBUG_MOVES
            cgetc();
#endif
            for (y = destY; y >= 0; y--) {
                square = XY_TO_SQUARE(x, y);
                gemType = randomGem();
                GEM_TYPE_AT_SQUARE(square) = gemType;
                GEM_STARRED_AT_SQUARE(square) = false;
                gGameCallbacks->dropSquareFromOffscreen(square, gemType, false);
            }
#ifdef DEBUG_MOVES
            cgetc();
#endif
        }
    }
    gGameCallbacks->endDropAnim();
    
    gGameCallbacks->beginClearGemAnim();
    for (x = 0; x < BOARD_SIZE; x++) {
        for (y = 0; y < BOARD_SIZE; y++) {
            square = XY_TO_SQUARE(x, y);
            if (actOnMatchAtSquare(square, true))
                result = true;
        }
    }
    
    for (x = 0; x < BOARD_SIZE; x++) {
        for (y = 0; y < BOARD_SIZE; y++) {
            square = XY_TO_SQUARE(x, y);
            if (actOnMatchAtSquare(square, false))
                result = true;
        }
    }
    gGameCallbacks->endClearGemAnim();
    
    if (result) {
        while (explodeGems())
            ;
    }
    
    return result;
}


static tSquare randomSquare(void)
{
    return (rand() % NUM_SQUARES);
}


static void checkForNextLevel(void)
{
    uint8_t numStarred = 0;
    uint8_t numSpecial = 0;
    tSquare square;
    
    if (gGameState.numGemsCleared < gGameState.targetGemsToClear)
        return;
    
    gGameCallbacks->levelCallback(gGameState.level);
    
    for (square = 0; square < NUM_SQUARES; square++) {
        if (GEM_TYPE_AT_SQUARE(square) == GEM_SPECIAL) {
            numSpecial++;
        } else if (GEM_STARRED_AT_SQUARE(square)) {
            numStarred++;
        }
        GEM_TYPE_AT_SQUARE(square) = GEM_NONE;
        GEM_STARRED_AT_SQUARE(square) = false;
        gGameCallbacks->addClearAtSquare(square);
    }
    
    gGameState.level++;
    gGameState.score = 0;
    gGameState.numGemsCleared = 0;
    if (gGameState.level > (MAX_GEMS_PER_POINT - STARTING_GEMS_PER_POINT)) {
        gGameState.gemsPerPoint = MAX_GEMS_PER_POINT;
    } else {
        gGameState.gemsPerPoint = STARTING_GEMS_PER_POINT - 1 + gGameState.level;
    }
    gGameState.targetGemsToClear = gGameState.gemsPerPoint * SCORE_PER_LEVEL;
    gGameCallbacks->scoreCallback(gGameState.score);
    
    gGameCallbacks->beginDropAnim();
    for (square = MIN_SQUARE; square <= MAX_SQUARE; square++) {
        initSquare(square);
    }
    
    while (numStarred > 0) {
        square = randomSquare();
        if (GEM_STARRED_AT_SQUARE(square))
            continue;
        
        GEM_STARRED_AT_SQUARE(square) = true;
        numStarred--;
        gGameCallbacks->dropSquareFromOffscreen(square, GEM_TYPE_AT_SQUARE(square), true);
    }

    while (numSpecial > 0) {
        square = randomSquare();
        if (GEM_STARRED_AT_SQUARE(square))
            continue;
        if (GEM_TYPE_AT_SQUARE(square) == GEM_SPECIAL)
            continue;
        
        GEM_TYPE_AT_SQUARE(square) = GEM_SPECIAL;
        numSpecial--;
        gGameCallbacks->dropSquareFromOffscreen(square, GEM_SPECIAL, false);
    }
    gGameCallbacks->endDropAnim();
}


static void doSpecialForGemType(tGemType gemType, tSquare square)
{
    gGameCallbacks->beginClearGemAnim();
    clearSquare(square);
    
    for (square = MIN_SQUARE; square <= MAX_SQUARE; square++) {
        if (GEM_TYPE_AT_SQUARE(square) == gemType) {
            clearSquare(square);
        }
    }
    gGameCallbacks->playSoundForExplodingGem();
    gGameCallbacks->endClearGemAnim();
    
    while (explodeGems())
        ;
    
    while (dropGems())
        ;
    
    checkForNextLevel();
}


bool moveSquareInDir(tSquare square, tDirection dir)
{
    tPos x = SQUARE_TO_X(square);
    tPos y = SQUARE_TO_Y(square);
    tSquare otherSquare;
    tGemType gemType = GEM_TYPE_AT_SQUARE(square);
    tGemType otherGemType;
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
    
    // Actually do the fun stuff here...
    doSwapSquares(square, otherSquare, true);
    
    if (gemType == GEM_SPECIAL) {
        doSpecialForGemType(otherGemType, otherSquare);
        return true;
    }
    if (otherGemType == GEM_SPECIAL) {
        doSpecialForGemType(gemType, square);
        return true;
    }
    
    gGameCallbacks->beginClearGemAnim();
    if (actOnMatchAtSquare(square, false))
        goodMove = true;
    if (actOnMatchAtSquare(otherSquare, false))
        goodMove = true;
    gGameCallbacks->endClearGemAnim();
    
    if (!goodMove) {
        doSwapSquares(square, otherSquare, true);
    } else {
        
        while (explodeGems())
            ;
        
        while (dropGems())
            ;
        
        checkForNextLevel();
    }
    
    return goodMove;
}


void saveGame(void)
{
    FILE *saveFile = fopen(SAVE_GAME_FILE, "wb");
    if (saveFile != NULL) {
        bool isValid = true;
        fwrite(&isValid, sizeof(isValid), 1, saveFile);
        fwrite(&gGameState, sizeof(gGameState), 1, saveFile);
        fclose(saveFile);
    }
}


static void deleteGame(void)
{
    // So, I tried using unlink() from unistd.h but it seems it
    // does nothing on an Apple // with cc65.  Instead, I will
    // just open the file for writing and close it again which
    // will leave it empty.  That way, there won't be a saved
    // game in the file.
    FILE *saveFile = fopen(SAVE_GAME_FILE, "wb");
    if (saveFile != NULL) {
        bool isValid = false;
        fwrite(&isValid, sizeof(isValid), 1, saveFile);
        fclose(saveFile);
    }
}


bool loadGame(void)
{
    bool isValid = false;
    bool result = false;
    FILE *saveFile= fopen(SAVE_GAME_FILE, "rb");
    
    if (saveFile == NULL) {
        return false;
    }
    
    if ((fread(&isValid, sizeof(isValid), 1, saveFile) != 1) ||
        (!isValid)) {
        fclose(saveFile);
        return false;
    }
    
    if (fread(&gGameState, sizeof(gGameState), 1, saveFile) != 1) {
        fclose(saveFile);
        deleteGame();
        return false;
    }
    
    fclose(saveFile);
    deleteGame();
    
    return true;
}