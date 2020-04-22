#ifndef DUMP_H
#define DUMP_H

#include <stdio.h>

#define DUMP_DBG_SOURCE      "data-source"
#define DUMP_DBG_SOURCE_DESC "dbg-source"

#define DUMP_DESC_MAX_LEN (32)
#define DUMP_DESC_FD_MAX  (64)

typedef struct
{
    FILE * fd;
    char desc[DUMP_DESC_MAX_LEN];
} dump_fd_t;

typedef struct
{
#ifdef DEBUG_SOURCE
    dump_fd_t * source_fd;
#endif /* DEBUG_SOURCE */
    dump_fd_t * fds[DUMP_DESC_FD_MAX];
    size_t count;
} dump_t;

#ifdef DEBUG_SOURCE
int dump_dbg_source_get_sample(double * sample);
#endif /* DEBUG_SOURCE */
int dump_create();
int dump_fd_add(const char * desc, const char * path);
int dump_sample_by_desc(const char * desc, double sample);
int dump_fd_remove(const char * desc);
void dump_destroy();

#endif /* DUMP_H */