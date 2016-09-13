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
#include <string.h>

#include "ui.h"
#include "anim.h"
#include "dbllores.h"
#include "game.h"
#include "joystick.h"
#include "machine.h"
#include "mouseWrapper.h"


// Defines

#define SAVE_OPTIONS_FILE "a2bejwld.opts"
#define VERSION "v1.2"


// Typedefs

typedef struct tGameOptions {
    bool optionsSaved;
    bool enableJoystick;
    bool enableMouse;
    bool enableSound;
} tGameOptions;


// Forward delcarations

static void refreshSquare(tSquare square);
static void refreshScore(tScore score);
static void refreshLevel(tLevel level);

static bool joystickChangedCallback(tJoyState *oldState, tJoyState *newState);
static bool joystickNoChangeCallback(tJoyState *oldState);

static bool mouseSelectSquare(tSquare square);
static bool swapDir(tDirection dir);


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

static tMouseCallbacks gMouseCallbacks = {
    mouseSelectSquare,
    swapDir,
};

static bool gShouldSave = false;

static tGameOptions gGameOptions = {
    false,
    false,
    true,
    true,
};


// Implementation


void badThingHappened(void)
{
    if (gGameOptions.enableSound)
        printf("\007");
}


void playSound(int8_t startFreq, int8_t duration)
{
    int8_t freq;
    
    if (!gGameOptions.enableSound)
        return;
    
    while (duration > 0) {
        asm ("STA %w", 0xc030);
        freq = startFreq;
        while (freq > 0) {
            freq--;
        }
        duration--;
    }
}


static void showAndClearDblLoRes(void)
{
    showDblLoRes();
    clearDblLoRes();
}


void saveOptions(void)
{
    FILE *optionsFile = fopen(SAVE_OPTIONS_FILE, "wb");

    if (optionsFile != NULL) {
        gGameOptions.optionsSaved = true;
        fwrite(&gGameOptions, sizeof(gGameOptions), 1, optionsFile);
        fclose(optionsFile);
    }
}


bool loadOptions(void)
{
    FILE *optionsFile = fopen(SAVE_OPTIONS_FILE, "rb");
    
    if (optionsFile == NULL) {
        return false;
    }
    
    if (fread(&gGameOptions, sizeof(gGameOptions), 1, optionsFile) != 1) {
        fclose(optionsFile);
        return false;
    }
    
    fclose(optionsFile);
    
    return true;
}


void applyNewOptions(tGameOptions *newOptions)
{
    bool oldEnableMouse = gGameOptions.enableMouse;
    
    // If there is no change in game options, then nothing to do.
    if (memcmp(newOptions, &gGameOptions, sizeof(gGameOptions)) == 0) {
        return;
    }
    
    memcpy(&gGameOptions, newOptions, sizeof(gGameOptions));
    gGameOptions.optionsSaved = false;
    if (oldEnableMouse != gGameOptions.enableMouse) {
        if (gGameOptions.enableMouse) {
            gGameOptions.enableMouse = initMouse(&gMouseCallbacks);
        }
    }
    saveOptions();
}


void selectOptions(void)
{
    tGameOptions newOptions;
    
    unshowDblLoRes();
    videomode(VIDEOMODE_80x24);
    clrscr();
    
    memcpy(&newOptions, &gGameOptions, sizeof(newOptions));
    
    while (true) {
        clrscr();
        printf(
               //      0000000001111111111222222222233333333334444444444555555555566666666667
               //      1234567890123456789012345678901234567890123456789012345678901234567890
               "                               Apple // Bejeweled\n"
               "                                    Options\n"
               "\n"
               "                        J - Joystick control - %s\n"
               "                        M - Mouse control    - %s\n"
               "                        S - Sound            - %s\n"
               "\n"
               "       Type a letter to toggle a setting or any other key to save settings\n"
               "       and continue",
               (newOptions.enableJoystick ? "Enable" : "Disabled"),
               (newOptions.enableMouse ? "Enable" : "Disabled"),
               (newOptions.enableSound ? "Enable" : "Disabled"));
        
        switch (cgetc()) {
            case 'j':
            case 'J':
                newOptions.enableJoystick = !newOptions.enableJoystick;
                if (newOptions.enableJoystick) {
                    newOptions.enableMouse = false;
                }
                break;
                
            case 'm':
            case 'M':
                newOptions.enableMouse = !newOptions.enableMouse;
                if (newOptions.enableMouse) {
                    newOptions.enableJoystick = false;
                }
                break;
                
            case 's':
            case 'S':
                newOptions.enableSound = !newOptions.enableSound;
                break;
                
            default:
                applyNewOptions(&newOptions);
                clrscr();
                return;
        }
    }
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
           "                              Apple // Bejeweled             (" VERSION ")\n"
           "                                by Jeremy Rand\n"
           "\n"
           "     Use I-J-K-M, the arrow keys, joystick or mouse to move your selection.\n"
           "     Hold either apple key, joystick or mouse button and move your selection\n"
           "     to swap two jewels and match 3 or more jewels.  When you match three\n"
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
           "                    Press O to select options.\n"
           "                    Press H to get a hint.\n"
           "                    Press ? to see this info again.\n"
           "\n"
           "                           Press any key to start");
    
    // The amount of time the user waits to read the in
    while (!kbhit())
        seed++;
    
    srand(seed);
    
    switch (cgetc()) {
        case 'o':
        case 'O':
            selectOptions();
            break;
            
        default:
            break;
    }
    
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
    moveMouseToSquare(gSelectedSquare);
    drawScore(gScoreBar);
}


