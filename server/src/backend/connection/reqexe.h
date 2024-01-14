#ifndef REQEXE_H
#define REQEXE_H

#include "ast.h"
#include "backend/db/db.h"

int reqexe(db_t *db, struct ast *root);

#endif