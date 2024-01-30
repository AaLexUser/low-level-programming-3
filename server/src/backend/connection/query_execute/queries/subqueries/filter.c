#include "subqueries_include.h"

static void simple_condition(db_t *db,
                      struct filter_condition_ast *root,
                      row_likedlist_t *rll,
                      schema_t *schema,
                      struct response *resp) {
    struct filter_expr_ast *filter_expr_ptr = (struct filter_expr_ast *) root->l;
    struct attr_name_ast *attr_ptr = (struct attr_name_ast *) filter_expr_ptr->attr_name;
    field_t sel_field;

    if (sch_get_field(schema, attr_ptr->attr_name, &sel_field) == SCHEMA_NOT_FOUND) {
        logger(LL_ERROR, __func__, "Field not found %s", attr_ptr->attr_name);
        return;
    }

    condition_t condition = get_condition_type(filter_expr_ptr->cmp);
    struct constant_val *constant_val = get_constant(filter_expr_ptr->constant);

    if (constant_val == NULL) {
        logger(LL_ERROR, __func__, "Failed to get constant");
        return;
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
            return;
    }
    if (sel_field.type != constant_val->type) {
        log_error_and_update_response(resp, "Invalid field type %d", constant_val->type);
        free(constant_val);
        return;
    }

    rll_filter(db, rll, &sel_field, condition, value_ptr, constant_val->type);
    free(constant_val);
}

row_likedlist_t *filter_exec(db_t *db, struct ast *root, row_likedlist_t *rll, schema_t *schema, struct response *resp) {
    row_likedlist_t *result_list = rll;
    struct filter_ast *filter_ast_ptr = (struct filter_ast *) root;
    struct filter_condition_ast *condition_ast_ptr = (struct filter_condition_ast *) filter_ast_ptr->conditions_tree_root;
    if (condition_ast_ptr->r != NULL && condition_ast_ptr->logic != -1) {

    } else {
        simple_condition(db, condition_ast_ptr, rll, schema, resp);
    }
    return result_list;
}