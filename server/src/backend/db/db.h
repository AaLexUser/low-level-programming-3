#pragma once

#include "backend/journal/metatab.h"
#include "core/io/pager.h"

typedef struct db{
    int64_t meta_table_idx;
    int64_t varchar_mgr_idx;
} db_t;

void* db_init(const char* filename);
int db_close(void);
int db_drop(void);

enum dbsts_t {DB_SUCCESS = 0, DB_FAIL = -1};
