#ifndef ALPHA_BLENDING_H
#define ALPHA_BLENDING_H

#include "picture.h"

void alpha_blending(const int *const front,
                    const int *const back ,
                          int *const blend, const int width, const int height);

#endif //ALPHA_BLENDING_H