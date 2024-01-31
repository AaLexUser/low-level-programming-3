#include "reqexe.h"
#include "utils/utils.h"
#include "backend/connection/query_execute/utils/exe_utils.h"
#include "backend/connection/query_execute/queries/queries_include.h"

int reqexe(db_t *db, struct ast *root, struct response *resp) {
    if (!root) {
        LOG_ERROR_AND_UPDATE_RESPONSE(resp, "Root is NULL");
        return -1;
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
            LOG_ERROR_AND_UPDATE_RESPONSE(resp, "Invalid root type %d", root->nodetype);
            return -1;
        }
    }
    return 0;
}
