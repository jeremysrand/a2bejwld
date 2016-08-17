//
//  ui.c
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-07-20.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//


#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

#include "ui.h"
#include "anim.h"
#include "dbllores.h"
#include "game.h"
#include "joystick.h"


// Forward delcarations

static void refreshSquare(tSquare square);
static void refreshScore(tScore score);
static void refreshLevel(tLevel level);

static bool joystickChangedCallback(tJoyState *oldState, tJoyState *newState);
static bool joystickNoChangeCallback(tJoyState *oldState);


// Globals

static tSquare gSelectedSquare = 0;
static uint8_t gScoreBar = 0;

static tGameCallbacks gCallbacks = {
    refreshSquare,
    refreshScore,
    refreshLevel,
    
    beginClearGemAnim,
    addClearAtSquare,
    undoClearAtSquare,
    playSoundForExplodingGem,
    playSoundForStarringGem,
    playSoundForSpecialGem,
    endClearGemAnim,
    
    swapSquares,
    
    beginDropAnim,
    dropSquareFromTo,
    dropSquareFromOffscreen,
    endDropAnim
};


static tJoyCallbacks gJoyCallbacks = {
    joystickChangedCallback,
    joystickNoChangeCallback,
    25,             // Read poll time
    40,             // Initial no change poll time
    10              // Subsequent no change poll time
};

static bool gShouldSave = false;


// Implementation

static void showAndClearDblLoRes(void)
{
    showDblLoRes();
    clearDblLoRes();
}


void printInstructions(void)
{
    
    int seed = 0;
    
    unshowDblLoRes();
    videomode(VIDEOMODE_80x24);
    clrscr();
    printf(
          //      0000000001111111111222222222233333333334444444444555555555566666666667
          //      1234567890123456789012345678901234567890123456789012345678901234567890
           "                                 Apple Jeweled\n"
           "                                by Jeremy Rand\n"
           "\n"
           "     Use I-J-K-M, the arrow keys or the joystick to move your selection.\n"
           "     Hold either apple key or joystick button and move your selection to\n"
           "     swap two jewels and match 3 or more jewels.  When you match three\n"
           "     jewels, they disappear and new jewels will drop from the top.\n"
           "\n"
           "     If you match four jewels or three jewels in two directions, then the\n"
           "     jewel does not disappear.  Match it again and it explodes taking more\n"
           "     jewels with it.  Match five jewels and a special jewel will appear.\n"
           "     Swap it with any other jewel and all jewels of that colour will\n"
           "     disappear.\n"
           "\n"
           "     When the score bar on the right fills, the board reloads and you level\n"
           "     up.  Play ends when no more matches can be made.\n"
           "\n"
           "                    Press Q or escape to quit at any time.\n"
           "                    Press R to start a new game.\n"
           "                    Press S to toggle sound.\n"
           "                    Press H to get a hint.\n"
           "                    Press ? to see this info again.\n"
           "\n"
           "                           Press any key to start");
    
    // The amount of time the user waits to read the in
    while (!kbhit())
        seed++;
    
    cgetc();
    srand(seed);
    
    clrscr();
}


static void refreshSquare(tSquare square)
{
    drawBgSquare(square);
    drawGemAtSquare(square);
    
    if (gemIsStarredAtSquare(square))
        starGem(square);
}


static void drawBoard(void)
{
    tSquare square;
    
    for (square = MIN_SQUARE; square <= MAX_SQUARE; square++) {
        refreshSquare(square);
    }
    
    selectSquare(gSelectedSquare);
    drawScore(gScoreBar);
}


static void quitGame(void)
{
    unshowDblLoRes();
    videomode(VIDEOMODE_40x24);
    clrscr();
    exit(0);
}


static void moveUp(void)
{
    tSquare oldSquare = gSelectedSquare;
    tPos x = SQUARE_TO_X(gSelectedSquare);
    tPos y = SQUARE_TO_Y(gSelectedSquare);
    
    if (y == 0)
        y = BOARD_SIZE - 1;
    else
        y--;
    
    gSelectedSquare = XY_TO_SQUARE(x, y);
    
    refreshSquare(oldSquare);
    selectSquare(gSelectedSquare);
}


