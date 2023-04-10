#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SFML/Graphics.hpp>

#include "../lib/logs/log.h"
#include "../lib/algorithm/algorithm.h"

//================================================================================================================================

#include "picture.h"
#include "alpha_blending.h"

//--------------------------------------------------------------------------------------------------------------------------------
// init by frame
//--------------------------------------------------------------------------------------------------------------------------------

static inline void set_pixels   (      picture *const paint,     const unsigned set_color);
static inline void frame_implant(      picture *const paint    , const picture *const segment    ,
                                 const v2_vector      paint_beg, const v2_vector      segment_beg, const v2_vector cover_size);
//--------------------------------------------------------------------------------------------------------------------------------
// parse_bmp32
//--------------------------------------------------------------------------------------------------------------------------------

static unsigned  *parse_bmp32(const char *filename, v2_vector *const size);
//--------------------------------------------------------------------------------------------------------------------------------
static inline bool bmp32_load           (const char *filename,       buffer *const     file_content);
static inline bool bmp32_check_signature(const char *filename,       buffer *const     file_content);
static inline bool bmp32_extract_data   (const char *filename, const buffer *const bmp_file_content, int *const _biWidth,
                                                                                                     int *const _biHeight,
                                                                                                     int *const _bfOffBits);
static inline void bmp32_reculc_pixels  (unsigned *const pixels, const unsigned pixels_size);

//================================================================================================================================
// V2_VECTOR
//================================================================================================================================

//--------------------------------------------------------------------------------------------------------------------------------
// ctor
//--------------------------------------------------------------------------------------------------------------------------------

bool v2_vector_ctor(v2_vector *const vec, const int x, const int y)
{
    log_verify(vec != nullptr, false);

    vec->x = x;
    vec->y = y;

    return true;
}

//--------------------------------------------------------------------------------------------------------------------------------
// min, max
//--------------------------------------------------------------------------------------------------------------------------------

v2_vector v2_vector_max(v2_vector a, v2_vector b)
{
    v2_vector res = {};

    res.x = (a.x > b.x) ? a.x : b.x;
    res.y = (a.y > b.y) ? a.y : b.y;

    return res;
}

v2_vector v2_vector_min(v2_vector a, v2_vector b)
{
    v2_vector res = {};

    res.x = (a.x < b.x) ? a.x : b.x;
    res.y = (a.y < b.y) ? a.y : b.y;

    return res;
}

//--------------------------------------------------------------------------------------------------------------------------------
// operator
//--------------------------------------------------------------------------------------------------------------------------------

v2_vector v2_vector_add(const v2_vector a, const v2_vector b)
{
    return {a.x + b.x, a.y + b.y};
}

v2_vector v2_vector_sub(const v2_vector a, const v2_vector b)
{
    return {a.x - b.x, a.y - b.y};
}

//================================================================================================================================
// PICTURE
//================================================================================================================================

#define $pixels         (paint->pixels)
#define $pixels_size    (paint->pixels_size)

#define $size           (paint->size)
#define $size_x         (paint->size.x)
#define $size_y         (paint->size.y)

//--------------------------------------------------------------------------------------------------------------------------------
// ctor, dtor
//--------------------------------------------------------------------------------------------------------------------------------

bool picture_ctor(picture *const paint, unsigned *const pixels, const v2_vector size)
{
    log_verify(paint  != nullptr, false);
    log_verify(pixels != nullptr, false);

    log_verify(size.x > 0, false);
    log_verify(size.y > 0, false);

    $size        = size;
    $pixels      = pixels;
    $pixels_size = (unsigned) (size.x * size.y);

    return true;
}

//--------------------------------------------------------------------------------------------------------------------------------

bool picture_ctor(picture *const paint, const v2_vector size)
{
    log_verify(paint != nullptr, false);

    log_verify(size.x > 0, false);
    log_verify(size.y > 0, false);

    $size        = size;
    $pixels_size = (unsigned) (size.x * size.y);
    $pixels      = (unsigned *) log_calloc($pixels_size, sizeof(unsigned));

    log_verify($pixels != nullptr, false);

    return true;
}

//--------------------------------------------------------------------------------------------------------------------------------

bool picture_init_by_bmp(picture *const paint, const char *bmp32_filename)
{
    log_verify(paint          != nullptr, false);
    log_verify(bmp32_filename != nullptr, false);

    $pixels      = parse_bmp32(bmp32_filename, &$size);
    $pixels_size = (unsigned) ($size_x * $size_y);

    return $pixels != nullptr;
}

