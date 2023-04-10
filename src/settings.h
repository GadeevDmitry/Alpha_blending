#ifndef SETTINGS_H
#define SETTINGS_H

const char * BACK_FILENAME = "../data/back.bmp" ;   // background filename
const char *FRONT_FILENAME = "../data/front.bmp";   // foreground filename
const char *BLEND_FILENAME = "../data/blend.bmp";   // blending   filename

const int FRONT_OFFSET_X   = 400;                   // horizontal offset of the foreground
const int FRONT_OFFSET_Y   = 400;                   //   vertical offset of the foreground

const int TEST_HEIGHT      = 1000;                  // height (in pixels) of the image to test
const int TEST_WIDTH       = 1000;                  // width  (in pixels) of the image to test
const int TEST_NUM         = 1000;                  // number of tests to average the result

#endif //SETTINGS_H