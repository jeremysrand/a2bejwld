//
//    Author: Jeremy Rand
//      Date: July 20, 2012
//
// This is the implementation for the Curta emulator UI.
//


#include <stdio.h>
#include <string.h>

#include "joystick.h"


// Defines

#define PREAD 0xFB1E
#define ROM_SWITCH 0xC082
#define RAM_SWITCH 0xC080
#define BTN0  0xC061
#define BTN1  0xC062

#define JOYSTICK_CENTER 127
#define JOYSTICK_THRESHOLD 60
#define LOWER_THRESHOLD (JOYSTICK_CENTER - JOYSTICK_THRESHOLD)
#define UPPER_THRESHOLD (JOYSTICK_CENTER + JOYSTICK_THRESHOLD)


// Globals

static tJoyCallbacks *gJoyCallbacks = NULL;

static tJoyState gJoyState = {
    JOY_POS_CENTER,
    false,
    false
};

static uint8_t gJoystickTemp;

static uint8_t gJoyPollsToNextRead;
static uint8_t gJoyPollsToNextRepeat;


// Implementation

void initJoystick(tJoyCallbacks *callbacks)
{
    gJoyCallbacks = callbacks;
    gJoyPollsToNextRead = 0;
    gJoyPollsToNextRepeat = callbacks->initialPollsPerRepeat;
}


bool isButtonPressed(tJoyButtonNum buttonNum)
{
    if (buttonNum == JOY_BUTTON_0) {
        __asm__("LDA %w", BTN0);
        __asm__("STA %v", gJoystickTemp);
    } else if (buttonNum == JOY_BUTTON_1) {
        __asm__("LDA %w", BTN1);
        __asm__("STA %v", gJoystickTemp);
    } else {
        return false;
    }
    return ((gJoystickTemp > 127) ? true : false);
}


static uint8_t joystickLeftRight(void)
{
    __asm__("BIT %w", ROM_SWITCH);
    __asm__("LDX #0");
    __asm__("JSR %w", PREAD);
    __asm__("STY %v", gJoystickTemp);
    __asm__("BIT %w", RAM_SWITCH);
    return gJoystickTemp;
}


static uint8_t joystickUpDown(void)
{
    __asm__("BIT %w", ROM_SWITCH);
    __asm__("LDX #1");
    __asm__("JSR %w", PREAD);
    __asm__("STY %v", gJoystickTemp);
    __asm__("BIT %w", RAM_SWITCH);
    return gJoystickTemp;
}


static void readJoystickState(tJoyState *state)
{
    static bool readLeftRight = true;
    static uint8_t axisLeftRight = 127;
    static uint8_t axisUpDown = 127;

    tJoyPos pos = JOY_POS_CENTER;

    if (readLeftRight) {
        int temp = joystickLeftRight();  // Get left/right position
        temp *= 3;
        temp /= 4;
        axisLeftRight /= 4;
        axisLeftRight += temp;
        readLeftRight = false;
    } else {
        int temp = joystickUpDown();
        temp *= 3;
        temp /= 4;
        axisUpDown /= 4;
        axisUpDown += temp;
        readLeftRight = true;
    }

    if (axisLeftRight < LOWER_THRESHOLD) {
        pos = JOY_POS_LEFT;
    } else if (axisLeftRight > UPPER_THRESHOLD) {
        pos = JOY_POS_RIGHT;
    }

    state->button0 = isButtonPressed(0);
    state->button1 = isButtonPressed(1);

    if (axisUpDown < LOWER_THRESHOLD) {
        switch (pos) {
            case JOY_POS_LEFT:
                pos = JOY_POS_UP_LEFT;
                break;
            case JOY_POS_RIGHT:
                pos = JOY_POS_UP_RIGHT;
                break;
            default:
                pos = JOY_POS_UP;
                break;
        }
    } else if (axisUpDown > UPPER_THRESHOLD) {
        switch (pos) {
            case JOY_POS_LEFT:
                pos = JOY_POS_DOWN_LEFT;
                break;
            case JOY_POS_RIGHT:
                pos = JOY_POS_DOWN_RIGHT;
                break;
            default:
                pos = JOY_POS_DOWN;
                break;
        }
    }
    state->position = pos;
}


static bool joystickStateChanged(tJoyState *state1, tJoyState *state2) {
    if ((state1->position != state2->position) ||
        (state1->button0 != state2->button0) ||
        (state1->button1 != state2->button1)) {
        return true;
    }
    return false;
}


bool pollJoystick(void)
{
    bool result = false;
    
    tJoyState newState;
    
    if (gJoyPollsToNextRead > 0) {
        gJoyPollsToNextRead--;
        return result;
    }
    
    gJoyPollsToNextRead = gJoyCallbacks->pollsPerRead;
    
    readJoystickState(&newState);
    if (joystickStateChanged(&newState, &gJoyState)) {
        result = (gJoyCallbacks->joyChanged)(&gJoyState, &newState);
        memcpy(&gJoyState, &newState, sizeof(gJoyState));
        gJoyPollsToNextRepeat = gJoyCallbacks->initialPollsPerRepeat;
    } else {
        if (gJoyPollsToNextRepeat == 0) {
            result = (gJoyCallbacks->joyNoChange)(&gJoyState);
            gJoyPollsToNextRepeat = gJoyCallbacks->pollsPerRepeat;
        } else {
            gJoyPollsToNextRepeat--;
        }
    }
    
    return result;
}
