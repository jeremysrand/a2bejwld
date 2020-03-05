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

int main(void)
{
    initUI();
    
    printInstructions();
    
    playGame();
    
    return 0;
}
