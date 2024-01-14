#ifndef REQEXE_H
#define REQEXE_H

#include "ast.h"
#include "backend/db/db.h"

struct response {
    int status;
    char* message;
    table_t* table;
};

struct response* reqexe(db_t *db, struct ast *root);

#endif