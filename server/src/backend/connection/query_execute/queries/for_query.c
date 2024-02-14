#include "queries_include.h"
#include "subqueries/subqueries_include.h"
#include "backend/connection/query_execute/utils/constant_value.h"
#include "utils/hashtable.h"

void remove_rows(row_likedlist_t *rll) {
    row_node_t *current = rll->head;
    while (current != NULL) {
        row_node_t *next = current->next;
        rst_node_t *rst_current = current->rst_head;
        while (rst_current != NULL) {
            rst_node_t *rst_next = rst_current->next;
            tab_delete_row(rst_current->table, &rst_current->rowix);
            rst_current = rst_next;
        }
        current = next;
    }
}


static void process_update(default_query_args_t *args, row_likedlist_t *row_list){
    if(row_list == NULL){
        return;
    }
    struct update_ast *update_ast_ptr = (struct update_ast *)args->root;
    struct list_ast *list_head = (struct list_ast *)update_ast_ptr->list;

    while (list_head != NULL){
        struct pair_ast *pair = (struct pair_ast *)list_head->value;
        struct constant_val *constant_val = init_constant(args->db, pair->value);
        void *value_ptr = GET_VALUE_PTR(constant_val, constant_val->type);

        for (row_node_t *current = row_list->head; current != NULL; current = current->next){
            for (rst_node_t *rst_current = current->rst_head; rst_current != NULL; rst_current = rst_current->next){
                field_t field;
                if (sch_get_field(rst_current->schema, pair->key, &field) != SCHEMA_NOT_FOUND &&
                    field.type == constant_val->type){
                    tab_update_field(rst_current->table, rst_current->schema, &rst_current->rowix, &field, value_ptr);
                }
            }
        }
        list_head = (struct list_ast *)list_head->next;
    }
}



static int process_nonterminal(db_t *db, schema_t *schema, struct ast *root, struct response *resp,
                                    row_likedlist_t **filtered_list, row_likedlist_t **second_rll, hmap_t* hmap) {
    switch (root->nodetype) {
        case NT_FILTER: {
            *filtered_list = filter_exec(db, root, *filtered_list, schema, resp, *filtered_list);
            if (*filtered_list == NULL) {
                LOG_ERROR_AND_UPDATE_RESPONSE(resp, "Failed to filter");
                return -1;
            }
            if (resp->status == -1)
                return -1;
            break;
        }
        case NT_FOR: {
            *second_rll = for_stmt_exec(db, root, resp, hmap, *filtered_list);
            if (*second_rll == NULL) {
                return -1;
            }
            break;
        }
        default: {
            LOG_ERROR_AND_UPDATE_RESPONSE(resp, "Invalid type %d", root->nodetype);
            return -1;
        }
    }
    return 0;
}

static int process_terminal(db_t *db, schema_t *schema, struct ast *root, struct response *resp,
                                 row_likedlist_t *filtered_list, row_likedlist_t *second_rll, char* variable, hmap_t* hmap) {
    switch (root->nodetype) {
        case NT_RETURN: {
            struct return_ast *return_ast_ptr = (struct return_ast *) root;
            struct ast *return_value = return_ast_ptr->value;
            switch (return_value->nodetype) {
                case NT_ATTR_NAME: {
                    struct attr_name_ast *attr_name_ast_ptr = (struct attr_name_ast *) return_value;
                    char *var = attr_name_ast_ptr->variable;

                    if (strcmp(var, variable) == 0) {
                        table_t *filtered_table = tab_rll2table(db, filtered_list, "TEMP");
                        tab_print(db, filtered_table, schema);
                        set_response(resp, 0, "Selected successfully");
                        resp->table = filtered_table;

                    } else {
                        table_t *second_table = tab_rll2table(db, second_rll, "TEMP");
                        tab_print(db, second_table, second_rll->schema);
                        set_response(resp, 0, "Selected successfully");
                        resp->table = second_table;

                    }
                    break;
                }
                case NT_MERGE: {
                    if (filtered_list->schema != NULL && second_rll->schema != NULL) {
                        row_likedlist_t* join_list = second_rll;
                        table_t *join_table = tab_rll2table(db, second_rll, "TEMP");
                        tab_print(db, join_table, join_list->schema);
                        set_response(resp, 0, "Selected successfully");
                        resp->table = join_table;
                        break;
                    }
                    LOG_ERROR_AND_UPDATE_RESPONSE(resp, "Failed to join");
                    break;
                }
            }
            break;
        }
        case NT_REMOVE: {
            struct remove_ast *remove_ast_ptr = (struct remove_ast *) root;
            char *remove_table = remove_ast_ptr->tabname;
            struct attr_name_ast *attr_name_ast_ptr = (struct attr_name_ast *) remove_ast_ptr->attr;
            char *var = attr_name_ast_ptr->variable;
            row_likedlist_t* rll = ht_get(hmap, var);
            if(rll == NULL){
                LOG_ERROR_AND_UPDATE_RESPONSE(resp, "Variable not found %s", var);
                return -1;
            }
            if (strcmp(var, variable) == 0) {
                remove_rows(filtered_list);
            } else {
                remove_rows(second_rll);
            }
            set_response(resp, 0, "Removed successfully");
            break;
        }

        case NT_UPDATE: {
            default_query_args_t args = {.db = db, .root = root, .resp = resp};
            process_update(&args, filtered_list);
            set_response(resp, 0, "Updated successfully");
            break;
        }
        default: {
            LOG_ERROR_AND_UPDATE_RESPONSE(resp, "Invalid type %d", root->nodetype);
            return -1;
        }
    }
    return 0;
}

int for_exec(default_query_args_t *args) {
    struct for_ast *for_ast_ptr = (struct for_ast *) args->root;
    int64_t tabix = mtab_find_table_by_name(args->db->meta_table_idx, for_ast_ptr->tabname);
    char* variable = for_ast_ptr->var;
    if (tabix == TABLE_FAIL) {
        LOG_ERROR_AND_UPDATE_RESPONSE(args->resp, "Failed to find table %s", for_ast_ptr->tabname);
        return -1;
    }
    table_t *table = tab_load(tabix);
    if (table == NULL) {
        LOG_ERROR_AND_UPDATE_RESPONSE(args->resp, "Failed to load table %s", for_ast_ptr->tabname);
        return -1;
    }
    schema_t *schema = sch_load(table->schidx);
    hmap_t* hmap = ht_init();
    struct list_ast *temp = (struct list_ast *) for_ast_ptr->nonterm_list_head;
    reverseList(&temp);
    row_likedlist_t *filtered_list = tab_table2rll(args->db, table);
    row_likedlist_t *second_rll = NULL;
    while (temp != NULL) {
        struct list_ast *list_ast = (struct list_ast *) temp;
        if (process_nonterminal(args->db, schema, list_ast->value, args->resp, &filtered_list, &second_rll, hmap) == -1) {
            return -1;
        }
        temp = (struct list_ast *) list_ast->next;
    }
    struct ast *terminal = for_ast_ptr->terminal;
    ht_put(hmap, for_ast_ptr->var, filtered_list);
    if (process_terminal(args->db, schema, terminal, args->resp, filtered_list, second_rll, variable, hmap) == -1) {
        return -1;
    }
    if (filtered_list != NULL) row_likedlist_free(filtered_list);
    if (second_rll != NULL) row_likedlist_free(second_rll);
    return 0;
}