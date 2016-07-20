/*
 * main.c
 * a2bejwld
 *
 * Created by Jeremy Rand on 2016-07-20.
 * Copyright (c) 2016 Jeremy Rand. All rights reserved.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "dbllores.h"


int main(void)
{
    uint8_t square;
    
    srand(0);
    
    showDblLoRes();
    clearDblLoRes();
    
    for (square = 0; square < 64; square++) {
        drawBgSquare(square);
        
        switch (rand() % 7) {
            case 0:
                drawOrangeGem(square);
                break;
            
            case 1:
                drawBlueGem(square);
                break;
                
            case 2:
                drawRedGem(square);
                break;
                
            case 3:
                drawGreyGem(square);
                break;
                
            case 4:
                drawYellowGem(square);
                break;
                
            case 5:
                drawPurpleGem(square);
                break;
                
            case 6:
                drawGreenGem(square);
                break;
        }
    }
    
    selectSquare(19);
    
    cgetc();
    return 0;
}
