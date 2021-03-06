//
//  ui.c
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-07-20.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//


#include <conio.h>
#include <ctype.h>
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
#include "sound.h"
#include "text.h"


// Defines

#define SAVE_OPTIONS_FILE "A2BEJWLD.OPTS"
#define VERSION "v2.7"

#define OPTIONS_VERSION_UNSAVED 0
#define OPTIONS_VERSION_V2 2
#define OPTIONS_VERSION 3

#define OPTION_JOYSTICK_ENABLED            (1 << 0)
#define OPTION_MOUSE_ENABLED               (1 << 1)
#define OPTION_SOUND_ENABLED               (1 << 2)
#define OPTION_MOCKINGBOARD_ENABLED        (1 << 3)
#define OPTION_MOCKINGBOARD_SPEECH_ENABLED (1 << 4)

// Typedefs

typedef struct tGameOptionsV2 {
    uint8_t optionsVersion;
    bool enableJoystick;
    bool enableMouse;
    bool enableSound;
    bool enableMockingboard;
    bool enableMockingboardSpeech;
} tGameOptionsV2;

typedef struct tGameOptions {
    uint8_t optionsVersion;
    uint8_t flags;
    char upChar;
    char downChar;
    char leftChar;
    char rightChar;
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
    endDropAnim,
    
    speakGood,
    speakExcellent,
    speakIncredible
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
    OPTIONS_VERSION_UNSAVED,     // optionsVersion
    (OPTION_MOUSE_ENABLED | OPTION_SOUND_ENABLED | OPTION_MOCKINGBOARD_ENABLED | OPTION_MOCKINGBOARD_SPEECH_ENABLED), // flags
    'I',                        // upChar
    'M',                        // downChar
    'J',                        // leftChar
    'K'                         // rightChar
};


// Implementation


static void printChar (char ch)
{
    if (ch == '\n')
        ch = '\r';
    ch |= 0x80;
    cout(ch);
}

static void printString(char * buffer)
{
    char ch;
    
    while (*buffer != '\0') {
        ch = *buffer;
        printChar(ch);
        buffer++;
    }
}


static void printInteger(uint16_t val)
{
    static char buffer[7];
    snprintf(buffer, sizeof(buffer), "%u", val);
    printString(buffer);
}


static void badThingHappened(void)
{
    if ((gGameOptions.flags & OPTION_SOUND_ENABLED) != 0)
        printString("\007");
}


static void showAndClearDblLoRes(void)
{
    showDblLoRes();
    clearDblLoRes();
}


static void saveOptions(void)
{
    FILE *optionsFile = fopen(SAVE_OPTIONS_FILE, "wb");

    if (optionsFile != NULL) {
        gGameOptions.optionsVersion = OPTIONS_VERSION;
        fwrite(&gGameOptions, sizeof(gGameOptions), 1, optionsFile);
        fclose(optionsFile);
    }
}


static bool loadOptions(void)
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
    
    // If we are upgrading from v1 to v2 of the options file, then:
    //   - Force the mouse option on.  This option is now only used to disable the mouse when one is
    //     present.  When no mouse is installed, this option does nothing.
    //   - There used to be a tSlot of the mockingboard where we now have the enableMockingboard boolean.
    //     Overwrite it with true, forcing mockingboard sound to be on if one is detected.
    //   - There used to be a boolean to enable/disable the speech chip on the mockingboard.  It was only
    //     true if the user enabled it.  Now that we can detect the speech chip, the value is default true
    //     and the user can disable speech if they want.
    if (gGameOptions.optionsVersion < OPTIONS_VERSION) {
        tGameOptionsV2 * oldOptions = (tGameOptionsV2 *)&gGameOptions;
        if (oldOptions->enableJoystick)
            gGameOptions.flags = OPTION_JOYSTICK_ENABLED;
        else
            gGameOptions.flags = 0;
        
        if (oldOptions->enableMouse)
            gGameOptions.flags |= OPTION_MOUSE_ENABLED;
        
        if (oldOptions->enableSound)
            gGameOptions.flags |= OPTION_SOUND_ENABLED;
        
        if (oldOptions->enableMockingboard)
            gGameOptions.flags |= OPTION_MOCKINGBOARD_ENABLED;
        
        if (oldOptions->enableMockingboardSpeech)
            gGameOptions.flags |= OPTION_MOCKINGBOARD_SPEECH_ENABLED;
        
        if (gGameOptions.optionsVersion < OPTIONS_VERSION_V2)
            gGameOptions.flags |= (OPTION_MOUSE_ENABLED | OPTION_MOCKINGBOARD_ENABLED | OPTION_MOCKINGBOARD_SPEECH_ENABLED);
        
        gGameOptions.upChar = 'I';
        gGameOptions.downChar = 'M';
        gGameOptions.leftChar = 'J';
        gGameOptions.rightChar = 'K';
        
        gGameOptions.optionsVersion = OPTIONS_VERSION;
    }
    
    return true;
}


