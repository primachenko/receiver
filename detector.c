#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "detector.h"
#include "debug.h"
#include "dump.h"
#include "queue.h"

detector_t * detector_by_integr_create(int    spb,
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

    DTCT_DBG("integral detector inited");
    DTCT_DBG("samples per bit: %d", spb);
    DTCT_DBG("high level: %5.5f", d->high_lvl);
    DTCT_DBG("low level: %5.5f", d->low_lvl);

    return d;
}

detector_t * detector_by_period_create(char * desc,
                                       double high_lvl,
                                       double low_lvl,
                                       double align_trashhold,
                                       int    init_period,
                                       int    avg_period)
{
    detector_t * d = calloc(1, sizeof(detector_t));
    if (!d)
    {
        DTCT_ERR("calloc failed");
        return NULL;
    }

    strncpy(d->desc, desc, sizeof(d->desc));

    d->high_lvl = high_lvl;
    d->low_lvl = low_lvl;
    d->samples_cnt = 0;

    d->align_trashhold = align_trashhold;
    d->avg_period = avg_period;
    d->init_period = init_period;
    d->state = DETECTOR_INIT;

    DTCT_DBG("period detector '%s' inited", d->desc);
    DTCT_DBG("high level: %5.5f", d->high_lvl);
    DTCT_DBG("low level: %5.5f", d->low_lvl);
    DTCT_DBG("init period: %d", d->init_period);
    DTCT_DBG("avg period: %d", d->avg_period);

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
    if (!d) return;
    if (d->samples_cnt)
    {
        DTCT_DBG("fill undef from %d to %d", d->prev_switch, d->samples_cnt);
        for (int j = d->prev_switch; j < QUEUE_SAMPLES_LEN_MAX; ++j)
            dump_sample_by_desc(d->desc, DOUBLE_LOGIC_UNDEF);
    }
    free(d);
    d = NULL;
}

void detector_detect_by_integr(detector_t * d, double sample)
{
    d->storage += sample;
    d->samples_cnt++;

    if (0 != d->samples_cnt && 0 == d->samples_cnt % d->spb)
    {
        DTCT_DBG("storage %5.5f", d->storage);
        if (d->storage >= d->high_lvl)
        {
            DTCT_DBG("detected high");
            dump_sample_by_desc("detector", DOUBLE_LOGIC_1);
            if (d->recv_high_cb)
            {
                DTCT_DBG("calling recv_high_cb");
                d->recv_high_cb();
            }
        }
        else if (d->storage <= d->low_lvl)
        {
            DTCT_DBG("detected low");
            dump_sample_by_desc("detector", DOUBLE_LOGIC_0);
            if (d->recv_low_cb)
            {
                DTCT_DBG("calling recv_low_cb");
                d->recv_low_cb();
            }
        }
        else
        {
            DTCT_DBG("detected undef");
            dump_sample_by_desc("detector", DOUBLE_LOGIC_UNDEF);
            if (d->recv_undef_cb)
            {
                DTCT_DBG("calling recv_undef_cb");
                d->recv_undef_cb();
            }
        }

        d->storage = 0;
    }
}

detector_rc_e detector_detect_by_period(detector_t * d,
                                        double       sample)
{
    d->samples_cnt++;
    if (DETECTOR_INIT == d->state && 0 == d->samples_cnt % d->init_period)
    {
        if (0 == d->align)
        {
            DTCT_DBG("detector not inited - no signal");
            d->samples_cnt = 0;
            return DETECTOR_NOSIGNAL;
        }
        if (d->align_trashhold > d->align)
        {
            DTCT_DBG("detector not inited - align %f so small", d->align);
            for (int j = 0; j < d->samples_cnt; ++j)
                dump_sample_by_desc(d->desc, DOUBLE_LOGIC_UNDEF);
            d->samples_cnt = 0;
            return DETECTOR_NOSIGNAL;
        }
        d->prev_switch = d->samples_cnt;
        d->state = DETECTOR_SEARCH_HIGH;
        for (int j = 0; j < d->prev_switch; ++j)
            dump_sample_by_desc(d->desc, DOUBLE_LOGIC_UNDEF);
        return DETECTOR_NOERROR;
    }

    if (d->align < sample)
    {
        d->align = sample;
    }

    if (DETECTOR_INIT == d->state)
        return 0;

    sample /= d->align;

    if (d->high_lvl < sample)
    {
        if (DETECTOR_SEARCH_HIGH == d->state)
        {
            d->state = DETECTOR_SEARCH_LOW;
            int len = d->samples_cnt - d->prev_switch;
            if (d->avg_period < len)
            {
                for (int i = 0; i < len/d->avg_period; ++i)
                {
                    DTCT_DBG("detected low");
                    if (d->recv_low_cb)
                    {
                        DTCT_DBG("calling recv_low_cb");
                        d->recv_high_cb();
                    }
                }
                for (int j = 0; j < len; ++j)
                    dump_sample_by_desc(d->desc, DOUBLE_LOGIC_0);
            }
            else
            {
                DTCT_DBG("detected undef");
                for (int j = 0; j < len; ++j)
                    dump_sample_by_desc(d->desc, DOUBLE_LOGIC_UNDEF);
                if (d->recv_undef_cb)
                {
                    DTCT_DBG("calling recv_undef_cb");
                    d->recv_undef_cb();
                }
            }
            d->prev_switch = d->samples_cnt;
        }
    }

    if (d->low_lvl > sample)
    {
        if (DETECTOR_SEARCH_LOW == d->state)
        {
            d->state = DETECTOR_SEARCH_HIGH;
            int len = d->samples_cnt - d->prev_switch;
            if (d->avg_period < len)
            {
                for (int i = 0; i < len/d->avg_period; ++i)
                {
                    DTCT_DBG("detected high");
                    if (d->recv_high_cb)
                    {
                        DTCT_DBG("calling recv_high_cb");
                        d->recv_low_cb();
                    }
                }
                for (int j = 0; j < len; ++j)
                    dump_sample_by_desc(d->desc, DOUBLE_LOGIC_1);
            }
            else
            {
                DTCT_DBG("detected undef");
                for (int j = 0; j < len; ++j)
                    dump_sample_by_desc(d->desc, DOUBLE_LOGIC_UNDEF);
                if (d->recv_undef_cb)
                {
                    DTCT_DBG("calling recv_undef_cb");
                    d->recv_undef_cb();
                }
            }
            d->prev_switch = d->samples_cnt;
        }
    }

    return DETECTOR_NOERROR;
}