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

    __m128i maskload_epi32 = _mm_set1_epi32(1 << 31);

    for (int y = 0; y < size_y; ++y) { const int offset = y * size_x;
    for (int x = 0; x < size_x; ++x)
        {
            if (x + 8 > size_x) { reculc_no_simd(alpha, y, x); continue; }

        //    0  1  2  3    4  5  6  7    8  9 10 11   12 13 14 15
        //  =======================================================
        // | r0 g0 b0 a0 | r1 g1 b1 a1 | r2 g2 b2 a2 | r3 g3 b3 a3 |    8_low
        // | r4 g4 b4 a4 | r5 g5 b5 a5 | r6 g6 b6 a6 | r7 g7 b7 a7 |    8_high

            __m128i bk8_low  = _mm_maskload_epi32(int_back  + offset + x, maskload_epi32);
            __m128i fr8_low  = _mm_maskload_epi32(int_front + offset + x, maskload_epi32);

            __m128i bk8_high = _mm_maskload_epi32(int_back  + offset + x + 4, maskload_epi32);
            __m128i fr8_high = _mm_maskload_epi32(int_front + offset + x + 4, maskload_epi32);

        //    0  1  2  3    4  5  6  7    8  9 10 11   12 13 14 15   16 17 18 19   20 21 22 23   24 25 26 27   28 29 30 31
        //  ===============================================================================================================
        // | r0 g0 b0 a0 | r1 g1 b1 a1 | r2 g2 b2 a2 | r3 g3 b3 a3 |                                                          8
        // | r0 -- g0 -- | b0 -- a0 -- | r1 -- g1 -- | b1 -- a1 -- | r2 -- g2 -- | b2 -- a2 -- | r3 -- g3 -- | b3 -- a3 -- |  16

            __m256i bk16_low  = _mm256_cvtepu8_epi16(bk8_low);
            __m256i fr16_low  = _mm256_cvtepu8_epi16(fr8_low);

            __m256i bk16_high = _mm256_cvtepu8_epi16(bk8_high);
            __m256i fr16_high = _mm256_cvtepu8_epi16(fr8_high);
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------------

static void reculc_no_simd(alpha_blending *const alpha, const int y, const int x)
{

}

//--------------------------------------------------------------------------------------------------------------------------------
// other
//--------------------------------------------------------------------------------------------------------------------------------

static inline int int_min(const int a, const int b)
{
    return (a < b) ? a : b;
}
