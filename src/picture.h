#ifndef PICTURE_H
#define PICTURE_H

//================================================================================================================================
// ALL
//================================================================================================================================

struct v2_vector
{
    int x;
    int y;
};
//--------------------------------------------------------------------------------------------------------------------------------
struct picture
{
    unsigned *pixels;
    unsigned  pixels_size;

    v2_vector size;
};
//--------------------------------------------------------------------------------------------------------------------------------
struct frame
{
    picture   content;

    v2_vector  offset;
};

//================================================================================================================================
// V2_VECTOR
//================================================================================================================================

bool      v2_vector_ctor(v2_vector *const vec, const int x, const int y);
//--------------------------------------------------------------------------------------------------------------------------------
v2_vector v2_vector_max(v2_vector a, v2_vector b);
v2_vector v2_vector_min(v2_vector a, v2_vector b);
//--------------------------------------------------------------------------------------------------------------------------------
v2_vector v2_vector_add(const v2_vector a, const v2_vector b);
v2_vector v2_vector_sub(const v2_vector a, const v2_vector b);

//================================================================================================================================
// PICTURE
//================================================================================================================================

bool picture_ctor         (picture *const paint, unsigned *const pixels, const v2_vector size);
bool picture_ctor         (picture *const paint,                         const v2_vector size);
bool picture_init_by_bmp  (picture *const paint, const char *bmp32_filename);
bool picture_init_by_frame(picture *const paint, const frame *const segment, const v2_vector size, const unsigned set_space_color);
void picture_dtor         (picture *const paint);
//--------------------------------------------------------------------------------------------------------------------------------
void picture_alpha_blending(const picture *const front,
                            const picture *const back ,
                                  picture *const blend);

//================================================================================================================================
// FRAME
//================================================================================================================================

bool frame_ctor       (frame *const segment, picture *const content    , const v2_vector offset = {0, 0});
bool frame_init_by_bmp(frame *const segment, const char *bmp32_filename, const v2_vector offset = {0, 0});
bool frame_set_offset (frame *const segment,                             const v2_vector offset);
void frame_dtor       (frame *const segment);

#endif //PICTURE_H