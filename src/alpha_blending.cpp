#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

#include "../lib/logs/log.h"
#include "../lib/algorithm/algorithm.h"

//================================================================================================================================

#include "alpha_blending.h"

//--------------------------------------------------------------------------------------------------------------------------------
// alpha_blending
//--------------------------------------------------------------------------------------------------------------------------------

static void no_simd_reculc(const unsigned *const front,
                           const unsigned *const  back,
                                 unsigned *const blend, const int cur_x,
                                                        const int width);

//--------------------------------------------------------------------------------------------------------------------------------
// RGBA
//--------------------------------------------------------------------------------------------------------------------------------

struct RGBA
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
};

static RGBA     rgba_ctor (const unsigned val);
static unsigned rgba_parse(RGBA pixel);

//================================================================================================================================

//--------------------------------------------------------------------------------------------------------------------------------
// ALPHA_BLENDING
//--------------------------------------------------------------------------------------------------------------------------------

void alpha_blending_intrin(const int *const front,
                           const int *const back ,
                                 int *const blend, const int width, const int height)
{
    log_assert(front != nullptr);
    log_assert(back  != nullptr);
    log_assert(blend != nullptr);

    log_assert(width  > 0);
    log_assert(height > 0);

    __m128i mask_load_store    = _mm_set1_epi32(1 << 31);

    __m128i mask_shuf_16to8_hi = _mm_set_epi64x   ((long long) 0x0F0D'0B09'0705'0301, (long long) 0x8080'8080'8080'8080);
    __m128i mask_shuf_16to8_lo = _mm_set_epi64x   ((long long) 0x8080'8080'8080'8080, (long long) 0x0F0D'0B09'0705'0301);

    __m256i mask_shuf_alpha    = _mm256_set_epi64x((long long) 0x800E'800E'800E'800E, (long long) 0x8006'8006'8006'8006,
                                                   (long long) 0x800E'800E'800E'800E, (long long) 0x8006'8006'8006'8006);

    for (int y = 0; y < height; y += 1) { const int base = y * width;
    for (int x = 0; x <  width; x += 4)
        {
            if (x + 4 > width)
            {
                no_simd_reculc((const unsigned *) front + base,
                               (const unsigned *) back  + base,
                               (      unsigned *) blend + base, x, width);
                break;
            }

            const int ind = base + x;

    //    15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
    //   =======================================================
    //  | a3 b3 g3 r3 | a2 b2 g2 r2 | a1 b1 g1 r1 | a0 b0 g0 r0 | **

            __m128i bk = _mm_maskload_epi32(back  + ind, mask_load_store);
            __m128i fr = _mm_maskload_epi32(front + ind, mask_load_store);

    //    31 30 29 28   27 26 25 24   23 22 21 20   19 18 17 16   15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
    //   ===============================================================================================================
    //                                                          | a3 b3 g3 r3 | a2 b2 g2 r2 | a1 b1 g1 r1 | a0 b0 g0 r0 | **
    //  | -- a3 -- b3 | -- g3 -- r3 | -- a2 -- b2 | -- g2 -- r2 | -- a1 -- b1 | -- g1 -- r1 | -- a0 -- b0 | -- b0 -- r0 | **_all

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
    //  |  a3*a3  a3*b3 |  a3*g3  a3*r3 | a2*a2  a2*b2 |  a2*g2  a2*r2 | fr_hi
    //  | ~a3*a3 ~a3*b3 | ~a3*g3 ~a3*r3 |~a2*a2 ~a2*b2 | ~a2*g2 ~a2*r2 | bk_hi

            bk_lo = _mm_mullo_epi16(bk_lo, _mm256_extracti128_si256(bk_alpha, 0));
            bk_hi = _mm_mullo_epi16(bk_hi, _mm256_extracti128_si256(bk_alpha, 1));

            fr_lo = _mm_mullo_epi16(fr_lo, _mm256_extracti128_si256(fr_alpha, 0));
            fr_hi = _mm_mullo_epi16(fr_hi, _mm256_extracti128_si256(fr_alpha, 1));

    //    15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
    //   =======================================================
    //  | A3 B3 G3 R3 | A2 B2 G2 R2 | 00 00 00 00 | 00 00 00 00 | fr_hi
    //  | 00 00 00 00 | 00 00 00 00 | A1 B1 G1 R1 | A0 B0 G0 R0 | fr_lo
    //                                              ^^--------------------- Xi = (ai*xi) >> 8

            bk_lo = _mm_shuffle_epi8(bk_lo, mask_shuf_16to8_lo);
            bk_hi = _mm_shuffle_epi8(bk_hi, mask_shuf_16to8_hi);

            fr_lo = _mm_shuffle_epi8(fr_lo, mask_shuf_16to8_lo);
            fr_hi = _mm_shuffle_epi8(fr_hi, mask_shuf_16to8_hi);

            __m128i res = _mm_add_epi8(_mm_blend_epi32(bk_lo, bk_hi, 12), _mm_blend_epi32(fr_lo, fr_hi, 12));

    //   =======================================================

            _mm_maskstore_epi32(blend + ind, mask_load_store, res);
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------------

void alpha_blending_simple(const int *const front,
                           const int *const back ,
                                 int *const blend, const int width, const int height)
{
    log_assert(front != nullptr);
    log_assert(back  != nullptr);
    log_assert(blend != nullptr);

    log_assert(width  > 0);
    log_assert(height > 0);

    for (int y = 0; y < height; ++y)
    { 
        const int base = y * width;

        no_simd_reculc((const unsigned *) front + base,
                       (const unsigned *) back  + base,
                       (      unsigned *) blend + base, 0, width);
    }
}

//--------------------------------------------------------------------------------------------------------------------------------

#define $fr_red     (unsigned) (pixel_front.red)
#define $fr_green   (unsigned) (pixel_front.green)
#define $fr_blue    (unsigned) (pixel_front.blue)
#define $fr_alpha   (unsigned) (pixel_front.alpha)

#define $bk_red     (unsigned) (pixel_back.red)
#define $bk_green   (unsigned) (pixel_back.green)
#define $bk_blue    (unsigned) (pixel_back.blue)
#define $bk_alpha   (unsigned) (pixel_back.alpha)

static void no_simd_reculc(const unsigned *const front,
                           const unsigned *const  back,
                                 unsigned *const blend, const int cur_x,
                                                        const int width)
{
    log_assert(front != nullptr);
    log_assert(back  != nullptr);
    log_assert(blend != nullptr);

    log_assert(cur_x >= 0);
    log_assert(width >= 0);
    log_assert(width > cur_x);

    for (int x = cur_x; x < width; ++x)
    {
        RGBA pixel_front = rgba_ctor(front[x]);
        RGBA pixel_back  = rgba_ctor(back [x]);

        RGBA pixel_blend = {(unsigned char) (($fr_red   * $fr_alpha + $bk_red   * (0xFF - $fr_alpha)) >> 8),
                            (unsigned char) (($fr_green * $fr_alpha + $bk_green * (0xFF - $fr_alpha)) >> 8),
                            (unsigned char) (($fr_blue  * $fr_alpha + $bk_blue  * (0xFF - $fr_alpha)) >> 8),
                            (unsigned char) (($fr_alpha * $fr_alpha + $bk_alpha * (0xFF - $fr_alpha)) >> 8)};

        blend[x] = rgba_parse(pixel_blend);
    }
}

//--------------------------------------------------------------------------------------------------------------------------------
// RGBA
//--------------------------------------------------------------------------------------------------------------------------------

#define $red    (pixel.red)
#define $green  (pixel.green)
#define $blue   (pixel.blue)
#define $alpha  (pixel.alpha)

//--------------------------------------------------------------------------------------------------------------------------------

static RGBA rgba_ctor(const unsigned val)
{
    RGBA pixel = {};

    $red   = (unsigned char) ((val >>  0U) & 0xFF);
    $green = (unsigned char) ((val >>  8U) & 0xFF);
    $blue  = (unsigned char) ((val >> 16U) & 0xFF);
    $alpha = (unsigned char) ((val >> 24U) & 0xFF);

    return pixel;
}

static unsigned rgba_parse(RGBA pixel)
{
    unsigned red   = (unsigned) $red;
    unsigned green = (unsigned) $green;
    unsigned blue  = (unsigned) $blue;
    unsigned alpha = (unsigned) $alpha;

    return ((alpha << 24U) | (blue << 16U)) | ((green << 8U) | red);
}