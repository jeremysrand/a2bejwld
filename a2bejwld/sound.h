//
//  sound.h
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-12-18.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//

#ifndef __a2bejwld__sound__
#define __a2bejwld__sound__


#include <stdint.h>


extern void beginClearGemSound(void);
extern void playSoundForExplodingGem(void);
extern void playSoundForStarringGem(void);
extern void playSoundForSpecialGem(void);
extern void playClearGemSound(uint8_t frame);

extern void playLandingSound(uint8_t numLanded);


#endif /* defined(__a2bejwld__sound__) */
