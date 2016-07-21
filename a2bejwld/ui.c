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
#include "dbllores.h"
#include "game.h"


#define BTN1  0xC062


static tSquare gSelectedSquare = 0;
static bool gPlaySounds = true;


static void initUI(void)
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
           //                     0000000001111111111222222222233333333334
           //                     1234567890123456789012345678901234567890
           "                                  Apple Jeweled\n"
           "\n"
           "                    Use I-J-K-M or the arrow keys to move\n"
           "                    your selection.  Hold the closed apple key\n"
           "                    and move your selection to swap two jewels\n"
           "                    and match 3 or more jewels.\n"
           "\n"
           "                    Play ends when no more matches can be\n"
           "                    made.\n"
           "\n"
           "                    Press escape or Q to quit at any time.\n"
           "                    Press R to start a new game.\n"
           "                    Press S to toggle sound.\n"
           "                    Press H to see this info again.\n"
           "\n"
           "\n"
           "\n"
           "                           Press any key to start");
    
    // The amount of time the user waits to read the in
    while (!kbhit())
        seed++;
    
    cgetc();
    srand(seed);
    
    clrscr();
    initUI();
}


static void drawGemAtSquare(tSquare square)
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
            
        default:
            break;
    }
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


static void swapUp(void)
{
    tPos y = SQUARE_TO_Y(gSelectedSquare);
    
    if (y == 0) {
        badThingHappened();
        return;
    }
    
    moveSquareInDir(gSelectedSquare, DIR_UP);
    selectSquare(gSelectedSquare);
}


static void swapDown(void)
{
    tPos y = SQUARE_TO_Y(gSelectedSquare);
    
    if (y == BOARD_SIZE - 1) {
        badThingHappened();
        return;
    }
    
    moveSquareInDir(gSelectedSquare, DIR_DOWN);
    selectSquare(gSelectedSquare);
}


static void swapLeft(void)
{
    tPos x = SQUARE_TO_X(gSelectedSquare);
    
    if (x == 0) {
        badThingHappened();
        return;
    }
    
    moveSquareInDir(gSelectedSquare, DIR_LEFT);
    selectSquare(gSelectedSquare);
}


static void swapRight(void)
{
    tPos x = SQUARE_TO_X(gSelectedSquare);
    
    if (x == BOARD_SIZE - 1) {
        badThingHappened();
        return;
    }
    
    moveSquareInDir(gSelectedSquare, DIR_RIGHT);
    selectSquare(gSelectedSquare);
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
    unshowDblLoRes();
    videomode(VIDEOMODE_80x24);
    clrscr();
    
    printf("GAME OVER!!!\n");
}


void playGame(void)
{
    initGame(refreshSquare);
    
    initUI();
    drawBoard();
    while (true) {
        uint8_t ch;
        
        if (gameIsOver()) {
            endGame();
            return;
        }
        
        while (!kbhit()) {
            // Maybe do some animation stuff here...
        }
    
        ch = cgetc();
        switch (ch) {
            case 'i':
            case 'I':
            case CH_CURS_UP:
                if (isAppleButtonPressed())
                    swapUp();
                else
                    moveUp();
                break;
                
            case 'j':
            case 'J':
            case CH_CURS_LEFT:
                if (isAppleButtonPressed())
                    swapLeft();
                else
                    moveLeft();
                break;
                
            case 'k':
            case 'K':
            case CH_CURS_RIGHT:
                if (isAppleButtonPressed())
                    swapRight();
                else
                    moveRight();
                break;
                
            case 'm':
            case 'M':
            case CH_CURS_DOWN:
                if (isAppleButtonPressed())
                    swapDown();
                else
                    moveDown();
                break;
                
            case CH_ESC:
            case 'q':
            case 'Q':
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
                printInstructions();
                drawBoard();
                break;
                
            default:
                badThingHappened();
                break;
        }
    }
}
