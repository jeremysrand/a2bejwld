//
//  dbllores.h
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-07-20.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//

#ifndef __a2bejwld__dbllores__
#define __a2bejwld__dbllores__

#include <stdint.h>


void __fastcall__ showDblLoRes(void);
void __fastcall__ clearDblLoRes(void);

void __fastcall__ drawBgSquare(uint8_t square);

void __fastcall__ drawBlueGem(uint8_t square);
void __fastcall__ drawYellowGem(uint8_t square);
void __fastcall__ drawRedGem(uint8_t square);
void __fastcall__ drawGreenGem(uint8_t square);
void __fastcall__ drawOrangeGem(uint8_t square);
void __fastcall__ drawGreyGem(uint8_t square);
void __fastcall__ drawPurpleGem(uint8_t square);

void __fastcall__ selectSquare(uint8_t square);


#endif /* defined(__a2bejwld__dbllores__) */