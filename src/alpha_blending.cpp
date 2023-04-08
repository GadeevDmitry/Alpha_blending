#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>

#include <SFML/Graphics.hpp>

#include "../lib/logs/log.h"
#include "alpha_blending.h"

//================================================================================================================================
// STATIC GLOBAL DECLARATION
//================================================================================================================================

static void     reculc_no_simd (alpha_blending *const alpha, const int y, int x);
static unsigned rgba_color_ctor(const unsigned r,
                                const unsigned g,
                                const unsigned b,
                                const unsigned a);
//--------------------------------------------------------------------------------------------------------------------------------

static inline int int_min(const int a, const int b);

//================================================================================================================================
// BODY
//================================================================================================================================

//--------------------------------------------------------------------------------------------------------------------------------
// DSL
//--------------------------------------------------------------------------------------------------------------------------------

#define $front      alpha->front
#define $back       alpha->back
#define $blend      alpha->blend

#define $size_total alpha->size_total
#define $size_x     alpha->size_x
#define $size_y     alpha->size_y

//--------------------------------------------------------------------------------------------------------------------------------
// ctor, dtor
//--------------------------------------------------------------------------------------------------------------------------------

bool alpha_blending_ctor(alpha_blending *const alpha, unsigned *const front,
                                                      unsigned *const back , const int size_x,
                                                                             const int size_y)
{
    log_verify(alpha != nullptr, false);

    log_verify(front != nullptr, false);
    log_verify(back  != nullptr, false);

    log_verify(size_x > 0, false);
    log_verify(size_y > 0, false);

    const int size_total = size_x * size_y;

    $size_x     = size_x;
    $size_y     = size_y;
    $size_total = size_total;

    $front = front;
    $back  = back;
    $blend = (unsigned *) log_calloc(size_total, sizeof(unsigned));

    log_verify($blend != nullptr, false);

    return true;
}

//--------------------------------------------------------------------------------------------------------------------------------

void alpha_blending_dtor(alpha_blending *const alpha)
{
    if (alpha == nullptr) return;

    log_free($front);
    log_free($back );
    log_free($blend);
}

//--------------------------------------------------------------------------------------------------------------------------------
// reculc
//--------------------------------------------------------------------------------------------------------------------------------

