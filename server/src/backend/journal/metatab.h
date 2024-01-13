#pragma once

#include "backend/table/table_base.h"

table_t* mtab_init(void);
int64_t mtab_find_table_by_name(int64_t metatab_idx, const char* name);
int mtab_add(int64_t metatab_idx, const char* name, int64_t index);
int mtab_delete(int64_t metatab_idx, int64_t index);
