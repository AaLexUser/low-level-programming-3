#ifndef QUERIES_INCLUDE_H
#define QUERIES_INCLUDE_H

#include "backend/db/db.h"
#include "../exe_utils.h"

typedef struct default_query_args {
    db_t *db;
    struct ast *root;
    struct response *resp;
} default_query_args_t;

int for_exec(default_query_args_t *args);
int drop_exec(default_query_args_t* args);
int insert_exec(default_query_args_t* args);
int create_exec(default_query_args_t* args);

#endif