//--------------------------------------------------------------------------------------------------------------------------------

bool picture_init_by_frame(picture *const paint, const frame *const segment, const v2_vector size, const unsigned set_space_color)
{
    log_verify(paint   != nullptr, false);
    log_verify(segment != nullptr, false);

    log_verify(size.x > 0, false);
    log_verify(size.y > 0, false);

    const unsigned pixels_size = (unsigned) (size.x * size.y);

    $size        = size;
    $pixels_size = pixels_size;
    $pixels      = (unsigned *) log_calloc($pixels_size, sizeof(unsigned));

    log_verify($pixels != nullptr, false);

    set_pixels(paint, set_space_color);

    v2_vector   paint_beg = v2_vector_max({0, 0}   , segment->offset);
    v2_vector segment_beg = v2_vector_sub(paint_beg, segment->offset);

    v2_vector cover_size  = v2_vector_min(v2_vector_sub(size, paint_beg), segment->content.size);

    frame_implant(paint, &(segment->content), paint_beg, segment_beg, cover_size);

    return true;
}

//--------------------------------------------------------------------------------------------------------------------------------

static inline void set_pixels(picture *const paint, const unsigned set_color)
{
    log_assert(paint != nullptr);

    const unsigned pixels_size = $pixels_size;

    for (unsigned i = 0; i < pixels_size; ++i) $pixels[i] = set_color;
}

//--------------------------------------------------------------------------------------------------------------------------------

#define $paint_x0       paint_beg.x
#define $paint_y0       paint_beg.y

#define $segment_x0     segment_beg.x
#define $segment_y0     segment_beg.y

#define $paint_width    paint  ->size.x
#define $segment_width  segment->size.x

