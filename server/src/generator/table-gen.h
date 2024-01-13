#pragma once
#include "backend/journal/varchar_mgr.h"
#include "backend/table/table.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct gentab_mgr{
    int64_t next_index;
}gentab_mgr;

table_t* gen_table(db_t* db, gentab_mgr* gentabMgr_ptr, int64_t min_number_of_rows);
table_t* gen_empty_table(db_t* db, gentab_mgr* gentabMgr_ptr, schema_t* schema);
int gen_fill_element(db_t* db, field_t* field, uint8_t* element);
int gen_fill_row(db_t* db, schema_t* schema, uint8_t* row);
int gen_fill_rows(db_t* db, table_t* table, schema_t* schema, int64_t number);