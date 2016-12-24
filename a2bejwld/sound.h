//
//  sound.h
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-12-18.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//

#ifndef __a2bejwld__sound__
#define __a2bejwld__sound__


#include <stdbool.h>
#include <stdint.h>

#include "mockingboard.h"


extern void soundInit(tSlot mockingBoardSlot, bool enableSpeechChip);
extern void soundShutdown(void);

extern void beginClearGemSound(void);
extern void playSoundForExplodingGem(void);
extern void playSoundForStarringGem(void);
extern void playSoundForSpecialGem(void);
extern void playClearGemSound(uint8_t frame);

extern void playLandingSound(uint8_t numLanded);

extern void speakGo(void);
extern void speakLevelComplete(void);
extern void speakGetReady(void);
extern void speakNoMoreMoves(void);

extern bool speakGood(void);
extern bool speakExcellent(void);
extern bool speakIncredible(void);


#endif /* defined(__a2bejwld__sound__) */
