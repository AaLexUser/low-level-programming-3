#ifndef SUBQUERIES_INCLUDE_H
#define SUBQUERIES_INCLUDE_H

#include "backend/connection/query_execute/utils/exe_utils.h"

row_likedlist_t *filter_exec(db_t *db, struct ast *root, row_likedlist_t *rll, schema_t *schema, struct response *resp);
row_likedlist_t *for_stmt_exec(db_t *db, struct ast *root, struct response *resp);
#endif