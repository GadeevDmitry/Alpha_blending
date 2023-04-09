#ifndef PICTURE_H
#define PICTURE_H

#include <SFML/Graphics.hpp>

//================================================================================================================================
// V2_VECTOR
//================================================================================================================================

struct v2_vector
{
    int x;
    int y;
};

bool v2_vector_ctor(v2_vector *const vec, const int x, const int y);

//================================================================================================================================
// PICTURE
//================================================================================================================================

struct picture
{
    unsigned *pixels;
    unsigned  pixels_size;

    v2_vector size;
};

bool picture_ctor(picture *const paint, unsigned *const pixels, const v2_vector size);
bool picture_ctor(picture *const paint, const char *bmp32_filename);
void picture_dtor(picture *const paint);
//--------------------------------------------------------------------------------------------------------------------------------
void picture_draw(picture *const paint, sf::RenderWindow *const wnd);

//================================================================================================================================
// FRAME
//================================================================================================================================

struct frame
{
    picture   content;

    v2_vector  offset;
};

bool frame_ctor      (frame *const segment, picture *const content    , const v2_vector offset = {0, 0});
bool frame_ctor      (frame *const segment, const char *bmp32_filename, const v2_vector offset = {0, 0});
bool frame_set_offset(frame *const segment,                             const v2_vector offset);
void frame_dtor      (frame *const segment);
//--------------------------------------------------------------------------------------------------------------------------------
void frame_draw      (frame *const segment, sf::RenderWindow *const wnd);

/*
struct piece
{
    picture    *main;

    v2_vector   size;
    v2_vector offset;
};
*/

#endif //PICTURE_H