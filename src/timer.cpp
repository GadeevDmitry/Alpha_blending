#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../lib/logs/log.h"

#include "alpha_blending.h"
#include "settings.h"

#include "timer.h"

//--------------------------------------------------------------------------------------------------------------------------------
// MAIN
//--------------------------------------------------------------------------------------------------------------------------------

int main(const int argc, const char *argv[])
{
    const int    is_mode_size = sizeof(MODE_NAME) / sizeof(char *);
    bool is_mode[is_mode_size] = {};

    if (!mode_init(argc, argv, is_mode, is_mode_size)) return 0;
    if (is_mode[IS_HELP])
    {
        fprintf(stderr, "Flags:\n"
                        "--all (or none): to run all alpha blending methods\n"
                        "--intrin       : to run \"intrin\"       method \n"
                        "--improved     : to run \"improved\"     method \n"
                        "--help (single): to get manual\n"
                        "\n"
                        "see \"alpha_blending.cpp\" about methods\n\n");
        return 0;
    }

    if (is_mode[IS_ALL] || is_mode[IS_INTRIN])      { double run_time = blending_run(alpha_blending_intrin);
                                                      printf("\"intrin\"   time: %10lf ms\n", run_time); }

    if (is_mode[IS_ALL] || is_mode[IS_IMPROVED])    { double run_time = blending_run(alpha_blending_intrin_improve);
                                                      printf("\"improved\" time: %10lf ms\n", run_time); }

    if (is_mode[IS_ALL] || is_mode[IS_SIMPLE])      { double run_time = blending_run(alpha_blending_simple);
                                                      printf("\"simple\"   time: %10lf ms\n", run_time); }
}

//--------------------------------------------------------------------------------------------------------------------------------

bool mode_init(const int argc, const char *argv[], bool *const is_mode, const int is_mode_size)
{
    log_assert(argv    != nullptr);
    log_assert(is_mode != nullptr);

    if (argc == 1) { is_mode[IS_ALL] = true; return true; }

    for (int i = 0; i < is_mode_size; ++i) is_mode[i] = false;

    for (int i = 1; i <         argc; ++i) { bool is_smth = false;
    for (int j = 0; j < is_mode_size; ++j)
        {
            if (strcmp(argv[i], MODE_NAME[j]) == 0) { is_mode[j] = true;
                                                      is_smth    = true; break; }
        }
        if (!is_smth) { fprintf(stderr, "Undefined mode. Use --help to get manual.\n"); return false; }
    }

    if (is_mode[IS_HELP] && argc > 2) { fprintf(stderr, "You can't use --help with other modes\n"); return false; }

    return true;
}

//--------------------------------------------------------------------------------------------------------------------------------

bool blending_data_init(const int **front,
                        const int **back ,
                              int **blend, int *const width,
                                           int *const height)
{
    log_assert(front != nullptr);
    log_assert(back  != nullptr);
    log_assert(blend != nullptr);

    log_assert(width  != nullptr);
    log_assert(height != nullptr);

    const unsigned data_size = TEST_WIDTH * TEST_HEIGHT;

    (*front) = (int *) log_calloc(data_size, sizeof(unsigned));
    (*back ) = (int *) log_calloc(data_size, sizeof(unsigned));
    (*blend) = (int *) log_calloc(data_size, sizeof(unsigned));

    log_verify(*front != nullptr, false);
    log_verify(*back  != nullptr, false);
    log_verify(*blend != nullptr, false);

    *width  = TEST_WIDTH;
    *height = TEST_HEIGHT;

    return true;
}

//--------------------------------------------------------------------------------------------------------------------------------

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"

void blending_data_dtor(const int *const front,
                        const int *const back ,
                              int *const blend)
{
    if (front != nullptr) log_free((int *) front);
    if (back  != nullptr) log_free((int *) back );
    if (blend != nullptr) log_free((int *) blend);
}

#pragma GCC diagnostic pop

//--------------------------------------------------------------------------------------------------------------------------------
// FRAME_RUN
//--------------------------------------------------------------------------------------------------------------------------------

double blending_run(void (*blending_func)  (const int *front,
                                            const int *back ,
                                                  int *blend, const int width,
                                                              const int height))
{
    log_assert(blending_func != nullptr);

    const int *front = nullptr;
    const int *back  = nullptr;
          int *blend = nullptr;

    int width = 0, height = 0;

    if (!blending_data_init(&front, &back, &blend, &width, &height))
    {
        log_error("can't init data to run alpha_blending\n");

        return -1;
    }

    clock_t start = clock();

    for (int i = 0; i < TEST_NUM; ++i) (*blending_func) (front, back, blend, width, height);

    double ans = reculc_time(clock() - start);
    blending_data_dtor(front, back, blend);

    return ans;
}

//--------------------------------------------------------------------------------------------------------------------------------

double reculc_time(const clock_t run_num_time)
{
    double run_time_ms = (1000.0 * (double) run_num_time) / (TEST_NUM * CLOCKS_PER_SEC);
    return run_time_ms;
}