static void moveUpLeft(void)
{
    tSquare oldSquare = gSelectedSquare;
    tPos x = SQUARE_TO_X(gSelectedSquare);
    tPos y = SQUARE_TO_Y(gSelectedSquare);
    
    if (y == 0)
        y = BOARD_SIZE - 1;
    else
        y--;
    
    if (x == 0)
        x = BOARD_SIZE - 1;
    else
        x--;
    
    gSelectedSquare = XY_TO_SQUARE(x, y);
    
    refreshSquare(oldSquare);
    selectSquare(gSelectedSquare);
}



static void moveUpRight(void)
{
    tSquare oldSquare = gSelectedSquare;
    tPos x = SQUARE_TO_X(gSelectedSquare);
    tPos y = SQUARE_TO_Y(gSelectedSquare);
    
    if (y == 0)
        y = BOARD_SIZE - 1;
    else
        y--;
    
    if (x == BOARD_SIZE - 1)
        x = 0;
    else
        x++;
    
    gSelectedSquare = XY_TO_SQUARE(x, y);
    
    refreshSquare(oldSquare);
    selectSquare(gSelectedSquare);
}


static void moveDown(void)
{
    tSquare oldSquare = gSelectedSquare;
    tPos x = SQUARE_TO_X(gSelectedSquare);
    tPos y = SQUARE_TO_Y(gSelectedSquare);
    
    if (y == BOARD_SIZE - 1)
        y = 0;
    else
        y++;
    
    gSelectedSquare = XY_TO_SQUARE(x, y);
    
    refreshSquare(oldSquare);
    selectSquare(gSelectedSquare);
}


static void moveDownLeft(void)
{
    tSquare oldSquare = gSelectedSquare;
    tPos x = SQUARE_TO_X(gSelectedSquare);
    tPos y = SQUARE_TO_Y(gSelectedSquare);
    
    if (y == BOARD_SIZE - 1)
        y = 0;
    else
        y++;
    
    if (x == 0)
        x = BOARD_SIZE - 1;
    else
        x--;
    
    gSelectedSquare = XY_TO_SQUARE(x, y);
    
    refreshSquare(oldSquare);
    selectSquare(gSelectedSquare);
}


static void moveDownRight(void)
{
    tSquare oldSquare = gSelectedSquare;
    tPos x = SQUARE_TO_X(gSelectedSquare);
    tPos y = SQUARE_TO_Y(gSelectedSquare);
    
    if (y == BOARD_SIZE - 1)
        y = 0;
    else
        y++;
    
    if (x == BOARD_SIZE - 1)
        x = 0;
    else
        x++;
    
    gSelectedSquare = XY_TO_SQUARE(x, y);
    
    refreshSquare(oldSquare);
    selectSquare(gSelectedSquare);
}


static void moveLeft(void)
{
    tSquare oldSquare = gSelectedSquare;
    tPos x = SQUARE_TO_X(gSelectedSquare);
    tPos y = SQUARE_TO_Y(gSelectedSquare);
    
    if (x == 0)
        x = BOARD_SIZE - 1;
    else
        x--;
    
    gSelectedSquare = XY_TO_SQUARE(x, y);
    
    refreshSquare(oldSquare);
    selectSquare(gSelectedSquare);
}


static void moveRight(void)
{
    tSquare oldSquare = gSelectedSquare;
    tPos x = SQUARE_TO_X(gSelectedSquare);
    tPos y = SQUARE_TO_Y(gSelectedSquare);
    
    if (x == BOARD_SIZE - 1)
        x = 0;
    else
        x++;
    
    gSelectedSquare = XY_TO_SQUARE(x, y);
    
    refreshSquare(oldSquare);
    selectSquare(gSelectedSquare);
}


