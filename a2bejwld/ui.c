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


// Defines

#define BTN1  0xC062


// Forward delcarations

static void refreshSquare(tSquare square);
static void refreshScore(tScore score);
static void refreshLevel(tLevel level);


// Globals

static tSquare gSelectedSquare = 0;
static bool gPlaySounds = true;
static uint8_t gScoreBar = 0;

static tGameCallbacks gCallbacks = {
    refreshSquare,
    refreshScore,
    refreshLevel,
    beginClearGemAnim,
    addClearAtSquare,
    undoClearAtSquare,
    endClearGemAnim,
    swapSquares,
};


// Implementation

static void showAndClearDblLoRes(void)
{
    showDblLoRes();
    clearDblLoRes();
}


static void badThingHappened(void)
{
    if (gPlaySounds)
        printf("\007");
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
           "     Use I-J-K-M or the arrow keys to move your selection.  Hold the closed\n"
           "     apple key and move your selection to swap two jewels and match 3 or\n"
           "     more jewels.  When you match three jewels, they disappear and new\n"
           "     jewels will drop from the top.\n"
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
    
    result = moveSquareInDir(gSelectedSquare, DIR_UP);
    selectSquare(gSelectedSquare);
    
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
    
    result = moveSquareInDir(gSelectedSquare, DIR_DOWN);
    selectSquare(gSelectedSquare);
    
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
    
    result = moveSquareInDir(gSelectedSquare, DIR_LEFT);
    selectSquare(gSelectedSquare);
    
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
    
    result = moveSquareInDir(gSelectedSquare, DIR_RIGHT);
    selectSquare(gSelectedSquare);
    
    return result;
}


static bool isAppleButtonPressed(void)
{
    static uint8_t temp;
    
    __asm__("LDA %w", BTN1);
    __asm__("STA %v", temp);
    
    return ((temp > 127) ? true : false);
}


static void endGame(void)
{
    mixedTextMode();
    videomode(VIDEOMODE_80x24);
    cputsxy(0, 20, "                    No more moves  -  GAME OVER!!");
    gotoxy(0,21);
    cprintf(       "                    You made it to level %u", getLevel());
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
    cprintf(      "                    Completed level %u!!", level);
    cputsxy(0,22, "                    Press space to continue to the next level...");
    
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
}


void playGame(void)
{
    static bool firstGame = true;
    bool shouldSave = false;
    bool gameLoaded = false;
    bool checkForGameOver = false;
    uint8_t ch;
    
    gScoreBar = 0;
    
    if (firstGame) {
        firstGame = false;
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
                        shouldSave = true;
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
    }
    
    showAndClearDblLoRes();
    if (!gameLoaded) {
        startNewGame();
    }
    drawBoard();
    while (true) {
        if ((checkForGameOver) &&
            (gameIsOver())) {
            endGame();
            return;
        }
        checkForGameOver = false;
        
        beginStarAnim();
        while (!kbhit()) {
            doStarAnim();
        }
        endStarAnim();
    
        ch = cgetc();
        switch (ch) {
            case 'i':
            case 'I':
            case CH_CURS_UP:
                shouldSave = true;
                if (isAppleButtonPressed())
                    checkForGameOver = swapUp();
                else
                    moveUp();
                break;
                
            case 'j':
            case 'J':
            case CH_CURS_LEFT:
                shouldSave = true;
                if (isAppleButtonPressed())
                    checkForGameOver = swapLeft();
                else
                    moveLeft();
                break;
                
            case 'k':
            case 'K':
            case CH_CURS_RIGHT:
                shouldSave = true;
                if (isAppleButtonPressed())
                    checkForGameOver = swapRight();
                else
                    moveRight();
                break;
                
            case 'm':
            case 'M':
            case CH_CURS_DOWN:
                shouldSave = true;
                if (isAppleButtonPressed())
                    checkForGameOver = swapDown();
                else
                    moveDown();
                break;
                
            case CH_ESC:
            case 'q':
            case 'Q':
                if (shouldSave) {
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
                return;
                
            case 's':
            case 'S':
                gPlaySounds = !gPlaySounds;
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
    }
}
