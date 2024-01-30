#include "queries_include.h"
#include "subqueries/subqueries_include.h"

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

int update_rows(row_likedlist_t *rll, struct list_ast *list_ast_ptr, schema_t *schema, struct response *resp) {
    struct pair_ast *pair_ast = (struct pair_ast *) list_ast_ptr->value;
    char *name = pair_ast->key;
    field_t field;
    if (sch_get_field(schema, name, &field) == SCHEMA_NOT_FOUND) {
        return log_error_and_update_response(resp, "Field not found %s", name);
    }
    struct ast *value = pair_ast->value;
    struct constant_val *constant_val = get_constant(value);
    if (constant_val == NULL) {
        return log_error_and_update_response(resp, "Failed to get constant");
    }
    void *value_ptr;
    switch (constant_val->type) {
        case DT_INT:
            value_ptr = &constant_val->int_val;
            break;
        case DT_FLOAT:
            value_ptr = &constant_val->float_val;
            break;
        case DT_BOOL:
            value_ptr = &constant_val->bool_val;
            break;
        case DT_VARCHAR:
            value_ptr = &constant_val->string_val;
            break;
        default:
            log_error_and_update_response(resp, "Invalid type %d", constant_val->type);
            free(constant_val);
            return -1;
    }
    if (field.type != constant_val->type) {
        log_error_and_update_response(resp, "Invalid field type %d", constant_val->type);
        free(constant_val);
        return -1;
    }
    row_node_t *current = rll->head;
    while (current != NULL) {
        row_node_t *next = current->next;
        rst_node_t *rst_current = current->rst_head;
        while (rst_current != NULL) {
            rst_node_t *rst_next = rst_current->next;
            tab_update_row(rst_current->table, &rst_current->rowix, &field, value_ptr);
            rst_current = rst_next;
        }
        current = next;
    }
    free(constant_val);
    return 0;
}



row_likedlist_t *for_stmt(db_t *db, struct ast *root, struct response *resp) {
    struct for_ast *for_ast_ptr = (struct for_ast *) root;
    int64_t tabix = mtab_find_table_by_name(db->meta_table_idx, for_ast_ptr->tabname);
    if (tabix == TABLE_FAIL) {
        log_error_and_update_response(resp, "Failed to find table %s", for_ast_ptr->tabname);
        return NULL;
    }
    table_t *table = tab_load(tabix);
    if (table == NULL) {
        log_error_and_update_response(resp, "Failed to load table %s", for_ast_ptr->tabname);
        return NULL;
    }
    schema_t *schema = sch_load(table->schidx);
    struct list_ast *temp = (struct list_ast *) for_ast_ptr->nonterm_list_head;
    reverseList(&temp);
    row_likedlist_t *filtered_list = tab_table2rll(db, table);
    while (temp != NULL) {
        struct list_ast *list_ast = (struct list_ast *) temp;
        switch (list_ast->value->nodetype) {
            case NT_FILTER: {
                filtered_list = filter_exec(db, list_ast->value, filtered_list, schema, resp);
                if (filtered_list == NULL) {
                    log_error_and_update_response(resp, "Failed to filter");
                    return NULL;
                }
                if (resp->status == -1)
                    return NULL;
                break;
            }
            default: {
                log_error_and_update_response(resp, "Invalid type %d", list_ast->value->nodetype);
                return NULL;
            }
        }
        temp = (struct list_ast *) list_ast->next;
    }
    return filtered_list;
}


static int process_nonterminal_list(db_t *db, schema_t *schema, struct list_ast *root, struct response *resp,
                             row_likedlist_t **filtered_list, row_likedlist_t **second_rll) {
    switch (root->nodetype) {
        case NT_FILTER: {
            *filtered_list = filter_exec(db, root->value, *filtered_list, schema, resp);
            if (*filtered_list == NULL) {
                return log_error_and_update_response(resp, "Failed to filter");
            }
            if (resp->status == -1)
                return -1;
            break;
        }
        case NT_FOR: {
            *second_rll = for_stmt(db, root->value, resp);
            if (*second_rll == NULL) {
                return -1;
            }
            break;
        }
        default: {
            return log_error_and_update_response(resp, "Invalid type %d", root->value->nodetype);
        }
    }
    return 0;
}

