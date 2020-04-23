#include "parallel_input.h"
#include "ADS1256.h"
#ifdef DAC_SHIFT
#include "DAC8532.h"
#endif /* DAC_SHIFT */
#include "queue.h"
#include "debug.h"

int parallel_input_init()
{
    DEV_ModuleInit();
#ifdef DAC_SHIFT
    DAC8532_Set_Voltage(DAC8532_CHANNEL_A, DAC8532_VREF/2);
#endif /* DAC_SHIFT */
    if (ADS1256_init())
    {
        PAR_ERR("ads1256 init failed\n");
        DEV_ModuleExit();
        return -1;
    }
    return 0;
}

void * parallel_input_routine(void * cookie)
{
    queue_t * q = cookie;

    double val;

    while (QUEUE_SAMPLES_LEN_MAX >= q->in_counter)
    {
        val = ADS1256_GetValueRDATAC() * ADS1256_MAX_VAL / ADS1256_MAX_BIT_VAL;
        if (0 != queue_push_sample(q, val))
        {
            PAR_ERR("push sample into queue failed");
            break;
        }
    }
    PAR_DBG("routine exit, received %d samples", QUEUE_SAMPLES_LEN_MAX);
    return NULL;
}

void parallel_input_deinit()
{
    DEV_ModuleExit();
}