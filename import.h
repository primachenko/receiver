#ifndef IMPORT_H
#define IMPORT_H

#define IMPORT_COEF_A_STR "a"
#define IMPORT_COEF_B_STR "b"
#define IMPORT_TYPE_IIR_STR "iir"
#define IMPORT_TYPE_FIR_STR "fir"
#define IMPORT_HEADER_OBJ (2)
#define IMPORT_COEF_HEADER_OBJ (2)
#define IMPORT_COEF_ENTRY_OBJ (1)
#define IMPORT_TYPE_STR_LEN (3)
#define IMPORT_COEF_TYPE_STR_LEN (1)
#define IMPORT_DESC_MAX_LEN (32)
#define IMPORT_ENTRIES_MAX (64)

typedef enum
{
    IMPORT_TYPE_IIR,
    IMPORT_TYPE_FIR
} import_entry_type_e;

typedef struct
{
    char                desc[IMPORT_DESC_MAX_LEN];
    double            * a;
    size_t              a_size;
    double            * b;
    size_t              b_size;
    import_entry_type_e type;
} import_entry_t;

typedef struct
{
    import_entry_t * entries[IMPORT_ENTRIES_MAX];
    size_t           count;
} import_t;

import_t * import_create_from_file(const char * path);

void import_destroy(import_t * i);

#endif /* IMPORT_H */