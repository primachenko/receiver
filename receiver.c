#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

#include "dump.h"
#include "import.h"
#include "receiver.h"
#include "debug.h"

#define DATA_BITS_SIZE (65535)
#define PREAMBLE_SEQUENCE (0xAB)

uint8_t data[DATA_BITS_SIZE+1];
uint16_t data_idx;

static int search_byte(uint8_t need, uint8_t * data, int size)
{
    for (int i = 0; i < size - 7; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            if (((need >> (7 - j)) & 0x01) != data[i+j])
                break;
            if (j == 7)
                return i + 8;
        }
    }
    return -1;
}

static char byte2ascii(uint8_t * data)
{
    char ch = 0;
    for (int i = 0; i < 8; ++i)
    {
        // RCVR_DBG("%d", data[i]);
        ch |= data[i] << (7 - i);
    }
    // RCVR_DBG("b = '%c'\n", ch);

    return ch;
}

static void receiver_get_data_str(receiver_t * r)
{
    static int idx = 0;
    static int new_idx = 0;
    static char buffer[DATA_BITS_SIZE] = {};
    static int rc = 0;
    static uint64_t bytes_cnt = 0;

    static int search_preamble = 1;
    // RCVR_DBG("data_idx=%d", data_idx);
    if (search_preamble)
    {
        if (idx + 8 > data_idx)
            return;

        new_idx = search_byte(PREAMBLE_SEQUENCE, &data[idx], data_idx - idx);
        if (-1 == new_idx)
            return;
        idx += new_idx;
        search_preamble = 0;
        rc += sprintf(buffer + rc, "\t");
        // RCVR_DBG("preamble founded, data begin from idx = %d", idx + new_idx);
    }
    else
    {
        if (idx + 8 > data_idx)
        {
            // RCVR_DBG(" need more bits, skip");
            return;
        }

        // RCVR_DBG("ascii search from idx = %d", idx);
        char ch = byte2ascii(&data[idx]);
        idx += 8;
        bytes_cnt++;
        if (0 == ch)
        {
            search_preamble = 1;
            rc = 0;
            // RCVR_DBG("end data, search new preamble from idx = %d", idx);
            RCVR_DATA("receiver decoded ascii data:\n%s", buffer);
            RCVR_LOG_EXT("%s <- %llu", buffer, bytes_cnt);
            return;
        }
        rc += sprintf(buffer + rc, "%c", ch);
    }
}

void dt_high_cb_freq1(void * cookie)
{
    RCVR_CB("[FREQ1] HIGH");
    dump_sample_by_desc("bits-200", DOUBLE_LOGIC_1);
}
void dt_low_cb_freq1(void * cookie)
{
    RCVR_CB("[FREQ1] LOW");
    dump_sample_by_desc("bits-200", DOUBLE_LOGIC_0);
}
void dt_undef_cb_freq1(void * cookie)
{
    RCVR_CB("[FREQ1] UNDEF");
    dump_sample_by_desc("bits-200", DOUBLE_LOGIC_UNDEF);
}
void dt_high_cb_freq2(void * cookie)
{
    RCVR_CB("[FREQ2] LOW");
    dump_sample_by_desc("bits-400", DOUBLE_LOGIC_0);
}
void dt_low_cb_freq2(void * cookie)
{
    RCVR_CB("[FREQ2] HIGH");
    dump_sample_by_desc("bits-400", DOUBLE_LOGIC_1);
}
void dt_undef_cb_freq2(void * cookie)
{
    RCVR_CB("FREQ2] UNDEF");
    dump_sample_by_desc("bits-400", DOUBLE_LOGIC_UNDEF);
}

void dt_high_cb(void * cookie)
{
    dump_sample_by_desc("result", DOUBLE_LOGIC_1);
    data[data_idx++] = 1;
    receiver_get_data_str(cookie);
}
void dt_low_cb(void * cookie)
{
    dump_sample_by_desc("result", DOUBLE_LOGIC_0);
    data[data_idx++] = 0;
    receiver_get_data_str(cookie);
}
void dt_undef_cb(void * cookie)
{
    RCVR_CB("UNDEF value, use random data");
    dump_sample_by_desc("result", DOUBLE_LOGIC_UNDEF);
    data[data_idx++] = (int)(rand()/(RAND_MAX/2));
    receiver_get_data_str(cookie);
}

char * receiver_stringize_state(receiver_state_e state)
{
    switch (state)
    {
        case RCVR_CREATED:
            return "CREATED";
        case RCVR_INIT:
            return "INIT";
        case RCVR_RESUME:
            return "RESUME";
        case RCVR_RUNNIG:
            return "RUNNIG";
        case RCVR_STOPPED:
            return "STOPPED";
        case RCVR_PAUSED:
            return "PAUSED";
        case RCVR_DESTROYING:
            return "DESTROYING";
        default:
            return "UNDEFINED";
    }
}

