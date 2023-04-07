#ifndef ALPHA_BLENDING_H
#define ALPHA_BLENDING_H

struct alpha_blending
{
    unsigned *front;
    unsigned *back;
    unsigned *blend;

    int size_total;
    int size_x;
    int size_y;
};

#endif //ALPHA_BLENDING_H