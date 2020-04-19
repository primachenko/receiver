#include "receiver.h"
#include "debug.h"

int main(int argc, char const *argv[])
{
    MAIN_DBG("receiver started");

    receiver_t * r;

    if (0 != receiver_create(&r))
    {
        MAIN_ERR("failed create receiver");
        return -1;
    }

    MAIN_DBG("receiver loop started");

    if (0 != receiver_loop(r))
    {
        MAIN_ERR("receiver loop failed");
        receiver_destroy(r);
        return -1;
    }

    MAIN_DBG("receiver loop done");

    receiver_destroy(r);

    MAIN_DBG("receiver destroyed, exit");

    return 0;
}