static int process_terminal_list(db_t *db, schema_t *schema, struct list_ast *root, struct response *resp,
                          row_likedlist_t *filtered_list, row_likedlist_t *second_rll) {
    switch (root->nodetype) {
        case NT_RETURN: {
            struct return_ast *return_ast_ptr = (struct return_ast *) root->value;
            struct ast *return_value = return_ast_ptr->value;
            switch (return_value->nodetype) {
                case NT_ATTR_NAME: {
                    struct attr_name_ast *attr_name_ast_ptr = (struct attr_name_ast *) return_value;
                    char *var = attr_name_ast_ptr->variable;

                    if (strcmp(var, "TEMP") == 0) {
                        table_t *filtered_table = tab_rll2table(db, filtered_list, "TEMP");
                        tab_print(db, filtered_table, schema);
                        resp->status = 0;
                        resp->message = strdup("Selected successfully");
                        resp->table = filtered_table;

                    } else {
                        table_t *second_table = tab_rll2table(db, second_rll, "TEMP");
                        tab_print(db, second_table, second_rll->schema);
                        resp->status = 0;
                        resp->message = strdup("Selected successfully");
                        resp->table = second_table;

                    }
                    break;
                }
                case NT_MERGE: {
                    if (filtered_list->schema == NULL || second_rll->schema == NULL)
                        rll_join(db, filtered_list, second_rll);
                    table_t *restab = tab_rll2table(db, filtered_list, "TEMP");
                    tab_print(db, restab, filtered_list->schema);
                    resp->status = 0;
                    resp->message = strdup("Selected successfully");
                    resp->table = restab;
                    break;
                }
            }
            break;
        }
        case NT_REMOVE: {
            struct remove_ast *remove_ast_ptr = (struct remove_ast *) root->value;
            char *remove_table = remove_ast_ptr->tabname;
            struct attr_name_ast *attr_name_ast_ptr = (struct attr_name_ast *) remove_ast_ptr->attr;
            char *var = attr_name_ast_ptr->variable;
            if (strcmp(var, "TEMP") == 0) {
                remove_rows(filtered_list);
            } else {
                remove_rows(second_rll);
            }
            resp->status = 0;
            resp->message = strdup("Removed successfully");
            break;
        }

        case NT_UPDATE: {
            struct update_ast *update_ast_ptr = (struct update_ast *) root->value;
            char *update_table = update_ast_ptr->tabname;
            struct attr_name_ast *attr_name_ast_ptr = (struct attr_name_ast *) update_ast_ptr->attr;
            struct list_ast *list_ast_ptr = (struct list_ast *) update_ast_ptr->list;
            if (strcmp(attr_name_ast_ptr->variable, "TEMP") == 0) {
                if (update_rows(filtered_list, list_ast_ptr, schema, resp) == -1) {
                    return -1;
                }
            } else {
                if (update_rows(second_rll, list_ast_ptr, schema, resp) == -1) {
                    return -1;
                }
            }
            break;
        }
        default: {
            return log_error_and_update_response(resp, "Invalid type %d", root->value->nodetype);
        }
    }
    return 0;
}

int for_exec(default_query_args_t *args) {
    struct for_ast *for_ast_ptr = (struct for_ast *) args->root;
    int64_t tabix = mtab_find_table_by_name(args->db->meta_table_idx, for_ast_ptr->tabname);
    if (tabix == TABLE_FAIL) {
        return log_error_and_update_response(args->resp, "Failed to find table %s", for_ast_ptr->tabname);
    }
    table_t *table = tab_load(tabix);
    if (table == NULL) {
        return log_error_and_update_response(args->resp, "Failed to load table %s", for_ast_ptr->tabname);
    }
    schema_t *schema = sch_load(table->schidx);
    struct list_ast *temp = (struct list_ast *) for_ast_ptr->nonterm_list_head;
    reverseList(&temp);
    row_likedlist_t *filtered_list = tab_table2rll(args->db, table);
    row_likedlist_t *second_rll = NULL;
    while (temp != NULL) {
        struct list_ast *list_ast = (struct list_ast *) temp;
        if(process_nonterminal_list(args->db, schema, list_ast, args->resp, &filtered_list, &second_rll) == -1){
            return -1;
        }
        temp = (struct list_ast *) list_ast->next;
    }
    struct ast *terminal = for_ast_ptr->terminal;
    if(process_terminal_list(args->db, schema, (struct list_ast *) terminal, args->resp, filtered_list, second_rll) == -1){
        return -1;
    }
    if (filtered_list != NULL) row_likedlist_free(filtered_list);
    if (second_rll != NULL) row_likedlist_free(second_rll);
    return 0;
}