#ifndef REQEXE_H
#define REQEXE_H

#include "backend/connection/ast.h"
#include "backend/db/db.h"
#include "backend/connection/query_execute/exe_utils.h"

int reqexe(db_t *db, struct ast *root, struct response* resp);

#endif