static bool swapUp(void)
{
    bool result = false;
    tPos y = SQUARE_TO_Y(gSelectedSquare);
    
    if (y == 0) {
        badThingHappened();
        return result;
    }
    
    resetStarAnim();
    result = moveSquareInDir(gSelectedSquare, DIR_UP);
    selectSquare(gSelectedSquare);
    
    if (result)
        gShouldSave = true;
    
    return result;
}


static bool swapDown(void)
{
    bool result = false;
    tPos y = SQUARE_TO_Y(gSelectedSquare);
    
    if (y == BOARD_SIZE - 1) {
        badThingHappened();
        return result;
    }
    
    resetStarAnim();
    result = moveSquareInDir(gSelectedSquare, DIR_DOWN);
    selectSquare(gSelectedSquare);
    
    if (result)
        gShouldSave = true;
    
    return result;
}


static bool swapLeft(void)
{
    bool result = false;
    tPos x = SQUARE_TO_X(gSelectedSquare);
    
    if (x == 0) {
        badThingHappened();
        return result;
    }
    
    resetStarAnim();
    result = moveSquareInDir(gSelectedSquare, DIR_LEFT);
    selectSquare(gSelectedSquare);
    
    if (result)
        gShouldSave = true;
    
    return result;
}


static bool swapRight(void)
{
    bool result = false;
    tPos x = SQUARE_TO_X(gSelectedSquare);
    
    if (x == BOARD_SIZE - 1) {
        badThingHappened();
        return result;
    }
    
    resetStarAnim();
    result = moveSquareInDir(gSelectedSquare, DIR_RIGHT);
    selectSquare(gSelectedSquare);
    
    if (result)
        gShouldSave = true;
    
    return result;
}


static bool isAppleButtonPressed(void)
{
    return (isButtonPressed(JOY_BUTTON_0) || isButtonPressed(JOY_BUTTON_1));
}


static void endGame(void)
{
    mixedTextMode();
    videomode(VIDEOMODE_80x24);
    cputsxy(0, 20, "               No more moves  -  GAME OVER!!");
    gotoxy(0,21);
    cprintf(       "               You made it to level %u", getLevel());
    cputsxy(0,23,  "                    Play again (Y/N)?");
    
    while (true) {
        switch (cgetc()) {
            case 'y':
            case 'Y':
                return;
                
            case 'n':
            case 'N':
            case CH_ESC:
            case 'q':
            case 'Q':
                quitGame();
                break;
                
            default:
                badThingHappened();
                break;
        }
    }
}


static void refreshScore(tScore score)
{
    if (score == gScoreBar)
        return;
    
    gScoreBar = score;
    drawScore(score);
}


static void refreshLevel(tLevel level)
{
    bool waiting = true;
    
    mixedTextMode();
    videomode(VIDEOMODE_80x24);
    gotoxy(0, 20);
    cprintf(      "               Completed level %u!!", level);
    cputsxy(0,22, "               Press space to continue to the next level...");
    
    while (waiting) {
        switch (cgetc()) {
            case ' ':
                waiting = false;
                break;
                
            default:
                badThingHappened();
                break;
        }
    }
    
    showAndClearDblLoRes();
}


static void getHint(void)
{
    refreshSquare(gSelectedSquare);
    
    gSelectedSquare = getHintSquare();
    selectSquare(gSelectedSquare);
}


void initUI(void)
{
    initGameEngine(&gCallbacks);
    animInit();
    initJoystick(&gJoyCallbacks);
}


static void joystickMove(tJoyPos position)
{
    switch (position) {
        case JOY_POS_DOWN:
            moveDown();
            break;
            
        case JOY_POS_DOWN_LEFT:
            moveDownLeft();
            break;
            
        case JOY_POS_LEFT:
            moveLeft();
            break;
            
        case JOY_POS_UP_LEFT:
            moveUpLeft();
            break;
            
        case JOY_POS_UP:
            moveUp();
            break;
            
        case JOY_POS_UP_RIGHT:
            moveUpRight();
            break;
            
        case JOY_POS_RIGHT:
            moveRight();
            break;
            
        case JOY_POS_DOWN_RIGHT:
            moveDownRight();
            break;
            
        default:
        case JOY_POS_CENTER:
            break;
    }
}


