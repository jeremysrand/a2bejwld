//
//  game.h
//  a2bejwld
//
//  Created by Jeremy Rand on 2016-07-20.
//  Copyright © 2016 Jeremy Rand. All rights reserved.
//

#ifndef __a2bejwld__game__
#define __a2bejwld__game__


#include <stdbool.h>
#include "types.h"


#define BOARD_SIZE 8
#define NUM_SQUARES (BOARD_SIZE * BOARD_SIZE)
#define MIN_SQUARE 0
#define MAX_SQUARE (MIN_SQUARE + NUM_SQUARES - 1)


#define SQUARE_TO_X(square) ((square) & 0x7)
#define SQUARE_TO_Y(square) ((square) >> 3)
#define XY_TO_SQUARE(x, y) ((y << 3) | x)


#define SCORE_PER_LEVEL 24

typedef struct tGameCallbacks {
    void (*squareCallback)(tSquare square);
    void (*scoreCallback)(tScore score);
    void (*levelCallback)(tLevel level);
} tGameCallbacks;


extern void initGameEngine(tGameCallbacks *callbacks);
extern void startNewGame(void);

extern bool moveSquareInDir(tSquare square, tDirection dir);

extern tGemType gemTypeAtSquare(tSquare square);
extern bool gemIsStarredAtSquare(tSquare square);

extern tLevel getLevel(void);
extern tScore getScore(void);

extern tSquare getHintSquare(void);

extern bool gameIsOver(void);

extern void saveGame(void);

extern bool loadGame(void);


#endif /* defined(__a2bejwld__game__) */
