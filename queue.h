#ifndef QUEUE_H
#define QUEUE_H

#include <inttypes.h>
#include <pthread.h>

#define QUEUE_SAMPLES_LEN_MAX (1024*1024*8)
#define QUEUE_DELAY_TIME_US (300)

typedef struct
{
    double * samples;
    uint32_t size;
    uint32_t in_counter;
    uint32_t out_counter;
    pthread_mutex_t lock;
} queue_t;

int queue_create(queue_t ** q);
void queue_destroy(queue_t * q);
int queue_push_sample(queue_t * q,
                      double    s);
double queue_pop_sample(queue_t * q);

#endif /* QUEUE_H */