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

#include "dbllores.h"
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


static bool machineIs2c(uint8_t machineType)
{
    switch (machineType) {
        case APPLE_IIC:
        case APPLE_IIC35:
        case APPLE_IICEXP:
        case APPLE_IICREV:
        case APPLE_IICPLUS:
            return true;
    }
    
    return false;
}


static bool machineIs2GS(uint8_t machineType)
{
    switch (machineType) {
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
    uint8_t machineType = get_ostype();
    
    if (machineIs2c(machineType)) {
        gVblWait = vblWait2c;
    } else if (machineIs2GS(machineType)) {
        vblInit2gs();
        gOldSpeed = setGSSpeed(GS_SPEED_SLOW);
    }
    else if (machineType == APPLE_IIECARD)
        setBuggyDblLoRes();
}


void uninitMachine(void)
{
    if (machineIs2GS(get_ostype())) {
        setGSSpeed(gOldSpeed);
    }
}
