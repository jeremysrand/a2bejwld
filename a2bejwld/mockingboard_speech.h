//
//  mockingboard_speech.h
//  mocktest
//
//  Created by Jeremy Rand on 2016-10-17.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//

#ifndef mockingboard_speech_h
#define mockingboard_speech_h


#include <stdint.h>


extern uint8_t *mockingBoardSpeechData;
extern uint16_t mockingBoardSpeechLen;
extern uint8_t mockingBoardSpeechBusy;
extern uint8_t mockingBoardSpeechPlaying;

extern void mockingBoardSpeechInit(uint8_t slot);
extern void mockingBoardSpeechShutdown(void);
extern void mockingBoardSpeakPriv(void);


#endif /* mockingboard_speech_h */