static void quitGame(void)
{
    unshowDblLoRes();
    videomode(VIDEOMODE_40x24);
    clrscr();
    shutdownMouse();
    
    uninitMachine();
    
    exit(0);
}


static void moveDir(tDirection dir)
{
    tSquare oldSquare = gSelectedSquare;
    tPos x = SQUARE_TO_X(gSelectedSquare);
    tPos y = SQUARE_TO_Y(gSelectedSquare);
    
    switch (dir) {
        case DIR_UP:
            if (y == 0)
                y = BOARD_SIZE - 1;
            else
                y--;
            break;
            
        case DIR_DOWN:
            if (y == BOARD_SIZE - 1)
                y = 0;
            else
                y++;
            break;
            
        case DIR_LEFT:
            if (x == 0)
                x = BOARD_SIZE - 1;
            else
                x--;
            break;
            
        case DIR_RIGHT:
            if (x == BOARD_SIZE - 1)
                x = 0;
            else
                x++;
            break;
    }
    
    gSelectedSquare = XY_TO_SQUARE(x, y);
    
    refreshSquare(oldSquare);
    selectSquare(gSelectedSquare);
    moveMouseToSquare(gSelectedSquare);
}


static void moveTwoDirs(tDirection dir1, tDirection dir2)
{
    tSquare oldSquare = gSelectedSquare;
    tPos x = SQUARE_TO_X(gSelectedSquare);
    tPos y = SQUARE_TO_Y(gSelectedSquare);
    
    switch (dir1) {
        case DIR_UP:
            if (y == 0)
                y = BOARD_SIZE - 1;
            else
                y--;
            break;
            
        case DIR_DOWN:
            if (y == BOARD_SIZE - 1)
                y = 0;
            else
                y++;
            break;
    }
    
    switch (dir2) {
        case DIR_LEFT:
            if (x == 0)
                x = BOARD_SIZE - 1;
            else
                x--;
            break;
            
        case DIR_RIGHT:
            if (x == BOARD_SIZE - 1)
                x = 0;
            else
                x++;
            break;
    }
    
    gSelectedSquare = XY_TO_SQUARE(x, y);
    
    refreshSquare(oldSquare);
    selectSquare(gSelectedSquare);
    moveMouseToSquare(gSelectedSquare);
}


static bool swapDir(tDirection dir)
{
    bool result = false;
    tPos x = SQUARE_TO_X(gSelectedSquare);
    tPos y = SQUARE_TO_Y(gSelectedSquare);
    
    switch (dir) {
        case DIR_UP:
            if (y == 0) {
                badThingHappened();
                return result;
            }
            break;
            
        case DIR_DOWN:
            if (y == BOARD_SIZE - 1) {
                badThingHappened();
                return result;
            }
            break;
            
        case DIR_LEFT:
            if (x == 0) {
                badThingHappened();
                return result;
            }
            break;
            
        case DIR_RIGHT:
            if (x == BOARD_SIZE - 1) {
                badThingHappened();
                return result;
            }
            break;
    }
    
    resetStarAnim();
    result = moveSquareInDir(gSelectedSquare, dir);
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
    videomode(VIDEOMODE_80x24);
    mixedTextMode();
    
    cputsxy(0, 0, "               No more moves  -  GAME OVER!!");
    gotoxy(0,1);
    cprintf(      "               You made it to level %u", getLevel());
    cputsxy(0, 3, "                    Play again (Y/N)?");
    
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
    
    videomode(VIDEOMODE_80x24);
    mixedTextMode();
    
    gotoxy(0, 0);
    cprintf(      "               Completed level %u!!", level);
    cputsxy(0, 2, "               Press space to continue to the next level...");
    
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
    moveMouseToSquare(gSelectedSquare);
}


