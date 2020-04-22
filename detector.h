#ifndef DETECTOR_H
#define DETECTOR_H

#define DOUBLE_LOGIC_0 (0.0)
#define DOUBLE_LOGIC_1 (1.0)
#define DOUBLE_LOGIC_UNDEF (0.5)
#define HLVL_N (0.7)
#define LLVL_N (0.3)
#define NBIT (600)

typedef enum
{
    DETECTOR_INIT,
    DETECTOR_SEARCH_LOW,
    DETECTOR_SEARCH_HIGH
} detector_state_e;

typedef struct
{
/* commmon variable */
    int samples_cnt;
    double high_lvl;
    double low_lvl;

/* for detection by integral */
    int spb;
    double storage;

/* for detection by period */
    int init_period;
    int avg_period;
    int prev_switch;
    double align;
    detector_state_e state;

/* callbacks for detector events */
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

detector_t * detector_by_integr_create(int    spb,
                                       double one_treshhold,
                                       double zero_treshhold);

detector_t * detector_by_period(double high_lvl,
                                double low_lvl,
                                int    init_period,
                                int    avg_period);

void detector_set_cb(detector_t * d, detector_cb_e type, void (*recv_cb)());

void detector_destroy(detector_t * d);

void detector_detect_by_integr(detector_t * d, double sample);
void detector_detect_by_period(detector_t * d, double sample);

#endif /* DETECTOR_H */