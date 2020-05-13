#include "queue.h"
#include "debug.h"

#include <stdlib.h>
#include <unistd.h>

#include <math.h>

int queue_create(queue_t ** q)
{
    queue_t * q_in = calloc(1, sizeof(queue_t));
    if (!q_in)
    {
        QUEUE_ERR("calloc failed");
        return -1;
    }

    q_in->samples = calloc(QUEUE_SAMPLES_LEN_MAX, sizeof(double));
    if (!q_in->samples)
    {
        QUEUE_ERR("calloc failed");
        free(q_in);
        return -1;
    }

    if (0 != pthread_mutex_init(&q_in->lock, NULL))
    {
        QUEUE_ERR("mutex init failed");
        free(q_in->samples);
        free(q_in);
        return -1;
    }

    q_in->size = QUEUE_SAMPLES_LEN_MAX;

    *q = q_in;
    QUEUE_DBG("queue created");
    return 0;
}

void queue_destroy(queue_t * q)
{
    if (!q)
    {
        QUEUE_ERR("queue is null");
        return;
    }

    if (q->samples)
    {
        free(q->samples);
        q->samples = NULL;
    }

    pthread_mutex_destroy(&q->lock);

    free(q);
    q = NULL;

    QUEUE_DBG("queue was destroyed");
}

int queue_push_sample(queue_t * q,
                      double    s)
{
    pthread_mutex_lock(&q->lock);
#ifdef SAMPLES_LIMIT
    if (QUEUE_SAMPLES_LEN_MAX < q->in_counter)
    {
        QUEUE_DBG("queue is full");
        pthread_mutex_unlock(&q->lock);
        return -1;
    }
#else
    if (QUEUE_SAMPLES_LEN_MAX == q->in_counter)
        q->in_counter = 0;
    QUEUE_DBG("queue in counter loop");
#endif /* SAMPLES_LIMIT */
    q->samples[q->in_counter] = s;
    q->in_counter++;
    pthread_mutex_unlock(&q->lock);

    return 0;
}

double queue_pop_sample(queue_t * q)
{
    pthread_mutex_lock(&q->lock);
#ifdef SAMPLES_LIMIT
    if (q->out_counter > q->in_counter)
    {
        QUEUE_DBG("queue is empty");
        pthread_mutex_unlock(&q->lock);
        usleep(QUEUE_DELAY_TIME_US);
        return FP_INFINITE;
    }
#else
    if (QUEUE_SAMPLES_LEN_MAX == q->out_counter)
        q->out_counter = 0;
    QUEUE_DBG("queue out counter loop");
#endif /* SAMPLES_LIMIT */
    double val = q->samples[q->out_counter];
    q->out_counter++;
    pthread_mutex_unlock(&q->lock);
    return val;
}