#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>

#include "../lib/logs/log.h"
#include "alpha_blending.h"

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

bool alpha_blending_ctor(alpha_blending *const alpha, const char *const file_bmp_front,
                                                      const char *const file_bmp_back)
{
    log_verify(alpha          != nullptr, false);
    log_verify(file_bmp_front != nullptr, false);
    log_verify(file_bmp_back  != nullptr, false);

    int size_x_front = 0,
        size_y_front = 0;

    unsigned *front = parse_file_bmp(file_bmp_front, &size_x_front, &size_y_front);
    if (      front == nullptr) return false;

    int size_x_back = 0,
        size_y_back = 0;

    unsigned *back = parse_file_bmp(file_bmp_back, &size_x_back, &size_y_back);
    if (      back == nullptr) return false;

    const int size_x     = int_min(size_x_back, size_x_front);
    const int size_y     = int_min(size_y_back, size_y_front);
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

static unsigned *parse_file_bmp(const char *const file_bmp, int *const size_x,
                                                            int *const size_y)
{
    log_assert(file_bmp != nullptr);

    log_assert(size_x != nullptr);
    log_assert(size_y != nullptr);

    return nullptr;
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
            if (x + 4 > size_x) { reculc_no_simd(alpha, y, x); continue; }

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

}

//--------------------------------------------------------------------------------------------------------------------------------
// other
//--------------------------------------------------------------------------------------------------------------------------------

static inline int int_min(const int a, const int b)
{
    return (a < b) ? a : b;
}
