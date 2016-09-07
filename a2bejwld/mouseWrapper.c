//
//  mouseWrapper.c
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-08-17.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//


#include <conio.h>
#include <stdio.h>
#include <mouse.h>

#include "mouseWrapper.h"
#include "game.h"
#include "vbl.h"


// Extern to mouse driver

extern char a2e_stdmou_mou;


// Globals

static tMouseCallbacks *gMouseCallbacks = NULL;
static bool gMouseInstalled = false;
static bool gMouseInPoll = false;

static struct mouse_callbacks gMouseDrvCallbacks;


bool initMouse(tMouseCallbacks *callbacks)
{
    if (!gMouseInstalled) {
        gMouseDrvCallbacks.hide = mouse_def_callbacks.hide;
        // This callback is here for the //c VBL which is only detectable
        // through the mouse interrupt.  By registering this as our "show"
        // function, we can ensure that we get called on our VBL interrupt
        // and can unblock our VBL wait function.
        gMouseDrvCallbacks.show = vblIRQCallback;
        gMouseDrvCallbacks.movex = mouse_def_callbacks.movex;
        gMouseDrvCallbacks.movey = mouse_def_callbacks.movey;
        
        if (mouse_install(&gMouseDrvCallbacks, &a2e_stdmou_mou) == 0) {
            gMouseInstalled = true;
            
            // This is required to ensure that the show callback is called
            // by the interrupt handler.  This whole thing is a bit of a
            // hack to get the default mouse interrupt handler to do what
            // we want on the //c to detect the VBL but it works for now.
            mouse_show();
        }
    }
    
    gMouseCallbacks = callbacks;
    return gMouseInstalled;
}
    

void shutdownMouse(void)
{
    if (gMouseInstalled) {
        mouse_uninstall();
        gMouseInstalled = false;
    }
}


bool pollMouse(void)
{
    static uint16_t mouseDownAtX = 0;
    static uint16_t mouseDownAtY = 0;
    static uint8_t oldX = 0;
    static uint8_t oldY = 0;
    static bool oldMouseDown = false;
    static bool handledMouseDown = false;
    
    struct mouse_info mouseInfo;
    bool newMouseDown;
    bool result = false;
    
    if (!gMouseInstalled) {
        return result;
    }
    
    gMouseInPoll = true;
    
    mouse_info(&mouseInfo);
    
    newMouseDown = (mouseInfo.buttons != 0);
    
    if ((oldMouseDown) &&
        (!newMouseDown))
        handledMouseDown = false;
    
    if ((!oldMouseDown) ||
        (handledMouseDown)) {
        uint8_t newX;
        uint8_t newY;
        
        newX = mouseInfo.pos.x / 35;
        newY = mouseInfo.pos.y / 8;
        
        if (newX >= BOARD_SIZE)
            newX = BOARD_SIZE - 1;
        
        if (newY >= BOARD_SIZE)
            newY = BOARD_SIZE - 1;
        
        if ((oldX != newX) ||
            (oldY != newY)) {
            result = gMouseCallbacks->mouseSelectSquare(XY_TO_SQUARE(newX, newY));
        }
        
        oldX = newX;
        oldY = newY;
    }
    
    
    if ((!oldMouseDown) &&
        (newMouseDown)) {
        mouseDownAtX = mouseInfo.pos.x;
        mouseDownAtY = mouseInfo.pos.y;
    }
    
    if ((newMouseDown) &&
        (!handledMouseDown)) {
        if (mouseDownAtX + 35 < mouseInfo.pos.x) {
            result = gMouseCallbacks->mouseSwapSquare(DIR_RIGHT);
            handledMouseDown = true;
        } else if (mouseInfo.pos.x + 35 < mouseDownAtX) {
            result = gMouseCallbacks->mouseSwapSquare(DIR_LEFT);
            handledMouseDown = true;
        } else if (mouseDownAtY + 8 < mouseInfo.pos.y) {
            result = gMouseCallbacks->mouseSwapSquare(DIR_DOWN);
            handledMouseDown = true;
        } else if (mouseInfo.pos.y + 8 < mouseDownAtY) {
            result = gMouseCallbacks->mouseSwapSquare(DIR_UP);
            handledMouseDown = true;
        }
    }
    oldMouseDown = newMouseDown;
    gMouseInPoll = false;
    
    return result;
}


void moveMouseToSquare(tSquare square)
{
    uint16_t newX;
    uint16_t newY;
    
    if (!gMouseInstalled)
        return;
    
    if (gMouseInPoll)
        return;
    
    newX = (SQUARE_TO_X(square) * 35) + 18;
    newY = (SQUARE_TO_Y(square) * 8) + 4;
    
    mouse_move(newX, newY);
}