#include "reqexe.h"
#include "utils/utils.h"
#include "exe_utils.h"
#include "backend/connection/query_execute/queries/queries_include.h"

int reqexe(db_t *db, struct ast *root, struct response *resp) {
    if (!root) {
        return log_error_and_update_response(resp, "Root is NULL");
    }
    default_query_args_t args = {.db = db, .root = root, .resp = resp};
    switch (root->nodetype) {
        case NT_CREATE: {
            create_exec(&args);
            break;
        }
        case NT_INSERT: {
            insert_exec(&args);
            break;
        }
        case NT_FOR: {
            for_exec(&args);
            break;
        }
        case NT_DROP: {
            drop_exec(&args);
            break;
        }
        default: {
            return log_error_and_update_response(resp, "Invalid root type %d", root->nodetype);
        }
    }
    return 0;
}