static bool joystickChangedCallback(tJoyState *oldState, tJoyState *newState)
{
    
    if (oldState->position != JOY_POS_CENTER)
        return false;
    
    if ((newState->button0) ||
        (newState->button1)) {
        switch (newState->position) {
            case JOY_POS_UP:
                return swapUp();
                
            case JOY_POS_DOWN:
                return swapDown();
                
            case JOY_POS_LEFT:
                return swapLeft();
                
            case JOY_POS_RIGHT:
                return swapRight();
                
            default:
                break;
        }
        return false;
    }
    
    joystickMove(newState->position);
    
    return false;
}


static bool joystickNoChangeCallback(tJoyState *oldState)
{
    if (oldState->button0)
        return false;
    
    if (oldState->button1)
        return false;
    
    joystickMove(oldState->position);
    return false;
}


static bool pollKeyboard(void)
{
    bool result = false;
    uint8_t ch;
    
    if (!kbhit())
        return result;
    
    ch = cgetc();
    switch (ch) {
        case 'i':
        case 'I':
        case CH_CURS_UP:
            if (!isAppleButtonPressed()) {
                moveUp();
                break;
            }
            // Fallthrough...
        case 139:
            result = swapUp();
            break;
            
        case 'j':
        case 'J':
        case CH_CURS_LEFT:
            if (!isAppleButtonPressed()) {
                moveLeft();
                break;
            }
            // Fallthrough...
        case 136:
            result = swapLeft();
            break;
            
        case 'k':
        case 'K':
        case CH_CURS_RIGHT:
            if (!isAppleButtonPressed()) {
                moveRight();
                break;
            }
            // Fallthrough...
        case 149:
            result = swapRight();
            break;
            
        case 'm':
        case 'M':
        case CH_CURS_DOWN:
            if (!isAppleButtonPressed()) {
                moveDown();
                break;
            }
            // Fallthrough...
        case 138:
            result = swapDown();
            break;
            
        case CH_ESC:
        case 'q':
        case 'Q':
            if (gShouldSave) {
                mixedTextMode();
                videomode(VIDEOMODE_80x24);
                gotoxy(0, 20);
                cprintf("\n\nSaving your game so you can continue\r\n    later...");
                saveGame();
            }
            quitGame();
            break;
            
        case 'r':
        case 'R':
            refreshScore(0);
            startNewGame();
            gShouldSave = false;
            return true;
            
        case 's':
        case 'S':
            toggleSound();
            break;
            
        case 'h':
        case 'H':
            getHint();
            break;
            
        case '?':
            printInstructions();
            showAndClearDblLoRes();
            drawBoard();
            break;
            
        default:
            badThingHappened();
            break;
    }
    
    return result;
}



void playGame(void)
{
    bool gameLoaded = false;
    uint8_t ch;
    
    gScoreBar = 0;
    gShouldSave = false;
    
    printf("\n\nChecking for a saved game...");
    
    if (loadGame()) {
        bool gotAnswer = false;
        
        printf("\n\nYou have a saved game!\n    Would you like to continue it (Y/N)");
        
        while (!gotAnswer) {
            ch = cgetc();
            switch (ch) {
                case 'y':
                case 'Y':
                    printf("\n\nLoading your saved puzzle");
                    gotAnswer = true;
                    gShouldSave = true;
                    gameLoaded = true;
                    break;
                    
                case 'n':
                case 'N':
                    gotAnswer = true;
                    break;
                    
                default:
                    badThingHappened();
                    break;
            }
        }
    }
    
    showAndClearDblLoRes();
    if (!gameLoaded) {
        startNewGame();
    }
    drawBoard();
    while (true) {
        resetStarAnim();
        
        while (true) {
            doStarAnim();
            
            if (pollKeyboard()) {
                break;
            }
            
            if (pollJoystick()) {
                break;
            }
        }
        
        if (gameIsOver()) {
            endGame();
            showAndClearDblLoRes();
            refreshScore(0);
            startNewGame();
            gShouldSave = false;
        }
    }
}
