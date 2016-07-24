//
//    Author: Jeremy Rand
//      Date: July 29, 2012
//
// This is an interface for the Apple // joystick.  Note that I used to use
// the cc65 joystck interface but found it lacking on real hardware.  It
// worked fine on an emulator but I think it was testing the second axis of
// the joystick too quickly leading to inaccuracies.
//


#include <stdint.h>
#include <stdbool.h>


// Defines

#define JOY_POS_CENTER 0
#define JOY_POS_DOWN 1
#define JOY_POS_DOWN_LEFT 2
#define JOY_POS_LEFT 3
#define JOY_POS_UP_LEFT 4
#define JOY_POS_UP 5
#define JOY_POS_UP_RIGHT 6
#define JOY_POS_RIGHT 7
#define JOY_POS_DOWN_RIGHT 8
#define NUM_JOY_POSITIONS 9

#define JOY_BUTTON_0 0
#define JOY_BUTTON_1 1
#define NUM_JOY_BUTTONS 2


// Typedefs

typedef int8_t tJoyButtonNum;

typedef int8_t tJoyPos;

typedef struct tJoyState {
    tJoyPos position;
    bool button0;
    bool button1;
} tJoyState;

typedef struct tJoyCallbacks {
    bool (*joyChanged)(tJoyState *oldState, tJoyState *newState);
    bool (*joyNoChange)(tJoyState *oldState);
    
    // On the Apple //, you cannot read each axis of the joystick as quickly as you might like.
    // You must wait between each read.  This is the number of calls into pollJoystick() before
    // an axis is polled.
    uint8_t pollsPerRead;
    
    // This value tells you how many calls to poll without a change in joystick state for the
    // joyNoChange() callback to be called.  After a change, initialPollsPerRepeat are used.
    // After that, pollsPerRepeat is used.
    uint8_t initialPollsPerRepeat;
    uint8_t pollsPerRepeat;
} tJoyCallbacks;


// API

extern void initJoystick(tJoyCallbacks *callbacks);

extern bool isButtonPressed(tJoyButtonNum buttonNum);

extern bool pollJoystick(void);
