#include "queries_include.h"

int insert_exec(default_query_args_t* args){
    struct insert_ast *insert_ast_ptr = (struct insert_ast *) args->root;
    int64_t tabix = mtab_find_table_by_name(args->db->meta_table_idx, insert_ast_ptr->tabname);
    if (tabix == TABLE_FAIL) {
        return log_error_and_update_response(args->resp, "Failed to find table %s", insert_ast_ptr->tabname);
    }
    table_t *table = tab_load(tabix);
    if (table == NULL) {
        return log_error_and_update_response(args->resp, "Failed to load table %s", insert_ast_ptr->tabname);
    }
    struct ast *temp = insert_ast_ptr->list;
    schema_t *schema = sch_load(table->schidx);
    uint8_t *row = malloc(schema->slot_size);
    if (row == NULL) {
        return log_error_and_update_response(args->resp, "Failed to allocate memory");
    }
    memset(row, 0, schema->slot_size);
    struct list_ast *head = (struct list_ast *) temp;
    reverseList(&head);
    sch_for_each(schema, chunk, fieldi, chblix, schema_index(schema)) {
        struct list_ast *current = (struct list_ast *) head;
        struct pair_ast *pair_ast = (struct pair_ast *) current->value;
        char *name = pair_ast->key;
        while (strcmp(name, fieldi.name) != 0) {
            current = (struct list_ast *) current->next;
            pair_ast = (struct pair_ast *) current->value;
            name = pair_ast->key;
        }
        switch (fieldi.type) {
            case DT_INT: {
                if (pair_ast->value->nodetype != NT_INTVAL) {
                    return log_error_and_update_response(args->resp, "Invalid type %d", pair_ast->value->nodetype);
                }
                struct nint *integer_val = (struct nint *) pair_ast->value;
                int64_t int_val = (int64_t) integer_val->value;
                memcpy(row + fieldi.offset, &int_val, fieldi.size);
                break;
            }
            case DT_FLOAT: {
                if (pair_ast->value->nodetype != NT_FLOATVAL) {
                    return log_error_and_update_response(args->resp, "Invalid type %d", pair_ast->value->nodetype);
                }
                struct nfloat *float_val = (struct nfloat *) pair_ast->value;
                double double_val = (double) float_val->value;
                memcpy(row + fieldi.offset, &double_val, fieldi.size);
                break;
            }
            case DT_BOOL: {
                if (pair_ast->value->nodetype != NT_BOOLVAL) {
                    return log_error_and_update_response(args->resp, "Invalid type %d", pair_ast->value->nodetype);
                }
                struct nint *bool_val = (struct nint *) pair_ast->value;
                bool boolval = (bool) bool_val->value;
                memcpy(row + fieldi.offset, &(boolval), fieldi.size);
                break;
            }
            case DT_VARCHAR: {
                if (pair_ast->value->nodetype != NT_STRINGVAL) {
                    return log_error_and_update_response(args->resp, "Invalid type %d", pair_ast->value->nodetype);
                }
                struct nstring *string_val = (struct nstring *) pair_ast->value;
                vch_ticket_t ticket = vch_add(args->db->varchar_mgr_idx, string_val->value);
                memcpy(row + fieldi.offset, &ticket, fieldi.size);
                break;
            }
            default: {
                return log_error_and_update_response(args->resp, "Invalid type %d", fieldi.type);
            }

        }
    }
    tab_insert(table, schema, row);
    args->resp->status = 0;
    args->resp->message = strdup("Row inserted successfully");
    free(row);
    return 0;
}