#include <stdio.h>
#include <stdlib.h>

#include "../lib/logs/log.h"

#include <SFML/Graphics.hpp>

#include "settings.h"
#include "picture.h"

//--------------------------------------------------------------------------------------------------------------------------------

bool picture_open (picture *const front, picture *const back, picture *const blend);
void picture_close(picture *const front, picture *const back, picture *const blend);

//--------------------------------------------------------------------------------------------------------------------------------
// MAIN
//--------------------------------------------------------------------------------------------------------------------------------

int main()
{
    picture front = {};
    picture back  = {};
    picture blend = {};

    if (!picture_open(&front, &back, &blend)) return 0;

    picture_alpha_blending(&front, &back, &blend);

    sf::Image result;

    result.create    ((unsigned) blend.size.x, (unsigned) blend.size.y, (sf::Uint8 *) blend.pixels);
    result.saveToFile(BLEND_FILENAME);

    picture_close(&front, &back, &blend);
}

//--------------------------------------------------------------------------------------------------------------------------------

bool picture_open(picture *const front, picture *const back, picture *const blend)
{
    log_assert(front != nullptr);
    log_assert(back  != nullptr);
    log_assert(blend != nullptr);

    if (!picture_init_by_bmp(back , BACK_FILENAME))                       return false;
    if (!picture_ctor       (blend, back->size   )) { picture_dtor(back); return false; }

    frame frame_front = {};

    if (!frame_init_by_bmp(&frame_front, FRONT_FILENAME, {FRONT_OFFSET_X, FRONT_OFFSET_Y}))
    {
        picture_dtor(back );
        picture_dtor(blend);

        return false;
    }

    if (!picture_init_by_frame(front, &frame_front, back->size, 0U))
    {
        picture_dtor(back );
        picture_dtor(blend);
          frame_dtor(&frame_front);

        return false;
    }

    frame_dtor(&frame_front);

    return true;
}

void picture_close(picture *const front, picture *const back, picture *const blend)
{
    log_assert(front != nullptr);
    log_assert(back  != nullptr);
    log_assert(blend != nullptr);

    picture_dtor(front);
    picture_dtor(back );
    picture_dtor(blend);
}