static void applyNewOptions(tGameOptions *newOptions)
{
    // If there is no change in game options, then nothing to do.
    if (memcmp(newOptions, &gGameOptions, sizeof(gGameOptions)) == 0) {
        return;
    }
    
    printString("\n\n\n   Saving options...");
    
    if ((gGameOptions.flags & (OPTION_SOUND_ENABLED | OPTION_MOCKINGBOARD_ENABLED | OPTION_MOCKINGBOARD_SPEECH_ENABLED)) !=
        (newOptions->flags & (OPTION_SOUND_ENABLED | OPTION_MOCKINGBOARD_ENABLED | OPTION_MOCKINGBOARD_SPEECH_ENABLED))) {
        // If the sound parameters have changed, then re-init sounds
        soundInit(((newOptions->flags & OPTION_SOUND_ENABLED) != 0),
                  ((newOptions->flags & OPTION_MOCKINGBOARD_ENABLED) != 0),
                  ((newOptions->flags & OPTION_MOCKINGBOARD_SPEECH_ENABLED) !=0));
    }
    
    memcpy(&gGameOptions, newOptions, sizeof(gGameOptions));
    gGameOptions.optionsVersion = OPTIONS_VERSION_UNSAVED;
    saveOptions();
}


static void showCursor(void)
{
    cout(0x20);
}


static void replaceCursor(char ch)
{
    cout(CH_CURS_LEFT);
    printChar(ch);
}


static char getKey(void)
{
    return toupper(cgetc());
}


static bool yorn(void)
{
    char ch;
    bool result = true;
    
    showCursor();
    while (true) {
        ch = getKey();
        
        if (ch == 'N') {
            result = false;
            break;
        }
        
        if (ch == 'Y')
            break;
        
        badThingHappened();
    }
    
    replaceCursor(ch);
    
    return result;
}


static void getSoundOptions(tGameOptions *newOptions)
{
    tSlot slot;
    
    printString("\n\nEnable sounds? (Y/N) ");
    if (yorn()) {
        newOptions->flags |= OPTION_SOUND_ENABLED;
    } else {
        newOptions->flags &= ~OPTION_SOUND_ENABLED;
        return;
    }
    
    // If no mockingboard present, don't bother to ask whether to enable/disable it.
    slot = mockingBoardSlot();
    if (slot == 0)
        return;
    
    printString("\nEnable MockingBoard sound found in slot ");
    printInteger(slot);
    printString("? (Y/N) ");
    if (yorn()) {
        newOptions->flags |= OPTION_MOCKINGBOARD_ENABLED;
    } else {
        newOptions->flags &= ~OPTION_MOCKINGBOARD_ENABLED;
        return;
    }
    
    // If the mockingboard does not have a speech chip, do not prompt whether to
    // enable/disable it.
    if (!mockingBoardHasSpeechChip())
        return;
    
    printString("\nEnable speech on the Mockingboard? (Y/N) ");
    if (yorn()) {
        newOptions->flags |= OPTION_MOCKINGBOARD_SPEECH_ENABLED;
    } else {
        newOptions->flags &= ~OPTION_MOCKINGBOARD_SPEECH_ENABLED;
    }
}


static char getKeyDirection(char *dir, char current, char other1, char other2, char other3)
{
    char ch;
    
    printString("\nKey for ");
    printString(dir);
    printString(" movement (current ");
    printChar(current);
    printString(") ");
    showCursor();
    while (true) {
        ch = getKey();
        if ((isalnum(ch)) &&
            (ch != 'Q') &&
            (ch != 'R') &&
            (ch != 'O') &&
            (ch != 'H') &&
            (ch != other1) &&
            (ch != other2) &&
            (ch != other3))
            break;
        
        badThingHappened();
    }
    replaceCursor(ch);
    
    return ch;
}

