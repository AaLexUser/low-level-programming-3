#include "subqueries_include.h"
#include "backend/connection/query_execute/utils/constant_value.h"

static row_likedlist_t *simple_condition(db_t *db,
                                         struct filter_expr_ast *root,
                                         row_likedlist_t *rll,
                                         schema_t *schema,
                                         struct response *resp,
                                         row_likedlist_t* list_1) {
    struct attr_name_ast *attr_ptr = (struct attr_name_ast *) root->attr_name;
    field_t sel_field;

    if (sch_get_field(schema, attr_ptr->attr_name, &sel_field) == SCHEMA_NOT_FOUND) {
        LOG_ERROR_AND_UPDATE_RESPONSE(resp, "Field not found %s", attr_ptr->attr_name);
        return NULL;
    }

    condition_t condition = get_condition_type(root->cmp);
    if(root->constant->nodetype == NT_ATTR_NAME){
        struct attr_name_ast *attr_ptr = (struct attr_name_ast *) root->constant;
        field_t sel_field_2;
        if (sch_get_field(list_1->schema, attr_ptr->attr_name, &sel_field_2) == SCHEMA_NOT_FOUND) {
            LOG_ERROR_AND_UPDATE_RESPONSE(resp, "Field not found %s", attr_ptr->attr_name);
            return NULL;
        }
        row_likedlist_t *filtered_rll = rll_filter_var(db, rll, &sel_field, condition, list_1, &sel_field_2,sel_field_2.type);
        return filtered_rll;
    }
    struct constant_val *constant_val = init_constant(db, root->constant);

    if (constant_val == NULL) {
        logger(LL_ERROR, __func__, "Failed to get constant");
        return NULL;
    }

    void *value_ptr;
    value_ptr = GET_VALUE_PTR(constant_val, constant_val->type);
    if (sel_field.type != constant_val->type) {
        LOG_ERROR_AND_UPDATE_RESPONSE(resp, "Invalid field type %d", constant_val->type);
        free(constant_val);
        return NULL;
    }

    row_likedlist_t *filtered_rll = rll_filter(db, rll, &sel_field, condition, value_ptr, constant_val->type);
    free(constant_val);
    return filtered_rll;
}

static row_likedlist_t *complex_condition(db_t *db,
                                          struct filter_condition_ast *root,
                                          row_likedlist_t *rll,
                                          schema_t *schema,
                                          struct response *resp,
                                          row_likedlist_t* list_1) {
    row_likedlist_t *result_list = NULL;
    if (root->r != NULL && ((struct filter_condition_ast *) root)->logic != -1) {
        struct filter_expr_ast *left = (struct filter_expr_ast *) root->l;
        struct filter_condition_ast *right = (struct filter_condition_ast *) root->r;
        row_likedlist_t *left_filtered = simple_condition(db, left, rll, schema, resp, list_1);
        row_likedlist_t *right_filtered = complex_condition(db, right, rll, schema, resp, list_1);
        if (root->logic == NT_AND) {
            result_list = rll_join_and(left_filtered, right_filtered);
        } else if (root->logic == NT_OR) {
            result_list = rll_join_or(left_filtered, right_filtered);
        }
        row_likedlist_free(left_filtered);
        row_likedlist_free(right_filtered);
        return result_list;
    } else {
        struct filter_expr_ast *left = (struct filter_expr_ast *) root->l;
        result_list = simple_condition(db, left, rll, schema, resp, list_1);
        return result_list;
    }
}

row_likedlist_t *
filter_exec(db_t *db, struct ast *root, row_likedlist_t *rll, schema_t *schema, struct response *resp, row_likedlist_t* list_1) {
    row_likedlist_t *result_list = rll;
    struct filter_ast *filter_ast_ptr = (struct filter_ast *) root;
    struct filter_condition_ast *condition_ast_ptr = (struct filter_condition_ast *) filter_ast_ptr->conditions_tree_root;
    if (condition_ast_ptr->r != NULL && condition_ast_ptr->logic != -1) {
        result_list = complex_condition(db, condition_ast_ptr, rll, schema, resp, list_1);
    } else {
        result_list = simple_condition(db, (struct filter_expr_ast *) condition_ast_ptr->l, rll, schema, resp, list_1);
    }
    row_likedlist_free(rll);
    return result_list;
}