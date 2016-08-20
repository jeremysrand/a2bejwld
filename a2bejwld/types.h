//
//  types.h
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-07-20.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//

#ifndef __a2bejwld__types__
#define __a2bejwld__types__


#include <stdint.h>


// Defines

#define DIR_NONE  0
#define DIR_UP    1
#define DIR_DOWN  2
#define DIR_LEFT  3
#define DIR_RIGHT 4


#define GEM_NONE    0
#define GEM_GREEN   1
#define GEM_RED     2
#define GEM_PURPLE  3
#define GEM_ORANGE  4
#define GEM_GREY    5
#define GEM_YELLOW  6
#define GEM_BLUE    7
#define GEM_SPECIAL 8

#define GEM_MIN_NORMAL GEM_GREEN
#define GEM_MAX_NORMAL GEM_BLUE


// Typedefs

typedef uint8_t tSquare;
typedef uint8_t tDirection;
typedef uint8_t tGemType;
typedef uint16_t tLevel;
typedef uint8_t tScore;

typedef int8_t tPos;


#endif /* defined(__a2bejwld__types__) */