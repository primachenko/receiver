#ifndef DETECTOR_H
#define DETECTOR_H

#define DOUBLE_LOGIC_0 (0.0)
#define DOUBLE_LOGIC_1 (1.0)
#define DOUBLE_LOGIC_UNDEF (0.5)
#define HLVL_N (0.7)
#define LLVL_N (0.3)
#define NBIT (600)

typedef struct
{
    int spb;
    int samples_cnt;
    double high_lvl;
    double low_lvl;
    double storage;
    void (*recv_high_cb)();
    void (*recv_low_cb)();
    void (*recv_undef_cb)();
} detector_t;

typedef enum
{
    DETECTOR_HIGH_CB,
    DETECTOR_LOW_CB,
    DETECTOR_UNDEF_CB
} detector_cb_e;

detector_t * detector_create(int    spb,
                             double one_treshhold,
                             double zero_treshhold);

void detector_set_cb(detector_t * d, detector_cb_e type, void (*recv_cb)());

void detector_destroy(detector_t * d);

int detector_detect(detector_t * d, double * sample);

#endif /* DETECTOR_H */