static void getKeyboardOptions(tGameOptions *newOptions)
{
    printChar('\n');
    newOptions->upChar = getKeyDirection("up", newOptions->upChar, 0, 0, 0);
    newOptions->downChar = getKeyDirection("down", newOptions->downChar, newOptions->upChar, 0, 0);
    newOptions->leftChar = getKeyDirection("left", newOptions->leftChar, newOptions->upChar, newOptions->downChar, 0);
    newOptions->rightChar = getKeyDirection("right", newOptions->rightChar, newOptions->upChar, newOptions->downChar, newOptions->leftChar);
}


static void selectOptions(void)
{
    tGameOptions newOptions;
    
    unshowDblLoRes();
    videomode(0x12);
    clrscr();
    
    memcpy(&newOptions, &gGameOptions, sizeof(newOptions));
    
    while (true) {
        bool enableSound = ((newOptions.flags & OPTION_SOUND_ENABLED) != 0);
        bool enableMockingboard = ((newOptions.flags & OPTION_MOCKINGBOARD_ENABLED) != 0);
        
        clrscr();
        printString(
               //      0000000001111111111222222222233333333334444444444555555555566666666667
               //      1234567890123456789012345678901234567890123456789012345678901234567890
               "                               Apple // Bejeweled\n"
               "                                    Options\n"
               "\n"
               "                        K - Keyboard control -  ");
        printChar(newOptions.upChar);
        printChar('\n');
        printString("                                               ");
        printChar(newOptions.leftChar);
        printChar(' ');
        printChar(newOptions.rightChar);
        printChar('\n');
        printString("                                                ");
        printChar(newOptions.downChar);
        printChar('\n');
        printString(
               "                        J - Joystick control - ");
        printString(((newOptions.flags & OPTION_JOYSTICK_ENABLED) != 0) ? "Enabled\n" : "Disabled\n");
        if (hasMouse())
        {
            printString(
               "                        M - Mouse control    - ");
            printString(((newOptions.flags & OPTION_MOUSE_ENABLED) != 0) ? "Enabled\n" : "Disabled\n");
        }
        printString(
               "                        S - Sound            - ");
        printString(enableSound ? "Enabled\n" : "Disabled\n");
        
        if (enableSound) {
            tSlot slot = mockingBoardSlot();
            
            if (slot != 0) {
                printString(
                       //      0000000001111111111222222222233333333334444444444555555555566666666667
                       //      1234567890123456789012345678901234567890123456789012345678901234567890
                       "                                MockingBoard - ");
                printString(enableMockingboard ? "Enabled (Slot " : "Disabled (Slot ");
                printInteger(slot);
                printString(")\n");
                
                if ((enableMockingboard) &&
                    (mockingBoardHasSpeechChip()))
                {
                    printString(
                    //      0000000001111111111222222222233333333334444444444555555555566666666667
                    //      1234567890123456789012345678901234567890123456789012345678901234567890
                    "                                      Speech - ");
                    printString(((newOptions.flags & OPTION_MOCKINGBOARD_SPEECH_ENABLED) != 0) ? "Enabled\n" : "Disabled\n");
                }
            }
        }
        printString(
               //      0000000001111111111222222222233333333334444444444555555555566666666667
               //      1234567890123456789012345678901234567890123456789012345678901234567890
               "\n"
               "       Type a letter to change a setting or any other key to save settings\n"
               "       and continue");
        
        switch (getKey()) {
            case 'J':
                newOptions.flags ^= OPTION_JOYSTICK_ENABLED;
                break;
                
            case 'S':
                getSoundOptions(&newOptions);
                break;
                
            case 'K':
                getKeyboardOptions(&newOptions);
                break;

            case 'M':
                if (hasMouse()) {
                    newOptions.flags ^= OPTION_MOUSE_ENABLED;
                    break;
                }
                // Fall through.  If no mouse, then pressing m is a fall through into the save code.
                
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
    videomode(0x12);
    clrscr();
    printString(
          //      0000000001111111111222222222233333333334444444444555555555566666666667
          //      1234567890123456789012345678901234567890123456789012345678901234567890
           "                              Apple // Bejeweled             (" VERSION ")\n"
           "                                by Jeremy Rand\n"
           "\n"
           "     Use ");
    
    printChar(gGameOptions.upChar);
    printChar('-');
    printChar(gGameOptions.leftChar);
    printChar('-');
    printChar(gGameOptions.rightChar);
    printChar('-');
    printChar(gGameOptions.downChar);
    
    printString(
                           " the arrow keys, joystick or mouse to move your selection.\n"
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
    
    switch (getKey()) {
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
    videomode(0x11);
    clrscr();
    shutdownMouse();
    soundShutdown();
    
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


static void endGame(void)
{
    char ch;
    
    videomode(0x12);
    mixedTextMode();
    
    speakNoMoreMoves();
    
    printString("               No more moves  -  GAME OVER!!\n"
                "               You made it to level ");
    printInteger(getLevel());
    printString("\n"
                "                    Play again (Y/N)? ");
    
    showCursor();
    while (true) {
        ch = getKey();
        switch (ch) {
            case 'Y':
                replaceCursor(ch);
                printString("\n");
                return;
                
            case 'N':
            case CH_ESC:
            case 'Q':
                replaceCursor(ch);
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
    
    videomode(0x12);
    mixedTextMode();
    speakLevelComplete();
    
    printString("\n"
                "         Completed level ");
    printInteger(level);
    printString("!!\n"
                "         Press space to continue to the next level...");
    
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
    
    printString("\n");
    showAndClearDblLoRes();
    speakGetReady();
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
    
    initMachine();
    
    optionsLoaded = loadOptions();
    
    soundInit(((gGameOptions.flags & OPTION_SOUND_ENABLED) != 0),
              ((gGameOptions.flags & OPTION_MOCKINGBOARD_ENABLED) != 0),
              ((gGameOptions.flags & OPTION_MOCKINGBOARD_SPEECH_ENABLED) != 0));
    
    initGameEngine(&gCallbacks);
    
    initMouse(&gMouseCallbacks);
    
    initJoystick(&gJoyCallbacks);
    
    if (gGameOptions.optionsVersion == OPTIONS_VERSION_UNSAVED) {
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
    
    if (newState->button) {
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
    if (oldState->button)
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
    
    ch = getKey();
    if (ch == gGameOptions.upChar)
        ch = 0x0b;
    else if (ch == gGameOptions.downChar)
        ch = 0x0a;
    else if (ch == gGameOptions.leftChar)
        ch = CH_CURS_LEFT;
    else if (ch == gGameOptions.rightChar)
        ch = CH_CURS_RIGHT;
    
    if ((ch < 128) &&
        (isButtonPressed()))
        ch += 128;
    
    switch (ch) {
        // case CH_CURS_UP:
        case 0x0b:
            moveDir(DIR_UP);
            break;
            
        case 139:
            result = swapDir(DIR_UP);
            break;
            
        case CH_CURS_LEFT:
            moveDir(DIR_LEFT);
            break;
            
        case 136:
            result = swapDir(DIR_LEFT);
            break;
            
        case CH_CURS_RIGHT:
            moveDir(DIR_RIGHT);
            break;
            
        case 149:
            result = swapDir(DIR_RIGHT);
            break;
            
        // case CH_CURS_DOWN:
        case 0x0a:
            moveDir(DIR_DOWN);
            break;
            
        case 138:
            result = swapDir(DIR_DOWN);
            break;
            
        case CH_ESC:
        case 'Q':
            if (gShouldSave) {
                videomode(0x12);
                mixedTextMode();
                printString("\nSaving your game so you can continue\n    later...");
                saveGame();
            }
            quitGame();
            break;
            
        case 'R':
            refreshScore(0);
            startNewGame();
            gShouldSave = false;
            return true;
            
        case 'O':
            selectOptions();
            showAndClearDblLoRes();
            drawBoard();
            break;
            
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
    
    printString("\n\nChecking for a saved game...");
    
    if (loadGame()) {
        bool loadSavedGame = false;
        
        printString("\n\nYou have a saved game!\n    Would you like to continue it (Y/N) ");
        loadSavedGame = yorn();
        if (loadSavedGame) {
            printString("\n\nLoading your saved puzzle");
            gShouldSave = true;
            gameLoaded = true;
        }
    }
    
    showAndClearDblLoRes();
    if (!gameLoaded) {
        startNewGame();
    }
    drawBoard();
    speakGo();
    while (true) {
        if (gameIsOver()) {
            endGame();
            showAndClearDblLoRes();
            refreshScore(0);
            startNewGame();
            gShouldSave = false;
        }
        
        resetStarAnim();
        
        while (true) {
            doStarAnim();
            
            if (pollKeyboard()) {
                break;
            }
            
            if (((gGameOptions.flags & OPTION_JOYSTICK_ENABLED) != 0) &&
                (pollJoystick())) {
                break;
            }
            
            if (((gGameOptions.flags & OPTION_MOUSE_ENABLED) != 0) &&
                (pollMouse())) {
                break;
            }
        }
    }
}
