//
//  mouseWrapper.h
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-08-17.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//

#ifndef __a2bejwld__mouseWrapper__
#define __a2bejwld__mouseWrapper__


#include <stdbool.h>

#include "types.h"


// Types

typedef struct tMouseCallbacks {
    bool (*mouseSelectSquare)(tSquare square);
    bool (*mouseSwapSquare)(tDirection dir);
} tMouseCallbacks;


// API

extern bool initMouse(tMouseCallbacks *callbacks);
extern void shutdownMouse(void);
extern bool pollMouse(void);


#endif /* defined(__a2bejwld__mouseWrapper__) */
