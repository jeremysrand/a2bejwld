//
//  machine.c
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-08-23.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//


#include <apple2.h>
#include <stdbool.h>
#include <stdint.h>

#include "machine.h"
#include "vbl.h"


// Typedefs
typedef enum {
    GS_SPEED_SLOW,
    GS_SPEED_FAST
} tMachineGSSpeed;


// Globals

tVblWaitFunction gVblWait = vblWait;

static tMachineGSSpeed gOldSpeed = GS_SPEED_SLOW;


// Implementation


static bool machineIs2c(void)
{
    switch (get_ostype()) {
        case APPLE_IIC:
        case APPLE_IIC35:
        case APPLE_IICEXP:
        case APPLE_IICREV:
        case APPLE_IICPLUS:
            return true;
    }
    
    return false;
}


static bool machineIs2GS(void)
{
    switch (get_ostype()) {
        case APPLE_IIGS:
        case APPLE_IIGS1:
        case APPLE_IIGS3:
            return true;
    }
    
    return false;
}


static tMachineGSSpeed setGSSpeed(tMachineGSSpeed newSpeed)
{
    uint8_t *speedRegister = (uint8_t *)0xc036;
    uint8_t value = *speedRegister;
    tMachineGSSpeed oldSpeed;
    
    if ((value & 0x80) != 0)
        oldSpeed = GS_SPEED_FAST;
    else
        oldSpeed = GS_SPEED_SLOW;
    
    if (oldSpeed != newSpeed) {
        if (newSpeed == GS_SPEED_FAST) {
            value |= 0x80;
        } else {
            value &= 0x7f;
        }
        *speedRegister = value;
    }
    
    return oldSpeed;
}


void initMachine(void)
{
    if (machineIs2c()) {
        gVblWait = vblWait2c;
    } else if (machineIs2GS()) {
        vblInit2gs();
        gOldSpeed = setGSSpeed(GS_SPEED_SLOW);
    }
}


void uninitMachine(void)
{
    if (machineIs2GS()) {
        setGSSpeed(gOldSpeed);
    }
}