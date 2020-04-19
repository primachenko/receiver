#include <stdlib.h>
#include <string.h>

#include "dump.h"
#include "debug.h"

dump_t * d;
int single;

void safe_free(void * p)
{
    if (p)
    {
        free(p);
        p = NULL;
    }
}

void dump_fd_destroy(dump_fd_t * dfd)
{
    if (0 != fclose(dfd->fd))
    {
        DMP_ERR("fclose for '%s' failed", dfd->desc);
    }
    safe_free(dfd);
}

#ifdef DEBUG_SOURCE
int dump_dbg_source_open()
{
    FILE * tmp = fopen(DUMP_DBG_SOURCE, "r");
    if (!tmp)
    {
        DMP_ERR("fopen '%s' failed", DUMP_DBG_SOURCE);
        return -1; 
    }

    dump_fd_t * fd = calloc(1, sizeof(dump_fd_t));
    if (!fd)
    {
        DMP_ERR("calloc failed");
        return -1;
    }

    fd->fd = tmp;
    strncpy(fd->desc, DUMP_DBG_SOURCE_DESC, sizeof(fd->desc));

    d->source_fd = fd;
    DMP_DBG("source fd '%s' was added", fd->desc);
    return 0;
}

void dump_dbg_source_close()
{
    if (!d->source_fd)
    {
        DMP_DBG("source fd doesn't exist");
        return;
    }

    dump_fd_destroy(d->source_fd);
    DMP_DBG("source fd was destroyed");
}

int dump_dbg_source_get_sample(double * sample)
{
    size_t readed = fread(sample, sizeof(double), 1, d->source_fd->fd);
    if (!readed)
    {
        DMP_ERR("fread failed");
        return -1;
    }

    return 0;
}
#endif /* DEBUG_SOURCE */

int dump_create()
{
    if (single)
    {
        DMP_DBG("already created");
        return 0;
    }

    d = calloc(1, sizeof(dump_t));
    if (!d)
    {
        DMP_ERR("calloc failed");
        return 0;
    }
#ifdef DEBUG_SOURCE
    if (0 != dump_dbg_source_open())
    {
        DMP_ERR("can't open default debug source failed");
        return 0;
    }
#endif /* DEBUG_SOURCE */
    single = 1;

    return 0;
}

int dump_fd_add(const char * desc, const char * path)
{
    if (DUMP_DESC_FD_MAX <= d->count)
    {
        DMP_DBG("can't add more fds");
        return 0;
    }

    if (!desc)
    {
        DMP_ERR("desc arg is NULL");
        return -1;
    }

    if (!path)
    {
        DMP_ERR("path arg is NULL");
        return -1;
    }

    FILE * tmp = fopen(path, "w");
    if (!tmp)
    {
        DMP_ERR("fopen '%s' failed", path);
        return -1; 
    }

    dump_fd_t * fd = calloc(1, sizeof(dump_fd_t));
    if (!fd)
    {
        DMP_ERR("calloc failed");
        return -1;
    }

    fd->fd = tmp;
    strncpy(fd->desc, desc, sizeof(fd->desc));

    int idx;
    for (idx = 0; idx < DUMP_DESC_FD_MAX; idx++)
    {
        dump_fd_t * fd_in = d->fds[idx];
        if (!fd_in) break;
    }
    d->fds[idx] = fd;
    DMP_DBG("fds[%d] '%s' was added", idx, fd->desc);
    d->count++;

    return 0;
}

int dump_fd_search_by_desc(const char * desc)
{
    for (int i = 0; i < d->count; ++i)
    {
        dump_fd_t * fd = d->fds[i];
        if (!fd) continue;
        if (0 == strcmp(desc, fd->desc))
        {
            return i;
        }
    }
    DMP_DBG("fds '%s' not found", desc);
    return -1;
}

int dump_fd_remove(const char * desc)
{
    if (!desc)
    {
        DMP_ERR("desc arg is NULL");
        return -1;
    }

    int idx = dump_fd_search_by_desc(desc);
    if (0 > idx)
    {
        DMP_DBG("fds '%s' not been deleted", desc);
        return 0;
    }

    dump_fd_destroy(d->fds[idx]);
    DMP_DBG("fds '%s' was destroyed", desc);

    return 0;
}

int dump_sample_by_desc(const char * desc, double * sample)
{
    if (!desc)
    {
        DMP_ERR("desc arg is NULL");
        return -1;
    }
    if (!sample)
    {
        DMP_ERR("sample arg is NULL");
        return -1;
    }

    int idx = dump_fd_search_by_desc(desc);
    if (0 > idx)
    {
        return 0;
    }

    if (1 != fwrite(sample, sizeof(double), 1, d->fds[idx]->fd))
    {
        DMP_ERR("fwrite in '%s' failed", d->fds[idx]->desc);
        return -1;
    }

    return 0;
}



void dump_destroy()
{
    for (int idx = 0; idx < DUMP_DESC_FD_MAX; idx++)
    {
        dump_fd_t * dfd = d->fds[idx];
        if (!dfd) continue;
        DMP_DBG("fds '%s' was destroyed", dfd->desc);
        dump_fd_destroy(dfd);
    }
#ifdef DEBUG_SOURCE
    dump_dbg_source_close();
#endif /* DEBUG_SOURCE */
    safe_free(d);
    single = 0;
}