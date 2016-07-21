/*
 * main.c
 * a2bejwld
 *
 * Created by Jeremy Rand on 2016-07-20.
 * Copyright (c) 2016 Jeremy Rand. All rights reserved.
 *
 */


#include <stdbool.h>

#include "ui.h"


int main(void)
{
    printInstructions();
    
    while (true) {
        playGame();
    }
}
