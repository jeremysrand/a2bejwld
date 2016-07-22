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


// API

extern void __fastcall__ showDblLoRes(void);
extern void __fastcall__ clearDblLoRes(void);
extern void __fastcall__ unshowDblLoRes(void);
extern void __fastcall__ mixedTextMode(void);

extern void __fastcall__ drawBgSquare(tSquare square);

extern void __fastcall__ drawBlueGem(tSquare square);
extern void __fastcall__ drawYellowGem(tSquare square);
extern void __fastcall__ drawRedGem(tSquare square);
extern void __fastcall__ drawGreenGem(tSquare square);
extern void __fastcall__ drawOrangeGem(tSquare square);
extern void __fastcall__ drawGreyGem(tSquare square);
extern void __fastcall__ drawPurpleGem(tSquare square);
extern void __fastcall__ drawSpecialGem(tSquare square);

extern void __fastcall__ explodeGemFrame1(tSquare square);
extern void __fastcall__ explodeGemFrame2(tSquare square);
extern void __fastcall__ explodeGemFrame3(tSquare square);
extern void __fastcall__ explodeGemFrame4(tSquare square);
extern void __fastcall__ explodeGemFrame5(tSquare square);
extern void __fastcall__ explodeGemFrame6(tSquare square);

extern void __fastcall__ starGem(tSquare square);

extern void __fastcall__ selectSquare(tSquare square);

extern void __fastcall__ drawScore(uint8_t score);


#endif /* defined(__a2bejwld__dbllores__) */