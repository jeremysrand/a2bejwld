//
//  anim.h
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-07-22.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//

#ifndef __a2bejwld__anim__
#define __a2bejwld__anim__


#include <stdbool.h>

#include "types.h"


// API

extern void animInit(void);

extern void drawGemAtSquare(tSquare square);

extern void beginStarAnim(void);
extern void endStarAnim(void);
extern void doStarAnim(void);

extern void beginClearGemAnim(void);
extern void addClearAtSquare(tSquare square);
extern void undoClearAtSquare(tSquare square);
extern void endClearGemAnim(void);

extern void swapSquares(tSquare square1, tGemType gemType1, bool starred1,
                        tSquare square2, tGemType gemType2, bool starred2);


#endif /* defined(__a2bejwld__anim__) */