void initUI(void)
{
    bool optionsLoaded;
    bool mouseInitialized;
    
    initMachine();
    
    optionsLoaded = loadOptions();
    
    initGameEngine(&gCallbacks);
    mouseInitialized = initMouse(&gMouseCallbacks);
    
    // If we couldn't initialize a mouse and it was enabled on the options, then disable it.
    if ((!mouseInitialized) &&
        (gGameOptions.enableMouse)) {
        gGameOptions.enableMouse = false;
        gGameOptions.optionsSaved = false;
        
        // If there were no options loaded, then let's turn on the joystick instead.
        if (!optionsLoaded) {
            gGameOptions.enableJoystick = true;
        }
    }
    
    initJoystick(&gJoyCallbacks);
    
    if (!gGameOptions.optionsSaved) {
        saveOptions();
    }
}


static bool mouseSelectSquare(tSquare square)
{
    refreshSquare(gSelectedSquare);
    gSelectedSquare = square;
    selectSquare(gSelectedSquare);

    return false;
}


static void joystickMove(tJoyPos position)
{
    switch (position) {
        case JOY_POS_DOWN:
            moveDir(DIR_DOWN);
            break;
            
        case JOY_POS_DOWN_LEFT:
            moveTwoDirs(DIR_DOWN, DIR_LEFT);
            break;
            
        case JOY_POS_LEFT:
            moveDir(DIR_LEFT);
            break;
            
        case JOY_POS_UP_LEFT:
            moveTwoDirs(DIR_UP, DIR_LEFT);
            break;
            
        case JOY_POS_UP:
            moveDir(DIR_UP);
            break;
            
        case JOY_POS_UP_RIGHT:
            moveTwoDirs(DIR_UP, DIR_RIGHT);
            break;
            
        case JOY_POS_RIGHT:
            moveDir(DIR_RIGHT);
            break;
            
        case JOY_POS_DOWN_RIGHT:
            moveTwoDirs(DIR_DOWN, DIR_RIGHT);
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
                return swapDir(DIR_UP);
                
            case JOY_POS_DOWN:
                return swapDir(DIR_DOWN);
                
            case JOY_POS_LEFT:
                return swapDir(DIR_LEFT);
                
            case JOY_POS_RIGHT:
                return swapDir(DIR_RIGHT);
                
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
                moveDir(DIR_UP);
                break;
            }
            // Fallthrough...
        case 139:
            result = swapDir(DIR_UP);
            break;
            
        case 'j':
        case 'J':
        case CH_CURS_LEFT:
            if (!isAppleButtonPressed()) {
                moveDir(DIR_LEFT);
                break;
            }
            // Fallthrough...
        case 136:
            result = swapDir(DIR_LEFT);
            break;
            
        case 'k':
        case 'K':
        case CH_CURS_RIGHT:
            if (!isAppleButtonPressed()) {
                moveDir(DIR_RIGHT);
                break;
            }
            // Fallthrough...
        case 149:
            result = swapDir(DIR_RIGHT);
            break;
            
        case 'm':
        case 'M':
        case CH_CURS_DOWN:
            if (!isAppleButtonPressed()) {
                moveDir(DIR_DOWN);
                break;
            }
            // Fallthrough...
        case 138:
            result = swapDir(DIR_DOWN);
            break;
            
        case CH_ESC:
        case 'q':
        case 'Q':
            if (gShouldSave) {
                videomode(VIDEOMODE_80x24);
                mixedTextMode();
                gotoxy(0, 0);
                cprintf("Saving your game so you can continue\r\n    later...");
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
            
        case 'o':
        case 'O':
            selectOptions();
            showAndClearDblLoRes();
            drawBoard();
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
            
            if ((gGameOptions.enableJoystick) &&
                (pollJoystick())) {
                break;
            }
            
            if ((gGameOptions.enableMouse) &&
                (pollMouse())) {
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
