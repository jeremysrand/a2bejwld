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

extern void __fastcall__ drawGem(tSquare square);

extern void __fastcall__ explodeGemFrame1(tSquare square);
extern void __fastcall__ explodeGemFrame2(tSquare square);
extern void __fastcall__ explodeGemFrame3(tSquare square);
extern void __fastcall__ explodeGemFrame4(tSquare square);
extern void __fastcall__ explodeGemFrame5(tSquare square);
extern void __fastcall__ explodeGemFrame6(tSquare square);

extern void __fastcall__ starGem(tSquare square);

extern void __fastcall__ selectSquare(tSquare square);

extern void __fastcall__ drawScore(uint8_t score);


// The following functions take the Y position (0-23) as
// their first argument.  The X position (0-39) should
// be in the X register.  The gem type should be in the
// Y register.
extern void __fastcall__ drawGemAtXY(uint8_t y);
extern void __fastcall__ drawAndStarGemAtXY(uint8_t y);


#endif /* defined(__a2bejwld__dbllores__) */
