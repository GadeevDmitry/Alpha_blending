#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SFML/Graphics.hpp>

#include "../lib/logs/log.h"
#include "../lib/algorithm/algorithm.h"

//================================================================================================================================

#include "picture.h"

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

//--------------------------------------------------------------------------------------------------------------------------------
// DSL
//--------------------------------------------------------------------------------------------------------------------------------

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
    $pixels_size = $size_x * $size_y;

    return true;
}

//--------------------------------------------------------------------------------------------------------------------------------

bool picture_ctor(picture *const paint, const char *bmp32_filename)
{
    log_verify(paint          != nullptr, false);
    log_verify(bmp32_filename != nullptr, false);

    $pixels      = parse_bmp32(bmp32_filename, &$size);
    $pixels_size = $size_x * $size_y;

    return $pixels != nullptr;
}

//--------------------------------------------------------------------------------------------------------------------------------

void picture_dtor(picture *const paint)
{
    if (paint != nullptr) log_free($pixels);
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

    unsigned  data_size = (unsigned  ) biWidth * biHeight;
    unsigned *data      = (unsigned *) log_calloc(data_size, sizeof(unsigned));

    if (data == nullptr)
    {
        log_error("Can't allocate memory for array of pixels\n");

        buffer_dtor(&bmp_file_content);

        return nullptr;
    }

    v2_vector_ctor(size, biWidth, biHeight);
    memcpy(data, bmp_file_content.buff_beg + bfOffBits, data_size);

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

    const int   size = file_content->buff_size;
    const char *data = file_content->buff_beg ;

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

    const int   size = bmp_file_content->buff_size;
    const char *data = bmp_file_content->buff_beg ;

    const int biWidth   = *(int *) (data + 18);
    const int biHeight  = *(int *) (data + 22);
    const int bfOffBits = *(int *) (data + 10);

    if (biWidth   <= 0) { log_error("biWidth   field of .bmp file \"%s\" is invalid\n", filename); return false; }
    if (biHeight  <= 0) { log_error("biHeight  field of .bmp file \"%s\" is invalid\n", filename); return false; }
    if (bfOffBits < 54) { log_error("bfOffBits field of .bmp file \"%s\" is invalid\n", filename); return false; }

    *_biWidth   = biWidth;
    *_biHeight  = biHeight;
    *_bfOffBits = bfOffBits;

    return true;
}

//--------------------------------------------------------------------------------------------------------------------------------
// draw
//--------------------------------------------------------------------------------------------------------------------------------

void picture_draw(picture *const paint, sf::RenderWindow *const wnd)
{
    log_verify(paint != nullptr, (void) 0);
    log_verify(wnd   != nullptr, (void) 0);

    sf::Image   img; img.create((unsigned) $size_x, (unsigned) $size_y, (sf::Uint8 *) $pixels);
    sf::Texture tex; tex.loadFromImage(img);
    sf::Sprite  spr(tex);

    wnd->draw(spr);
}

//================================================================================================================================
// FRAME
//================================================================================================================================

//--------------------------------------------------------------------------------------------------------------------------------
// DSL
//--------------------------------------------------------------------------------------------------------------------------------

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

    $content = content;
    $offset  =  offset;

    return true;
}

//--------------------------------------------------------------------------------------------------------------------------------

bool frame_ctor(frame *const segment, const char *bmp32_filename, const v2_vector offset /* = {0, 0} */)
{
    log_verify(segment        != nullptr, false);
    log_verify(bmp32_filename != nullptr, false);

    $offset = offset;

    return picture_ctor(&$content, bmp32_filename);
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
    if (segment != nullptr) picture_dtor($content);
}

//--------------------------------------------------------------------------------------------------------------------------------
// draw
//--------------------------------------------------------------------------------------------------------------------------------

void frame_draw(frame *const segment, sf::RenderWindow *const wnd)
{
    log_verify(segment != nullptr, (void) 0);
    log_verify(wnd     != nullptr, (void) 0);

    picture_draw($content, wnd);
}