void receiver_destroy(receiver_t * r)
{
    if (!r)
    {
        RCVR_DBG("reciever not created");
        return;
    }
    r->state = RCVR_DESTROYING;
    RCVR_DBG("receiver change state to %s", receiver_stringize_state(r->state));

    if (r->filter_50hz)
    {
        RCVR_DBG("filter 50hz destroy");
        filter_destroy(r->filter_50hz);
    }
    if (r->filter_freq1)
    {
        RCVR_DBG("filter freq1 destroy");
        filter_destroy(r->filter_freq1);
    }
    if (r->filter_freq2)
    {
        RCVR_DBG("filter freq2 destroy");
        filter_destroy(r->filter_freq2);
    }
    if (r->align_freq1)
    {
        RCVR_DBG("align freq1 destroy");
        filter_destroy(r->align_freq1);
    }
    if (r->align_freq2)
    {
        RCVR_DBG("align freq2 destroy");
        filter_destroy(r->align_freq2);
    }
#if defined DETECTOR_INTEG || defined DETECTOR_PERIOD
    if (r->detector_freq1)
    {
        RCVR_DBG("detector freq1 destroy");
        detector_destroy(r->detector_freq1);
    }
    if (r->detector_freq2)
    {
        RCVR_DBG("detector freq2 destroy");
        detector_destroy(r->detector_freq2);
    }
#endif /* DETECTOR_INTEG || DETECTOR_PERIOD */
    dump_destroy();
#ifdef LOG_EXTERNAL
    RCVR_LOG_EXT("ext log deinit");
    sleep(10); /* for demo */
    RCVR_LOG_EXT("done");
    RCVR_LOG_EXT("dieLogEx");
    pclose(r->efd);
#endif /* LOG_EXTERNAL */
#ifndef DEBUG_SOURCE
    queue_destroy(r->queue);
    if (r->parallel_input_tid)
        pthread_join(r->parallel_input_tid, NULL);
    parallel_input_deinit();
#endif /* DEBUG_SOURCE */
    RCVR_DBG("receiver destroy");
    free(r);
    r = NULL;
}

int receiver_dump_init(receiver_t * r)
{
    dump_create();
    dump_fd_add("original",     FILE_ORIGINAL);;
    dump_fd_add("filter-50",    FILE_FILTER_50);
    dump_fd_add("filter-200",   FILE_FILTER_200);
    dump_fd_add("filter-400",   FILE_FILTER_400);
    dump_fd_add("fabs-200",     FILE_FABS_200);
    dump_fd_add("fabs-400",     FILE_FABS_400);
    dump_fd_add("align-200",    FILE_ALIGN_200);
    dump_fd_add("align-400",    FILE_ALIGN_400);
    dump_fd_add("detector-200", FILE_DETECTOR_200);
    dump_fd_add("detector-400", FILE_DETECTOR_400);
    dump_fd_add("bits-200",     FILE_BITS_200);
    dump_fd_add("bits-400",     FILE_BITS_400);
    dump_fd_add("result",       FILE_BITS_RESULT);

    return 0;
}

