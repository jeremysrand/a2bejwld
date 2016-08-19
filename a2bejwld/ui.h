//
//  ui.h
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-07-20.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//

#ifndef __a2bejwld__ui__
#define __a2bejwld__ui__


#include <stdint.h>


// API

extern void initUI(void);

extern void printInstructions(void);

extern void playGame(void);

extern void playSound(int8_t startFreq, int8_t duration);


#endif /* defined(__a2bejwld__ui__) */