static inline void frame_implant(      picture *const paint    , const picture *const segment    ,
                                 const v2_vector      paint_beg, const v2_vector      segment_beg, const v2_vector cover_size)
{
    log_assert(paint   != nullptr);
    log_assert(segment != nullptr);

    for (int y = 0; y < cover_size.y; ++y) { const int   paint_base = (  $paint_y0 + y) *   $paint_width +   $paint_x0;
                                             const int segment_base = ($segment_y0 + y) * $segment_width + $segment_x0;
    for (int x = 0; x < cover_size.x; ++x)
        {
            paint->pixels[paint_base + x] = segment->pixels[segment_base + x];
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------------

void picture_dtor(picture *const paint)
{
    if (paint != nullptr) log_free($pixels);
}

//--------------------------------------------------------------------------------------------------------------------------------
// alpha_blending
//--------------------------------------------------------------------------------------------------------------------------------

void picture_alpha_blending(const picture *const front,
                            const picture *const back ,
                                  picture *const blend)
{
    log_verify(front != nullptr, (void) 0);
    log_verify(back  != nullptr, (void) 0);
    log_verify(blend != nullptr, (void) 0);

    const int width  = front->size.x;
    const int height = front->size.y;

    alpha_blending_intrin((int *) front->pixels, (int *) back->pixels, (int *) blend->pixels, width, height);
}

//--------------------------------------------------------------------------------------------------------------------------------
// parse_bmp32
//--------------------------------------------------------------------------------------------------------------------------------

static unsigned *parse_bmp32(const char *filename, v2_vector *const size)
{
    log_verify(filename != nullptr, nullptr);
    log_verify(size     != nullptr, nullptr);

    buffer bmp_file_content = {};

    if (!bmp32_load           (filename, &bmp_file_content)) {                                 return nullptr; }
    if (!bmp32_check_signature(filename, &bmp_file_content)) { buffer_dtor(&bmp_file_content); return nullptr; }

    int biWidth   = 0;
    int biHeight  = 0;
    int bfOffBits = 0;

    if (!bmp32_extract_data   (filename, &bmp_file_content, &biWidth,
                                                            &biHeight,
                                                            &bfOffBits)) { buffer_dtor(&bmp_file_content); return nullptr; }

    unsigned  data_size = (unsigned  ) (biWidth * biHeight);
    unsigned *data      = (unsigned *) log_calloc(data_size, sizeof(unsigned));

    if (data == nullptr)
    {
        log_error("Can't allocate memory for array of pixels\n");

        buffer_dtor(&bmp_file_content);

        return nullptr;
    }

    v2_vector_ctor(size, biWidth, biHeight);

    memcpy(data, bmp_file_content.buff_beg + bfOffBits, sizeof(unsigned) * data_size);
    bmp32_reculc_pixels(data, data_size);

    buffer_dtor(&bmp_file_content);

    return data;
}

//--------------------------------------------------------------------------------------------------------------------------------

static inline bool bmp32_load(const char *filename, buffer *const file_content)
{
    log_assert(filename     != nullptr);
    log_assert(file_content != nullptr);

    if (!buffer_ctor_file(file_content, filename))
    {
        log_error("Can't open .bmp file \"%s\" to parse\n", filename);
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------------------------------------

static inline bool bmp32_check_signature(const char *filename, buffer *const file_content)
{
    log_assert(filename     != nullptr);
    log_assert(file_content != nullptr);

    const int   size = (int) file_content->buff_size;
    const char *data =       file_content->buff_beg ;

    if (size < 54 || data[0] != 'B' || data[1] != 'M')
    {
        log_error("\"%s\" is not .bmp format\n", filename);
        return false;
    }

    const short biBitCount  = *(const short *) (data + 28);

    if (biBitCount != 32)
    {
        log_error("\"%s\" is not .bmp 32bit format\n", filename);
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------------------------------------

static inline bool bmp32_extract_data(const char *filename, const buffer *const bmp_file_content, int *const _biWidth,
                                                                                                  int *const _biHeight,
                                                                                                  int *const _bfOffBits)
{
    log_assert(filename         != nullptr);
    log_assert(bmp_file_content != nullptr);

    const char *data = bmp_file_content->buff_beg;

    int biWidth   = 0;
    int biHeight  = 0;
    int bfOffBits = 0;

    memcpy(&biWidth  , data + 18, sizeof(int));
    memcpy(&biHeight , data + 22, sizeof(int));
    memcpy(&bfOffBits, data + 10, sizeof(int));

    biWidth   = abs(biWidth);
    biHeight  = abs(biHeight);
    bfOffBits = abs(bfOffBits);

    if (bfOffBits < 54) { log_error("bfOffBits field of .bmp file \"%s\" is invalid\n", filename); return false; }

    *_biWidth   = biWidth;
    *_biHeight  = biHeight;
    *_bfOffBits = bfOffBits;

    return true;
}

//--------------------------------------------------------------------------------------------------------------------------------

static inline void bmp32_reculc_pixels(unsigned *const pixels, const unsigned pixels_size)
{
    log_assert(pixels != nullptr);

    for (unsigned i = 0; i < pixels_size; ++i)
    {
        unsigned red   = (pixels[i] >> 16U) & 0xFF;
        unsigned green = (pixels[i] >>  8U) & 0xFF;
        unsigned blue  = (pixels[i] >>  0U) & 0xFF;
        unsigned alpha = (pixels[i] >> 24U) & 0xFF;

        pixels[i] = ((alpha << 24U) | (blue << 16U)) | ((green << 8U) | (red << 0U));
    }
}

//================================================================================================================================
// FRAME
//================================================================================================================================

#define $content        (segment->content)

#define $offset         (segment->offset)
#define $offset_x       (segment->offset.x)
#define $offset_y       (segment->offset.y)

//--------------------------------------------------------------------------------------------------------------------------------
// ctor, dtor
//--------------------------------------------------------------------------------------------------------------------------------

bool frame_ctor(frame *const segment, picture *const content, const v2_vector offset /* = {0, 0} */)
{
    log_verify(segment != nullptr, false);
    log_verify(content != nullptr, false);

    $content = *content;
    $offset  =   offset;

    return true;
}

//--------------------------------------------------------------------------------------------------------------------------------

bool frame_init_by_bmp(frame *const segment, const char *bmp32_filename, const v2_vector offset /* = {0, 0} */)
{
    log_verify(segment        != nullptr, false);
    log_verify(bmp32_filename != nullptr, false);

    $offset = offset;

    return picture_init_by_bmp(&$content, bmp32_filename);
}

//--------------------------------------------------------------------------------------------------------------------------------

bool frame_set_offset(frame *const segment, const v2_vector offset)
{
    log_verify(segment != nullptr, false);

    $offset = offset;

    return true;
}

//--------------------------------------------------------------------------------------------------------------------------------

void frame_dtor(frame *const segment)
{
    if (segment != nullptr) picture_dtor(&$content);
}
