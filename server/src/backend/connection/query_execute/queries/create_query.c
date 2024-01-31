#include "queries_include.h"
#include "utils/utils.h"

int create_exec(default_query_args_t* args) {
    struct create_ast *create_ast = (struct create_ast *) args->root;
    schema_t *schema = sch_init();
    if (schema == NULL) {
        LOG_ERROR_AND_UPDATE_RESPONSE(args->resp, "Failed to create schema");
        return -1;
    }
    struct list_ast *temp = (struct list_ast *) create_ast->difinitions;
    reverseList(&temp);
    while (temp != NULL) {
        struct list_ast *list_ast = (struct list_ast *) temp;
        struct create_pair_ast *pair_ast = (struct create_pair_ast *) list_ast->value;
        char *name = pair_ast->name;
        switch (pair_ast->type) {
            case NT_STRING:
                sch_add_varchar_field(schema, name);
                break;
            case NT_INTEGER:
                sch_add_int_field(schema, name);
                break;
            case NT_FLOAT:
                sch_add_float_field(schema, name);
                break;
            case NT_BOOLEAN:
                sch_add_bool_field(schema, name);
                break;
            default:
                logger(LL_ERROR, __func__, "Invalid type %d", pair_ast->type);
                return -1;
        }
        temp = (struct list_ast *) list_ast->next;
    }
    table_t *table = tab_init(args->db, create_ast->name, schema);
    if (table == NULL) {
        LOG_ERROR_AND_UPDATE_RESPONSE(args->resp, "Failed to create schema");
        return -1;
    }
    args->resp->status = 0;
    args->resp->message = strdupf("Table %s created successfully", create_ast->name);
    return 0;
}