#pragma once
#include "backend/table/table_base.h"
#include <stdint.h>
#include <stdlib.h>


#if defined(__linux__)
#include <bits/stdint-intn.h>
#endif

typedef struct matertab{
    table_t table;
    int64_t next_index;
} matertab_t;

int64_t materializer_init(void);
table_t* materializer_materialize(int64_t mater_idx, schema_t* input_schema);