void alpha_blending_reculc(alpha_blending *const alpha)
{
    log_verify(alpha != nullptr, (void) 0);

    const int size_y = $size_y;
    const int size_x = $size_x;

    int *const int_back  = (int *) $back;
    int *const int_front = (int *) $front;
    int *const int_blend = (int *) $blend;

    __m128i mask_load_store    = _mm_set1_epi32(1 << 31);

    __m128i mask_shuf_16to8_hi = _mm_set_epi64x   (0x0F0D'0B09'0705'0301, 0x8080'8080'8080'8080);
    __m128i mask_shuf_16to8_lo = _mm_set_epi64x   (0x8080'8080'8080'8080, 0x0F0D'0B09'0705'0301);

    __m256i mask_shuf_alpha    = _mm256_set_epi64x(0x8008'8008'8008'8008, 0x8000'8000'8000'8000,
                                                   0x8008'8008'8008'8008, 0x8000'8000'8000'8000);

    for (int y = 0; y < size_y; y += 1) { const int offset = y * size_x;
    for (int x = 0; x < size_x; x += 4)
        {
            if (x + 4 > size_x) { reculc_no_simd(alpha, y, x); break; }

    //    15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
    //   =======================================================
    //  | r3 g3 b3 a3 | r2 g2 b2 a2 | r1 g1 b1 a1 | r0 g0 b0 a0 | **

            __m128i bk = _mm_maskload_epi32(int_back  + offset + x, mask_load_store);
            __m128i fr = _mm_maskload_epi32(int_front + offset + x, mask_load_store);

    //    31 30 29 28   27 26 25 24   23 22 21 20   19 18 17 16   15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
    //   ===============================================================================================================
    //                                                          | r3 g3 b3 a3 | r2 g2 b2 a2 | r1 g1 b1 a1 | r0 g0 b0 a0 | **
    //  | -- r3 -- g3 | -- b3 -- a3 | -- r2 -- g2 | -- b2 -- a2 | -- r1 -- g1 | -- b1 -- a1 | -- r0 -- g0 | -- b0 -- a0 | **_all

            __m256i bk_all = _mm256_cvtepu8_epi16(bk);
            __m128i bk_lo  = _mm256_extracti128_si256(bk_all, 0);
            __m128i bk_hi  = _mm256_extracti128_si256(bk_all, 1);

            __m256i fr_all = _mm256_cvtepu8_epi16(fr);
            __m128i fr_lo  = _mm256_extracti128_si256(fr_all, 0);
            __m128i fr_hi  = _mm256_extracti128_si256(fr_all, 1);

    //    31 30 29 28   27 26 25 24   23 22 21 20   19 18 17 16   15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
    //   ===============================================================================================================
    //  | -- a3 -- a3 | -- a3 -- a3 | -- a2 -- a2 | -- a2 -- a2 | -- a1 -- a1 | -- a1 -- a1 | -- a0 -- a0 | -- a0 -- a0 | fr_alpha
    //  | - ~a3 - ~a3 | - ~a3 - ~a3 | - ~a2 - ~a2 | - ~a2 - ~a2 | - ~a1 - ~a1 | - ~a1 - ~a1 | - ~a0 - ~a0 | - ~a0 - ~a0 | bk_alpha

            __m256i fr_alpha = _mm256_shuffle_epi8(fr_all, mask_shuf_alpha);
            __m256i bk_alpha = _mm256_sub_epi8(_mm256_set1_epi64x(0x00FF00FF00FF00FF), fr_alpha);

    //     15 14  13 12    11 10   9  8    7  6   5  4     3  2   1  0
    //   ==============================================================
    //  |  a3*r3  a3*g3 |  a3*b3  a3*a3 | a2*r2  a2*g2 |  a2*b2  a2*a2 | fr_hi
    //  | ~a3*r3 ~a3*g3 | ~a3*b3 ~a3*a3 |~a2*r2 ~a2*g2 | ~a2*b2 ~a2*a2 | bk_hi

            bk_lo = _mm_mulhi_epu16(bk_lo, _mm256_extracti128_si256(bk_alpha, 0));
            bk_hi = _mm_mulhi_epu16(bk_hi, _mm256_extracti128_si256(bk_alpha, 1));

            fr_lo = _mm_mulhi_epu16(fr_lo, _mm256_extracti128_si256(fr_alpha, 0));
            fr_hi = _mm_mulhi_epu16(fr_hi, _mm256_extracti128_si256(fr_alpha, 1));

    //    15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
    //   =======================================================
    //  | R3 G3 B3 A3 | R2 G2 B2 A2 | 00 00 00 00 | 00 00 00 00 | fr_hi
    //  | 00 00 00 00 | 00 00 00 00 | R1 G1 B1 A1 | R0 G0 B0 A0 | fr_lo
    //                                              ^^--------------------- Xi = (ai*xi) >> 8

            bk_lo = _mm_shuffle_epi8(bk_lo, mask_shuf_16to8_lo);
            bk_hi = _mm_shuffle_epi8(bk_hi, mask_shuf_16to8_hi);

            fr_lo = _mm_shuffle_epi8(fr_lo, mask_shuf_16to8_lo);
            fr_hi = _mm_shuffle_epi8(fr_hi, mask_shuf_16to8_hi);

            __m128i res = _mm_add_epi8(_mm_blend_epi32(bk_hi, bk_lo, 12), _mm_blend_epi32(fr_hi, fr_lo, 12));

    //   =======================================================

            _mm_maskstore_epi32(int_blend + offset + x, mask_load_store, res);
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------------

static void reculc_no_simd(alpha_blending *const alpha, const int y, int x)
{
    const int offset = y * $size_x;

    for (; x < $size_x; ++x)
    {
        const unsigned fr_red   = $front[offset + x] & (255U << 24);
        const unsigned bk_red   = $back [offset + x] & (255U << 24);

        const unsigned fr_green = $front[offset + x] & (255U << 16);
        const unsigned bk_green = $back [offset + x] & (255U << 16);

        const unsigned fr_blue  = $front[offset + x] & (255U <<  8);
        const unsigned bk_blue  = $back [offset + x] & (255U <<  8);

        const unsigned fr_alpha = $front[offset + x] &  255U;
        const unsigned bk_alpha = $back [offset + x] &  255U;

        $blend[offset + x] = rgba_color_ctor((fr_red  *fr_alpha + bk_red  *(1 - fr_alpha)) << 8,
                                             (fr_green*fr_alpha + bk_green*(1 - fr_alpha)) << 8,
                                             (fr_blue *fr_alpha + bk_blue *(1 - fr_alpha)) << 8,
                                             (fr_alpha*fr_alpha + bk_alpha*(1 - fr_alpha)) << 8);
    }
}

//--------------------------------------------------------------------------------------------------------------------------------

static unsigned rgba_color_ctor(const unsigned r,
                                const unsigned g,
                                const unsigned b,
                                const unsigned a)
{
    return ((r << 24) | (g << 16)) | ((b << 8) | a);
}

//--------------------------------------------------------------------------------------------------------------------------------
// draw
//--------------------------------------------------------------------------------------------------------------------------------

void alpha_blending_draw(alpha_blending *const alpha, sf::RenderWindow *const wnd)
{
    log_verify(alpha != nullptr, (void) 0);
    log_verify(wnd   != nullptr, (void) 0);

    sf::Image   img; img.create((unsigned) $size_x, (unsigned) $size_y, (sf::Uint8 *) $blend);
    sf::Texture tex; tex.loadFromImage(img);
    sf::Sprite  spr (tex);

    wnd->clear();
    wnd->draw (spr);
}

//--------------------------------------------------------------------------------------------------------------------------------
// other
//--------------------------------------------------------------------------------------------------------------------------------

static inline int int_min(const int a, const int b)
{
    return (a < b) ? a : b;
}
