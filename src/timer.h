#ifndef TIMER_H
#define TIMER_H

#include <time.h>

//--------------------------------------------------------------------------------------------------------------------------------
// GLOBAL
//--------------------------------------------------------------------------------------------------------------------------------

enum MODE
{
    IS_ALL    = 0   ,
    IS_INTRIN       ,
    IS_IMPROVED     ,
    IS_SIMPLE       ,
    IS_HELP         ,
};

const char *MODE_NAME[] =
{
    "--all"         ,
    "--intrin"      ,
    "--improved"    ,
    "--simple"      ,
    "--help"        ,
};


//--------------------------------------------------------------------------------------------------------------------------------

bool mode_init(const int   argc,
               const char *argv[],  bool *const is_mode,
                                    const int   is_mode_size);

bool blending_data_init(const int **front,
                        const int **back ,
                              int **blend, int *const width,
                                           int *const height);

void blending_data_dtor(const int *const front,
                        const int *const back ,
                              int *const blend);

//--------------------------------------------------------------------------------------------------------------------------------

double blending_run(void (*blending_func)  (const int *front,
                                            const int *back ,
                                                  int *blend, const int width,
                                                              const int height));
double reculc_time(const clock_t run_num_time);

#endif //TIMER_H