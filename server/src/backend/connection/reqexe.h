#ifndef REQEXE_H
#define REQEXE_H

#include "ast.h"
#include "backend/db/db.h"

struct response {
    int status;
    char* message;
    table_t* table;
};

int reqexe(db_t *db, struct ast *root, struct response* resp);

#endif