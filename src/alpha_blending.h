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

bool alpha_blending_ctor(alpha_blending *const alpha, unsigned *const front, unsigned *const back, const int size_x, const int size_y);
void alpha_blending_dtor(alpha_blending *const alpha);
//--------------------------------------------------------------------------------------------------------------------------------
void alpha_blending_reculc(alpha_blending *const alpha);
void alpha_blending_draw  (alpha_blending *const alpha, sf::RenderWindow *const wnd);

#endif //ALPHA_BLENDING_H