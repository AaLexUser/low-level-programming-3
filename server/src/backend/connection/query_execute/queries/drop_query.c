#include "queries_include.h"
#include "utils/utils.h"

int drop_exec(default_query_args_t* args){
    struct drop_ast *drop_ast_ptr = (struct drop_ast *) args->root;
    int64_t tabix = mtab_find_table_by_name(args->db->meta_table_idx, drop_ast_ptr->name);
    if (tabix == -1) {
        LOG_ERROR_AND_UPDATE_RESPONSE(args->resp, "Failed to find table %s", drop_ast_ptr->name);
        return -1;
    }
    table_t *table = tab_load(tabix);
    if (table == NULL) {
        LOG_ERROR_AND_UPDATE_RESPONSE(args->resp, "Failed to load table %s", drop_ast_ptr->name);
        return -1;
    }
    if (tab_drop(args->db, table) == -1) {
        LOG_ERROR_AND_UPDATE_RESPONSE(args->resp, "Failed to drop table %s", drop_ast_ptr->name);
        return -1;
    }
    args->resp->status = 0;
    args->resp->message = strdupf("Table %s dropped successfully", drop_ast_ptr->name);
    return 0;
}