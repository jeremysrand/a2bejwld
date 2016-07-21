//
//  dbllores.h
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-07-20.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//

#ifndef __a2bejwld__dbllores__
#define __a2bejwld__dbllores__

#include "types.h"


void __fastcall__ showDblLoRes(void);
void __fastcall__ clearDblLoRes(void);
void __fastcall__ unshowDblLoRes(void);

void __fastcall__ drawBgSquare(tSquare square);

void __fastcall__ drawBlueGem(tSquare square);
void __fastcall__ drawYellowGem(tSquare square);
void __fastcall__ drawRedGem(tSquare square);
void __fastcall__ drawGreenGem(tSquare square);
void __fastcall__ drawOrangeGem(tSquare square);
void __fastcall__ drawGreyGem(tSquare square);
void __fastcall__ drawPurpleGem(tSquare square);
void __fastcall__ drawSpecialGem(tSquare square);

void __fastcall__ starGem(tSquare square);

void __fastcall__ selectSquare(tSquare square);

void __fastcall__ drawScore(uint8_t score);


#endif /* defined(__a2bejwld__dbllores__) */