#include <stdlib.h>
#include <string.h>

#include "import.h"
#include "debug.h"

static int import_alloc_coef_buf(double ** ptr,
                                 size_t    size)
{
    double * ptr_in = calloc(size, sizeof(double));
    if (!ptr_in)
    {
        IMP_ERR("calloc failed");
        return -1;
    }
    *ptr = ptr_in;
    return 0;
}

static int import_parse_coefs(FILE    * fd,
                              double ** coefs,
                              size_t    size,
                              char    * type)
{
    if (0 != import_alloc_coef_buf(coefs, size))
    {
        IMP_ERR("alloc_coef_buf failed");
        return -1;
    }

    for (int i = 0; i < size; ++i)
    {
        if (IMPORT_COEF_ENTRY_OBJ != fscanf(fd, "%lf\n", &(*coefs)[i]))
        {
            IMP_ERR("coef entry has invalid format failed");
            return -1;
        }
        IMP_DBG("%s[%d]=%5.30lf\n", type, i, (*coefs)[i]);
    }

    return 0;
}

static int import_parse_entry(FILE           * fd,
                              import_entry_t * e)
{
    char type[IMPORT_TYPE_STR_LEN];
    if (IMPORT_HEADER_OBJ != fscanf(fd, "%s %s\n", type, e->desc))
    {
        IMP_ERR("header has invalid format");
        return -1;
    }

    if (0 == strcmp(IMPORT_TYPE_IIR_STR, type))
    {
        e->type = IMPORT_TYPE_IIR;
    }
    else if (0 == strcmp(IMPORT_TYPE_FIR_STR, type))
    {
        e->type = IMPORT_TYPE_FIR;
    }
    else
    {
        IMP_ERR("undefined import type");
        return -1;
    }
    IMP_DBG("import entry '%s' type '%s'", e->desc, type);

    char coef_type[IMPORT_COEF_TYPE_STR_LEN];

    if (IMPORT_TYPE_IIR == e->type)
    {
        if (IMPORT_COEF_HEADER_OBJ != fscanf(fd, "%s %u\n", coef_type, &e->a_size) &&
            0 == strcmp(IMPORT_COEF_A_STR, coef_type))
        {
            IMP_ERR("coef header has invalid format");
            return -1;
        }

        if (0 != import_parse_coefs(fd, &e->a, e->a_size, IMPORT_COEF_A_STR))
        {
            IMP_ERR("import_parse_coefs failed");
            return -1;
        }
    }

    if (IMPORT_COEF_HEADER_OBJ != fscanf(fd, "%s %u\n", coef_type, &e->b_size) &&
            0 == strcmp(IMPORT_COEF_B_STR, coef_type))
    {
        IMP_ERR("coef header has invalid format");
        return -1;
    }

    if (0 != import_parse_coefs(fd, &e->b, e->b_size, IMPORT_COEF_B_STR))
    {
        IMP_ERR("import_parse_coefs failed");
        return -1;
    }

    return 0;
}

static import_entry_t * import_alloc_entry()
{
    return calloc(1, sizeof(import_entry_t));
}

static int import_add_entry(import_t       * i,
                            import_entry_t * e)
{
    if (IMPORT_ENTRIES_MAX < i->count)
    {
        IMP_ERR("can't import more entries");
        return -1;
    }

    i->entries[i->count] = e;
    IMP_DBG("entry[%u] '%s' was added", i->count, e->desc);
    i->count++;
    return 0;
}

static void safe_free(void * p)
{
    if (p)
    {
        free(p);
        p = NULL;
    }
}

void import_destroy(import_t * i)
{
    IMP_DBG("import has %u entries, deleting", i->count);
    for (int k = 0; k < i->count; ++k)
    {
        import_entry_t * e = i->entries[k];
        if (!e) continue;
        IMP_DBG("entry[%u] '%s' deleting", i->count, e->desc);
        safe_free(e->a);
        safe_free(e->b);
        safe_free(e);
    }
    safe_free(i);
    IMP_DBG("import destroying completed");
}

import_t * import_create_from_file(const char * path)
{
    if (!path)
    {
        IMP_ERR("path arg is NULL");
        return NULL;
    }

    FILE * fd = fopen(path, "r");
    if (!fd)
    {
        printf("fopen '%s' failed\n", path);
        return NULL;
    }

    import_t * i = calloc(1, sizeof(import_t));
    if (!i)
    {
        IMP_ERR("calloc failed");
        return NULL;
    }

    while(EOF != fgetc(fd))
    {
        fseek(fd, ftell(fd) - 1, SEEK_SET);
        import_entry_t * e = import_alloc_entry();
        if (!e)
        {
            IMP_ERR("calloc failed");
            import_destroy(i);
            return NULL;
        }
        if (0 != import_parse_entry(fd, e))
        {
            IMP_ERR("import_parse_entry failed");
            import_destroy(i);
            return NULL;
        }
        if (0 != import_add_entry(i, e))
        {
            IMP_ERR("import_parse_entry failed");
            import_destroy(i);
            return NULL;
        }
    }

    if (fd)
    {
        if (0 != fclose(fd))
        {
            IMP_ERR("fclose failed");
        }
        fd = NULL;
    }

    return i;
}