int receiver_detectors_init(receiver_t * r)
{
#ifdef DETECTOR_INTEG
    r->detector_freq1 = detector_by_integr_create(SAMPLES_PER_BIT_DEFAULT,
                                                  HIGH_TRASHHOLD_DEFAULT_INTEG,
                                                  LOW_TRASHHOLD_DEFAULT_INTEG);
    if (!r->detector_freq1)
    {
        RCVR_ERR("creating detector for freq1 failed");
        return -1;
    }
    r->detector_freq2 = detector_by_integr_create(SAMPLES_PER_BIT_DEFAULT,
                                                  HIGH_TRASHHOLD_DEFAULT_INTEG,
                                                  LOW_TRASHHOLD_DEFAULT_INTEG);
    if (!r->detector_freq2)
    {
        RCVR_ERR("creating detector for freq2 failed");
        return -1;
    }
#elif defined DETECTOR_PERIOD
    r->detector_freq1 = detector_by_period_create("detector-200",
                                                  HIGH_TRASHHOLD_DEFAULT_PERIOD,
                                                  LOW_TRASHHOLD_DEFAULT_PERIOD,
                                                  ALIGN_TRASHHOLD,
                                                  PULSES_FOR_CALC_NORM_COEF*AVG_PERIOD_FREQ1,
                                                  AVG_PERIOD_FREQ1);
    if (!r->detector_freq1)
    {
        RCVR_ERR("creating detector for freq1 failed");
        return -1;
    }
    r->detector_freq2 = detector_by_period_create("detector-400",
                                                  HIGH_TRASHHOLD_DEFAULT_PERIOD,
                                                  LOW_TRASHHOLD_DEFAULT_PERIOD,
                                                  ALIGN_TRASHHOLD,
                                                  PULSES_FOR_CALC_NORM_COEF*AVG_PERIOD_FREQ2,
                                                  AVG_PERIOD_FREQ2);
    if (!r->detector_freq2)
    {
        RCVR_ERR("creating detector for freq2 failed");
        return -1;
    }
#elif defined DETECTOR_CMP
    r->detector = detector_cmp_create(GIST_COEF_DEFAULT,
                                      PULSES_FOR_FILTER_STABILIZATION * AVG_PERIOD,
                                      AVG_PERIOD);
    if (!r->detector)
    {
        RCVR_ERR("creating detector failed");
        return -1;
    }
    detector_set_cb(r->detector, DETECTOR_HIGH_CB, dt_high_cb);
    detector_set_cb(r->detector, DETECTOR_LOW_CB, dt_low_cb);
    detector_set_cb(r->detector, DETECTOR_UNDEF_CB, dt_undef_cb);
#endif
#if defined DETECTOR_INTEG || defined DETECTOR_PERIOD
    detector_set_cb(r->detector_freq1, DETECTOR_HIGH_CB, dt_high_cb_freq1);
    detector_set_cb(r->detector_freq1, DETECTOR_LOW_CB, dt_low_cb_freq1);
    detector_set_cb(r->detector_freq1, DETECTOR_UNDEF_CB, dt_undef_cb_freq1);
    detector_set_cb(r->detector_freq2, DETECTOR_HIGH_CB, dt_high_cb_freq2);
    detector_set_cb(r->detector_freq2, DETECTOR_LOW_CB, dt_low_cb_freq2);
    detector_set_cb(r->detector_freq2, DETECTOR_UNDEF_CB, dt_undef_cb_freq2);
#endif /* DETECTOR_INTEG || DETECTOR_PERIOD */
    return 0;
}

size_t receiver_calc_align_coefs(double * b, size_t size)
{
    for (int j = 0; j < ALIGN_LEN; ++j)
    {
        b[j] = 1;
    }

    return size;
}

int receiver_filters_init(receiver_t * r)
{
    import_t * i = import_create_from_file(FILE_FILTERS_PARAM);
    if (!i)
    {
        RCVR_ERR("import filter params from file '%s' failed", FILE_FILTERS_PARAM);
        return -1;
    }


    double b_align[ALIGN_LEN];
    const int b_align_size = receiver_calc_align_coefs(b_align, ALIGN_LEN);

    r->align_freq1 = filter_fir_create(&b_align[0], b_align_size);
    if (!r->align_freq1)
    {
        RCVR_ERR("align filter for freq1 create failed");
        return -1;
    }

    r->align_freq2 = filter_fir_create(&b_align[0], b_align_size);
    if (!r->align_freq2)
    {
        RCVR_ERR("align filter for freq2 create failed");
        return -1;
    }

    for (int j = 0; j < i->count; j++)
    {
        import_entry_t * e = i->entries[j];
        if (!e) continue;

        if (0 == strcmp("50hz", e->desc))
        {
            r->filter_50hz = filter_iir_create(e->a, e->a_size, e->b, e->b_size);
            if (!r->filter_50hz)
            {
                RCVR_ERR("filter for 50hz create failed");
                return -1;
            }
            RCVR_DBG("filter 50hz was added");
        }
        else if (0 == strcmp("200hz", e->desc))
        {
            r->filter_freq1 = filter_iir_create(e->a, e->a_size, e->b, e->b_size);
            if (!r->filter_freq1)
            {
                RCVR_ERR("align filter for freq2 create failed");
                return -1;
            }
            RCVR_DBG("filter freq1 was added");
        }
        else if (0 == strcmp("400hz", e->desc))
        {
            r->filter_freq2 = filter_iir_create(e->a, e->a_size, e->b, e->b_size);
            if (!r->filter_freq2)
            {
                RCVR_ERR("align filter for freq2 create failed");
                return -1;
            }
            RCVR_DBG("filter freq2 was added");
        }
        else
        {
            continue;
            RCVR_DBG("no filter '%s' required, skipping", e->desc);
        }
    }

    import_destroy(i);

    return 0;
}

