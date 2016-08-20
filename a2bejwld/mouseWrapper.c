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


// Extern to mouse driver

extern char a2e_stdmou_mou;


// Globals

static tMouseCallbacks *gMouseCallbacks = NULL;
static bool gMouseInstalled = false;


bool initMouse(tMouseCallbacks *callbacks)
{
    if (!gMouseInstalled) {
        if (mouse_install(&mouse_def_callbacks, &a2e_stdmou_mou) == 0) {
            gMouseInstalled = true;
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
    static uint8_t oldX = 0;
    static uint8_t oldY = 0;
    static bool oldMouseDown = false;
    static bool handledMouseDown = false;
    
    struct mouse_info mouseInfo;
    uint8_t newX;
    uint8_t newY;
    bool newMouseDown;
    bool result = false;
    
    if (!gMouseInstalled) {
        return result;
    }
    
    mouse_info(&mouseInfo);
    
    newMouseDown = (mouseInfo.buttons != 0);
    newX = mouseInfo.pos.x / 35;
    newY = mouseInfo.pos.y / 8;
    
    if (newX >= BOARD_SIZE)
        newX = BOARD_SIZE - 1;
    
    if (newY >= BOARD_SIZE)
        newY = BOARD_SIZE - 1;
    
    if (!newMouseDown) {
        handledMouseDown = false;
        
        if ((oldX != newX) ||
            (oldY != newY)) {
            result = gMouseCallbacks->mouseSelectSquare(XY_TO_SQUARE(newX, newY));
        }
    } else if (!handledMouseDown) {
        if (newX < oldX) {
            if (newY == oldY) {
                result = gMouseCallbacks->mouseSwapSquare(XY_TO_SQUARE(oldX, oldY), DIR_LEFT);
            }
            handledMouseDown = true;
        } else if (newX > oldX) {
            if (newY == oldY) {
                result = gMouseCallbacks->mouseSwapSquare(XY_TO_SQUARE(oldX, oldY), DIR_RIGHT);
            }
            handledMouseDown = true;
        } else if (newY < oldY) {
            result = gMouseCallbacks->mouseSwapSquare(XY_TO_SQUARE(oldX, oldY), DIR_UP);
            handledMouseDown = true;
        } else if (newY > oldY) {
            result = gMouseCallbacks->mouseSwapSquare(XY_TO_SQUARE(oldX, oldY), DIR_DOWN);
            handledMouseDown = true;
        }
    }
    
    oldX = newX;
    oldY = newY;
    oldMouseDown = newMouseDown;
    
    return result;
}