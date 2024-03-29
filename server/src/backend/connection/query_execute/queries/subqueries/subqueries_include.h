#ifndef SUBQUERIES_INCLUDE_H
#define SUBQUERIES_INCLUDE_H

#include "backend/connection/query_execute/utils/exe_utils.h"
#include "utils/hashtable.h"

row_likedlist_t *filter_exec(db_t *db, struct ast *root, row_likedlist_t *rll, schema_t *schema, struct response *resp,  row_likedlist_t* list_1);
row_likedlist_t *for_stmt_exec(db_t *db, struct ast *root, struct response *resp, hmap_t* hmap, row_likedlist_t* list_1);
#endif