int receiver_create(receiver_t ** rcvr)
{
    receiver_t * r = calloc(1, sizeof(receiver_t));
    if (!r)
    {
        RCVR_ERR("calloc failed");
        return -1;
    }

    if (0 != receiver_dump_init(r))
    {
        RCVR_ERR("dump init failed");
        receiver_destroy(r);
        return -1;
    }

#ifdef LOG_EXTERNAL
    r->efd = popen("./logExternal.py", "w");
    if (!r->efd)
    {
        RCVR_ERR("log external init failed");
        receiver_destroy(r);
        return -1;
    }
    RCVR_LOG_EXT("ext log init OK");
#endif /* LOG_EXTERNAL */
    if (0 != receiver_filters_init(r))
    {
        RCVR_ERR("filters init failed");
        receiver_destroy(r);
        return -1;
    }

    srand(time(NULL));

    if (0 != receiver_detectors_init(r))
    {
        RCVR_ERR("detectors init failed");
        receiver_destroy(r);
        return -1;
    }
#ifndef DEBUG_SOURCE
    if (0 != queue_create(&r->queue))
    {
        RCVR_ERR("queue init failed");
        RCVR_LOG_EXT("queue init ERR");
        receiver_destroy(r);
        return -1;
    }
    if (0 != parallel_input_init())
    {
        RCVR_ERR("parallel input init failed");
        receiver_destroy(r);
        return -1;
    }
    if (0 != pthread_create(&r->parallel_input_tid, NULL, parallel_input_routine, r->queue))
    {
        RCVR_ERR("parallel input thread create failed");
        receiver_destroy(r);
        return -1;
    }
    RCVR_LOG_EXT("in thread start");
    struct sched_param params;
    params.sched_priority = sched_get_priority_max(SCHED_FIFO);
    if (0 != pthread_setschedparam(r->parallel_input_tid, SCHED_FIFO, &params))
    {
        RCVR_ERR("set realrime prio for parallel input thread failed");
        receiver_destroy(r);
        return -1;
    }
    RCVR_LOG_EXT("thread prio OK");
#endif /* DEBUG_SOURCE */

    r->state = RCVR_INIT;
    RCVR_DBG("receiver change state to %s", receiver_stringize_state(r->state));

    *rcvr = r;

    RCVR_LOG_EXT("rcvr create OK");
    RCVR_DBG("receiver was created");

    return 0;
}

int receiver_init_process(receiver_t * r)
{
    if (r->samples_cnt > PULSES_FOR_CALC_NORM_COEF * SAMPLES_PER_BIT_DEFAULT)
    {
        if (0 >= r->normalize_freq1)
        {
            RCVR_ERR("receiver init failed for freq1");
            return -1;
        }
        else
        {
            RCVR_DBG("init done for freq1, normalize_freq1=%f", r->normalize_freq1);
        }

        if (0 >= r->normalize_freq2)
        {
            RCVR_ERR("receiver init failed for freq2");
            return -1;
        }
        else
        {
            RCVR_DBG("init done for freq2, normalize_freq2=%f", r->normalize_freq2);
        }

        r->state = RCVR_RUNNIG;

        return 0;
    }

    if (r->samples_cnt > PULSES_FOR_FILTER_STABILIZATION * SAMPLES_PER_BIT_DEFAULT)
    {
        if (r->normalize_freq1 < r->samples[RCVR_SMPL_ALIGNED_FREQ1])
            r->normalize_freq1 = r->samples[RCVR_SMPL_ALIGNED_FREQ1];
        if (r->normalize_freq2 < r->samples[RCVR_SMPL_ALIGNED_FREQ2])
            r->normalize_freq2 = r->samples[RCVR_SMPL_ALIGNED_FREQ2];
    }

    return 0;
}

