#ifndef RECIEVER_H
#define RECIEVER_H

#include <stdint.h>
#include <stdio.h>

#include "df.h"
#include "detector.h"

#ifdef DEBUG_SOURCE
#define DEBUG_SOURCE_SIZE (1024*8)
#endif /* DEBUG_SOURCE */

#define FILE_FILTER_50     "filter-50"
#define FILE_FILTER_200    "filter-200"
#define FILE_FILTER_400    "filter-400"
#define FILE_ALIGN_200     "align-200"
#define FILE_ALIGN_400     "align-400"
#define FILE_DATA_RECIEVED "data-received"
#define FILE_FILTERS_PARAM "filters.txt"

#define ALIGN_LEN (100)
#define PULSES_FOR_CALC_NORM_COEF (2)
#define PULSES_FOR_FILTER_STABILIZATION (1)
#define SAMPLES_PER_BIT_DEFAULT (600)
#define HIGH_TRASHHOLD_DEFAULT (0.7)
#define LOW_TRASHHOLD_DEFAULT (0.7)

typedef enum
{
    RCVR_SMPL_SOURCE = 0,
    RCVR_SMPL_FILTERED_50HZ,
    RCVR_SMPL_FILTERED_FREQ1,
    RCVR_SMPL_FILTERED_FREQ2,
    RCVR_SMPL_ABS_FREQ1,
    RCVR_SMPL_ABS_FREQ2,
    RCVR_SMPL_ALIGNED_FREQ1,
    RCVR_SMPL_ALIGNED_FREQ2,
    RCVR_SMPL_MAX
} receiver_samples_e;

typedef enum
{
    RCVR_CREATED = 0,
    RCVR_INIT,
    RCVR_RESUME,
    RCVR_RUNNIG,
    RCVR_STOPPED,
    RCVR_NEED_STOP,
    RCVR_PAUSED,
    RCVR_NEED_PAUSE,
    RCVR_DESTROYING
} receiver_state_e;

typedef struct
{
    // dump_t * dump;
    // import_t * import;

    filter_t * filter_50hz;
    filter_t * filter_freq1;
    filter_t * filter_freq2;
    filter_t * align_freq1;
    filter_t * align_freq2;

    detector_t * detector_freq1;
    detector_t * detector_freq2;

    double normalize_freq1;
    double normalize_freq2;

    double samples[RCVR_SMPL_MAX];

    uint64_t samples_cnt;

    receiver_state_e state; /* need sync */
} receiver_t;

int receiver_create(receiver_t ** rcvr);

int receiver_loop(receiver_t * rcvr);

void received_state_resume(receiver_t * rcvr);

void received_state_stop(receiver_t * rcvr);

void received_state_pause(receiver_t * rcvr);

void receiver_destroy(receiver_t * rcvr);

#endif /* RECIEVER_H */