#include <signal.h>
#include <stdlib.h>

#include "receiver.h"
#include "debug.h"

receiver_t * r;

void sig_handler(int signo)
{
    int rc;
    if (signo == SIGINT)
    {
        MAIN_DBG("received SIGINT");
        rc = 0;
    }
    if (signo == SIGTERM)
    {
        MAIN_DBG("received SIGTERM");
        rc = 0;
    }
    if (signo == SIGKILL)
    {
        MAIN_DBG("received SIGKILL");
        rc = -1;
    }
    if (signo == SIGSEGV)
    {
        MAIN_DBG("received SIGSEGV");
        rc = -1;
    }

    receiver_destroy(r);
    exit(rc);
}

int main(int argc, char const *argv[])
{
    MAIN_DBG("receiver started");

    if (SIG_ERR == signal(SIGINT | SIGTERM | SIGKILL |
                          SIGSEGV, sig_handler))
    {
        MAIN_ERR("failed set signal handler");
        return -1;
    }

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