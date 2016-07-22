//
//  anim.h
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-07-22.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//

#ifndef __a2bejwld__anim__
#define __a2bejwld__anim__


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


#endif /* defined(__a2bejwld__anim__) */
