#pragma once
#include "parray.h"

typedef struct parray64{
    parray_t parray;
    int64_t inval;
} parray64_t;

int64_t pa_init64(int64_t block_size, int64_t inval);
int pa_write64(parray64_t* parray, int64_t block_idx, int64_t value);
int pa_read64(parray64_t* parray64, int64_t block_idx, int64_t* dest);
int pa_delete64(int64_t page_index, int64_t block_idx);
int pa_append64(int64_t paidx, int64_t value);
int pa_pop64(int64_t paidx, int64_t* dest);
int64_t pa_find_first_int64(int64_t paidx, int64_t value);
int pa_exists64(int64_t paidx, int64_t value);
int pa_push_unique64(int64_t paidx, int64_t value);
int pa_delete_unique64(int64_t paidx, int64_t value);