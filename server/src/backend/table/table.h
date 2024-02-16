#pragma once
#include "backend/comparator/comparator.h"
#include "backend/utils/row_likedlist.h"
#include "backend/db/db.h"
#include "backend/journal/metatab.h"
#include "table_base.h"
#include <inttypes.h>


table_t* tab_init(db_t* db, const char* name, schema_t* schema);
chblix_t tab_get_row(db_t* db,
                     table_t* table,
                     schema_t* schema,
                     field_t* field,
                     void* value,
                     datatype_t type);
void tab_print(db_t* db, table_t* table, schema_t* schema);
table_t* tab_join_on_field(
        db_t* db,
        table_t* left,
        schema_t* left_schema,
        table_t* right,
        schema_t* right_schema,
        field_t* join_field_left,
        field_t* join_field_right,
        const char* name);
table_t* tab_select_op(db_t* db,
                            table_t* sel_table,
                            schema_t* sel_schema,
                            field_t* select_field,
                            const char* name,
                            condition_t condition,
                            void* value,
                            datatype_t type);

int tab_drop(db_t* db, table_t* table);
int tab_update_row_op(db_t* db,
                           table_t* table,
                           schema_t* schema,
                           field_t* field,
                           condition_t condition,
                           void* value,
                           datatype_t type,
                           void* row);
int tab_update_element_op(db_t* db,
                          int64_t tablix,
                          void* element,
                          const char* field_name,
                          const char* field_comp,
                          condition_t condition,
                          void* value,
                          datatype_t type);
int tab_delete_op(db_t* db,
                   table_t* table,
                   schema_t* schema,
                   field_t* comp,
                   condition_t condition,
                   void* value);

table_t* tab_projection(db_t* db,
                        table_t* table,
                        schema_t* schema,
                        field_t* fields,
                        int64_t num_of_fields,
                        const char* name);

table_t* tab_join(db_t* db,
                  table_t* left,
                  schema_t* left_schema,
                  table_t* right,
                  schema_t* right_schema,
                  const char* name);


row_likedlist_t *tab_filter(db_t *db,
                            table_t *sel_table,
                            schema_t *sel_schema,
                            field_t *select_field,
                            condition_t condition,
                            void *value,
                            datatype_t type);
row_likedlist_t *rll_filter(db_t *db,
                            row_likedlist_t *rll,
                            field_t *select_field,
                            condition_t condition,
                            void *value,
                            datatype_t type);


row_likedlist_t* rll_join(db_t *db,
                          row_likedlist_t *left,
                          row_likedlist_t *right);
row_likedlist_t *
rll_projection(db_t *db, row_likedlist_t *list, field_t *fields, int64_t num_of_fields, const char *name);
row_likedlist_t *rll_filter_var(db_t *db,
                                row_likedlist_t *right_list,
                                field_t *right_field,
                                condition_t condition,
                                row_likedlist_t *left_list,
                                field_t *left_field,
                                datatype_t type);

row_likedlist_t *rll_join_or(row_likedlist_t *left,
                             row_likedlist_t *right);
row_likedlist_t *rll_join_and(row_likedlist_t *left,
                              row_likedlist_t *right);

row_likedlist_t* tab_table2rll(db_t *db, table_t *table);
table_t *tab_rll2table(db_t *db, row_likedlist_t *row_ll, const char *name);

