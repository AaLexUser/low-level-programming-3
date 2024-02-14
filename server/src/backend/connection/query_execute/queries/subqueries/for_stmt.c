#include "subqueries_include.h"
#include "utils/hashtable.h"

row_likedlist_t *for_stmt_exec(db_t *db, struct ast *root, struct response *resp, hmap_t* hmap, row_likedlist_t* list_1) {
    struct for_ast *for_ast_ptr = (struct for_ast *) root;
    int64_t tabix = mtab_find_table_by_name(db->meta_table_idx, for_ast_ptr->tabname);
    if (tabix == TABLE_FAIL) {
        LOG_ERROR_AND_UPDATE_RESPONSE(resp, "Failed to find table %s", for_ast_ptr->tabname);
        return NULL;
    }
    table_t *table = tab_load(tabix);
    if (table == NULL) {
        LOG_ERROR_AND_UPDATE_RESPONSE(resp, "Failed to load table %s", for_ast_ptr->tabname);
        return NULL;
    }
    schema_t *schema = sch_load(table->schidx);
    struct list_ast *temp = (struct list_ast *) for_ast_ptr->nonterm_list_head;
    reverseList(&temp);
    row_likedlist_t *filtered_list = tab_table2rll(db, table);
    char* variable = for_ast_ptr->var;
    while (temp != NULL) {
        struct list_ast *list_ast = (struct list_ast *) temp;
        switch (list_ast->value->nodetype) {
            case NT_FILTER: {
                filtered_list = filter_exec(db, list_ast->value, filtered_list, schema, resp, list_1);
                if (filtered_list == NULL) {
                    LOG_ERROR_AND_UPDATE_RESPONSE(resp, "Failed to filter");
                    return NULL;
                }
                if (resp->status == -1)
                    return NULL;
                break;
            }
            default: {
                LOG_ERROR_AND_UPDATE_RESPONSE(resp, "Invalid type %d", list_ast->value->nodetype);
                return NULL;
            }
        }
        temp = (struct list_ast *) list_ast->next;
    }
    ht_put(hmap, variable, filtered_list);
    return filtered_list;
}