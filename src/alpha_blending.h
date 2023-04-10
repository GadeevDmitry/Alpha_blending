#ifndef ALPHA_BLENDING_H
#define ALPHA_BLENDING_H

#include "picture.h"

void alpha_blending_intrin        (const int *const front,
                                   const int *const back ,
                                         int *const blend, const int width, const int height);

void alpha_blending_intrin_improve(const int *const front,
                                   const int *const back ,
                                         int *const blend, const int width, const int height);

void alpha_blending_simple        (const int *const front,
                                   const int *const back ,
                                         int *const blend, const int width, const int height);

#endif //ALPHA_BLENDING_H
