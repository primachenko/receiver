#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "detector.h"
#include "debug.h"

detector_t * detector_create(int    spb,
                             double one_treshhold,
                             double zero_treshhold)
{
    detector_t * d = calloc(1, sizeof(detector_t));
    if (!d)
    {
        DTCT_ERR("calloc failed");
        return NULL;
    }
    d->spb = spb;
    d->high_lvl = one_treshhold * spb;
    d->low_lvl = zero_treshhold * spb;
    d->samples_cnt = 0;
    d->storage = 0;

    DTCT_DBG("detector inited");
    DTCT_DBG("samples per bit: %d", spb);
    DTCT_DBG("high level: %5.5f", one_treshhold);
    DTCT_DBG("high level: %5.5f", d->high_lvl);
    DTCT_DBG("low level: %5.5f", zero_treshhold);
    DTCT_DBG("low level: %5.5f", d->low_lvl);

    return d;
}

void detector_set_cb(detector_t * d, detector_cb_e type, void (*recv_cb)())
{
    switch(type)
    {
        case DETECTOR_HIGH_CB:
#ifdef DETECTOR_DEBUG
            if (d->recv_high_cb)
            {
                DTCT_DBG("callback for high already set, redefine");
            }
#endif /* DETECTOR_DEBUG */
            d->recv_high_cb = recv_cb;
            break;
        case DETECTOR_LOW_CB:
#ifdef DETECTOR_DEBUG
            if (d->recv_low_cb)
            {
                DTCT_DBG("callback for low already set, redefine");
            }
#endif /* DETECTOR_DEBUG */
            d->recv_low_cb = recv_cb;
            break;
        case DETECTOR_UNDEF_CB:
#ifdef DETECTOR_DEBUG
            if (d->recv_undef_cb)
            {
                DTCT_DBG("callback for undef already set, redefine");
            }
#endif /* DETECTOR_DEBUG */
            d->recv_undef_cb = recv_cb;
            break;
        default:
            DTCT_ERR("undefined callback type");
    }
}

void detector_destroy(detector_t * d)
{
    if (d)
    {
        free(d);
        d = NULL;
    }
}

int detector_detect(detector_t * d, double * sample)
{
    d->storage += *sample;
    d->samples_cnt++;

    if (0 != d->samples_cnt && 0 == d->samples_cnt % d->spb)
    {
        DTCT_DBG("storage %5.5f", d->storage);
        if (d->storage >= d->high_lvl)
        {
            DTCT_DBG("detected high");
            if (d->recv_high_cb)
            {
                DTCT_DBG("calling recv_high_cb");
                d->recv_high_cb();
            }
        }
        else if (d->storage <= d->low_lvl)
        {
            DTCT_DBG("detected low");
            if (d->recv_low_cb)
            {
                DTCT_DBG("calling recv_low_cb");
                d->recv_low_cb();
            }
        }
        else
        {
            DTCT_DBG("detected undef");
            if (d->recv_undef_cb)
            {
                DTCT_DBG("calling recv_undef_cb");
                d->recv_undef_cb();
            }
        }

        d->storage = 0;
    }

    return 0;
}