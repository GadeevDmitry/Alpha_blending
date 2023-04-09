#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

#include "../lib/logs/log.h"
#include "../lib/algorithm/algorithm.h"

#include "alpha_blending.h"


static void inline alpha_blending(const picture *const front,
                                  const frame   *const back ,
                                        picture *const blend,

                                  void (*alpha_blending_reculc) (const picture *front, const v2_vector front_0,
                                                                 const picture *back , const v2_vector  back_0, const v2_vector len_blend, int *const int_blend))
{
    log_verify(front != nullptr, (void) 0);
    log_verify(back  != nullptr, (void) 0);
    log_verify(blend != nullptr, (void) 0);

    log_verify(front->size.x == blend->size.x, (void) 0);
    log_verify(front->size.y == blend->size.y, (void) 0);

    const v2_vector front_0   = v2_vector_max({0, 0} , back->offset);
    const v2_vector  back_0   = v2_vector_sub(front_0, back->offset);

    const v2_vector blend_len = v2_vector_min(v2_vector_sub(front->size, front_0), back->content.size);


    (*alpha_blending_reculc)(front          , front_0,
                            &(back->content),  back_0, blend_len, (int *) blend->pixels);
}

//--------------------------------------------------------------------------------------------------------------------------------


static void alpha_blending_simd_epi(const picture *const front, const v2_vector front_0,
                                    const picture *const  back, const v2_vector  back_0, const v2_vector blend_len, int *const int_blend)
{
    log_assert(    front != nullptr);
    log_assert(    back  != nullptr);
    log_assert(int_blend != nullptr);

    log_assert(front_0.x >= 0);
    log_assert(front_0.y >= 0);
    log_assert( back_0.x >= 0);
    log_assert( back_0.y >= 0);

    const int *const int_front = (int *) front->pixels;
    const int *const int_back  = (int *) back ->pixels;

    __m128i mask_load_store    = _mm_set1_epi32(1 << 31);

    __m128i mask_shuf_16to8_hi = _mm_set_epi64x   (0x0F0D'0B09'0705'0301, 0x8080'8080'8080'8080);
    __m128i mask_shuf_16to8_lo = _mm_set_epi64x   (0x8080'8080'8080'8080, 0x0F0D'0B09'0705'0301);

    __m256i mask_shuf_alpha    = _mm256_set_epi64x(0x8008'8008'8008'8008, 0x8000'8000'8000'8000,
                                                   0x8008'8008'8008'8008, 0x8000'8000'8000'8000);

    for (int y = 0; y < blend_len.y; y += 4) {
    for (int x = 0; x < blend_len.x; x += 4)
        {
            if (x + 4 > blend_len.x)
            {
                no_simd_reculc(int_front, fr_offset, fr_x, front_max.x,
                               int_back , bk_offset, bk_x,  back_max.x, int_blend);
                break;
            }

    //    15 14 13 12   11 10  9  8    7  6  5  4    3  2  1  0
    //   =======================================================
    //  | r3 g3 b3 a3 | r2 g2 b2 a2 | r1 g1 b1 a1 | r0 g0 b0 a0 | **

            __m128i bk = _mm_maskload_epi32(int_back  + bk_offset + fr_x, mask_load_store);
            __m128i fr = _mm_maskload_epi32(int_front + fr_offset + bk_x, mask_load_store);

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

            _mm_maskstore_epi32(int_blend + fr_offset + fr_x, mask_load_store, res);
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------------------

static void no_simd_reculc(const unsigned *const front, const int fr_offset, const int fr_x const int fr_max_x,
                           const unsigned *const back , const int bk_offset, const int bk_x, unsigned *const blend)
{
    log_assert(front != nullptr);
    log_assert(back  != nullptr);

    for (; fr_x < )
}