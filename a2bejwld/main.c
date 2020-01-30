/*
 * main.c
 * a2bejwld
 *
 * Created by Jeremy Rand on 2016-07-20.
 * Copyright (c) 2016 Jeremy Rand. All rights reserved.
 *
 */


#include <stdbool.h>
#include <stdlib.h>

#include "ui.h"


// Implementation

#ifdef TOTAL_REPLAY_BUILD
void totalReplayQuit(void)
{
    __asm__ ("BIT $C082");
    __asm__ ("JMP $100");
}
#endif

int main(void)
{
#ifdef TOTAL_REPLAY_BUILD
    atexit(totalReplayQuit);
#endif
    
    initUI();
    
    printInstructions();
    
    playGame();
    
    return 0;
}