int receiver_loop(receiver_t * r)
{
    RCVR_LOG_EXT("loop started");
    for (;RCVR_STOPPED != r->state;)
    {
        if (RCVR_PAUSED == r->state)
        {
            RCVR_DBG("receiver state %s", receiver_stringize_state(r->state));
            sleep(1);
        }

#ifdef SAMPLES_LIMIT
        if (QUEUE_SAMPLES_LEN_MAX <= r->samples_cnt)
        {
            r->state = RCVR_STOPPED;
            RCVR_LOG_EXT("limit reached");
            RCVR_DBG("samples limit was reached");
            RCVR_DBG("receiver change state to %s", receiver_stringize_state(r->state));
            break;
        }
#endif /* SAMPLES_LIMIT */
#ifdef DEBUG_SOURCE
        if (0 != dump_dbg_source_get_sample(&r->samples[RCVR_SMPL_SOURCE]))
        {
            RCVR_ERR("can't get sample from dbg source");
            return -1;
        }
#else
        r->samples[RCVR_SMPL_SOURCE] = queue_pop_sample(r->queue);
        if (FP_INFINITE == r->samples[RCVR_SMPL_SOURCE])
        {
            RCVR_LOG_EXT("wait new data");
            RCVR_DBG("sample %llu is INF, wait new samples", r->samples_cnt);
            continue;
        }
#endif /* DEBUG_SOURCE */
        r->samples_cnt++;

        dump_sample_by_desc("original", r->samples[RCVR_SMPL_SOURCE]);

        r->samples[RCVR_SMPL_FILTERED_50HZ] = filter_apply(r->filter_50hz, r->samples[RCVR_SMPL_SOURCE]);
        dump_sample_by_desc("filter-50", r->samples[RCVR_SMPL_FILTERED_50HZ]);

        r->samples[RCVR_SMPL_FILTERED_FREQ1] = filter_apply(r->filter_freq1, r->samples[RCVR_SMPL_FILTERED_50HZ]);
        dump_sample_by_desc("filter-200", r->samples[RCVR_SMPL_FILTERED_FREQ1]);

        r->samples[RCVR_SMPL_FILTERED_FREQ2] = filter_apply(r->filter_freq2, r->samples[RCVR_SMPL_FILTERED_50HZ]);
        dump_sample_by_desc("filter-400", r->samples[RCVR_SMPL_FILTERED_FREQ2]);

        r->samples[RCVR_SMPL_ABS_FREQ1] = fabs(r->samples[RCVR_SMPL_FILTERED_FREQ1]);
        dump_sample_by_desc("fabs-200", r->samples[RCVR_SMPL_ABS_FREQ1]);
        r->samples[RCVR_SMPL_ABS_FREQ2] = fabs(r->samples[RCVR_SMPL_FILTERED_FREQ2]);
        dump_sample_by_desc("fabs-400", r->samples[RCVR_SMPL_ABS_FREQ2]);


        r->samples[RCVR_SMPL_ALIGNED_FREQ1] = filter_apply(r->align_freq1, r->samples[RCVR_SMPL_ABS_FREQ1]);
        r->samples[RCVR_SMPL_ALIGNED_FREQ2] = filter_apply(r->align_freq2, r->samples[RCVR_SMPL_ABS_FREQ2]);

#ifdef DETECTOR_INTEG
        if (RCVR_INIT == r->state || RCVR_RESUME == r->state)
        {
            if (0 != receiver_init_process(r))
            {
                RCVR_ERR("receiver init process failed");
                return -1;
            }

            continue;;
        }

        r->samples[RCVR_SMPL_ALIGNED_FREQ1] /= r->normalize_freq1;
        r->samples[RCVR_SMPL_ALIGNED_FREQ2] /= r->normalize_freq2;
#endif /* DETECTOR_INTEG */
        dump_sample_by_desc("align-200", r->samples[RCVR_SMPL_ALIGNED_FREQ1]);
        dump_sample_by_desc("align-400", r->samples[RCVR_SMPL_ALIGNED_FREQ2]);
#ifdef DETECTOR_INTEG
        detector_detect_by_integr(r->detector_freq1, r->samples[RCVR_SMPL_ALIGNED_FREQ1]);
        detector_detect_by_integr(r->detector_freq2, r->samples[RCVR_SMPL_ALIGNED_FREQ2]);
#endif /* DETECTOR_INTEGR */
#ifdef DETECTOR_PERIOD
        detector_detect_by_period(r->detector_freq1, r->samples[RCVR_SMPL_ALIGNED_FREQ1]);
        detector_detect_by_period(r->detector_freq2, r->samples[RCVR_SMPL_ALIGNED_FREQ2]);
#endif /* DETECTOR_PERIOD */
#ifdef DETECTOR_CMP
        detector_detect_cmp(r->detector, r, r->samples[RCVR_SMPL_ALIGNED_FREQ1], r->samples[RCVR_SMPL_ALIGNED_FREQ2]);
#endif /* DETECTOR_CMP */
    }

    return 0;
}

void received_state_resume(receiver_t * r)
{
    r->state = RCVR_RESUME;
    RCVR_DBG("receiver change state to %s", receiver_stringize_state(r->state));
}

void received_state_stop(receiver_t * r)
{
    r->state = RCVR_STOPPED;
    RCVR_DBG("receiver change state to %s", receiver_stringize_state(r->state));
}

void received_state_pause(receiver_t * r)
{
    r->state = RCVR_PAUSED;
    RCVR_DBG("receiver change state to %s", receiver_stringize_state(r->state));
}