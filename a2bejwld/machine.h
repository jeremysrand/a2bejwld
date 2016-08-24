//
//  machine.h
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-08-23.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//

#ifndef __a2bejwld__machine__
#define __a2bejwld__machine__



// Typedefs

typedef void (*tVblWaitFunction)(void);


// Globals

extern tVblWaitFunction gVblWait;


// API

extern void initMachine(void);
extern void uninitMachine(void);


#endif /* defined(__a2bejwld__machine__) */
