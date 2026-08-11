#include "gfx.h"
#include <stdint.h>
#include <fileioc.h>

#define MNCFTIMG_HEADER_SIZE 0

unsigned char *MNCFTIMG_appvar[25] =
{
    (unsigned char*)0,
    (unsigned char*)902,
    (unsigned char*)1804,
    (unsigned char*)2706,
    (unsigned char*)3608,
    (unsigned char*)4510,
    (unsigned char*)5412,
    (unsigned char*)6314,
    (unsigned char*)7216,
    (unsigned char*)8118,
    (unsigned char*)9020,
    (unsigned char*)9922,
    (unsigned char*)10824,
    (unsigned char*)11726,
    (unsigned char*)12628,
    (unsigned char*)13030,
    (unsigned char*)13432,
    (unsigned char*)13834,
    (unsigned char*)14236,
    (unsigned char*)14638,
    (unsigned char*)15040,
    (unsigned char*)15972,
    (unsigned char*)16874,
    (unsigned char*)17116,
    (unsigned char*)17358,
};

unsigned char MNCFTIMG_init(void)
{
    uintptr_t data;
    unsigned int i;
    uint8_t appvar;

    appvar = ti_Open("MNCFTIMG", "r");
    if (appvar == 0)
    {
        return 0;
    }

    data = (uintptr_t)ti_GetDataPtr(appvar) - (uintptr_t)MNCFTIMG_appvar[0] + MNCFTIMG_HEADER_SIZE;
    for (i = 0; i < 25; i++)
    {
        MNCFTIMG_appvar[i] += data;
    }

    ti_Close(appvar);

    return 1;
}

