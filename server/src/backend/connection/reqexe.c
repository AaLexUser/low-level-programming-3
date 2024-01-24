#include <stdarg.h>
#include "reqexe.h"
#include "backend/db/db.h"
#include "backend/table/table.h"

struct constant_val {
    datatype_t type;
    union {
        int64_t int_val;
        double float_val;
        bool bool_val;
        char *string_val;
    };
};

char *strdupf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int length = vsnprintf(NULL, 0, format, args);
    va_end(args);
    char *str = malloc(length + 1);
    va_start(args, format);
    vsprintf(str, format, args);
    va_end(args);
    return str;
}

void reverseList(struct list_ast **head_ref) {
    struct list_ast *prev = NULL;
    struct list_ast *current = *head_ref;
    struct list_ast *next = NULL;

    while (current != NULL) {
        next = (struct list_ast *) current->next;
        current->next = (struct ast *) prev;
        prev = current;
        current = next;
    }

    *head_ref = prev;
}

condition_t get_condition(int cmp) {
    condition_t condition = -1;
    switch (cmp) {
        case NT_EQ:
            condition = COND_EQ;
            break;
        case NT_NEQ:
            condition = COND_NEQ;
            break;
        case NT_LT:
            condition = COND_LT;
            break;
        case NT_LTE:
            condition = COND_LTE;
            break;
        case NT_GT:
            condition = COND_GT;
            break;
        case NT_GTE:
            condition = COND_GTE;
            break;
//        case NT_IN:
//            condition = COND_IN;
//            break;
        default:
            logger(LL_ERROR, __func__, "Invalid condition %d", cmp);
            break;
    }
    return condition;
}

datatype_t get_type(int type) {
    datatype_t datatype = -1;
    switch (type) {
        case NT_INTVAL:
            datatype = DT_INT;
            break;
        case NT_FLOATVAL:
            datatype = DT_FLOAT;
            break;
        case NT_BOOLVAL:
            datatype = DT_BOOL;
            break;
        case NT_STRINGVAL:
            datatype = DT_VARCHAR;
            break;
        default:
            logger(LL_ERROR, __func__, "Invalid type %d", type);
            break;
    }
    return datatype;
}


struct constant_val *get_constant(struct ast *constant) {
    struct constant_val *constant_val = malloc(sizeof(struct constant_val));
    if (constant->nodetype == NT_INTVAL) {
        struct nint *int_val = (struct nint *) constant;
        constant_val->type = DT_INT;
        constant_val->int_val = int_val->value;
    } else if (constant->nodetype == NT_FLOATVAL) {
        struct nfloat *float_val = (struct nfloat *) constant;
        constant_val->type = DT_FLOAT;
        constant_val->float_val = float_val->value;
    } else if (constant->nodetype == NT_BOOLVAL) {
        struct nint *bool_val = (struct nint *) constant;
        constant_val->type = DT_BOOL;
        constant_val->bool_val = bool_val->value;
    } else if (constant->nodetype == NT_STRINGVAL) {
        struct nstring *string_val = (struct nstring *) constant;
        constant_val->type = DT_VARCHAR;
        constant_val->string_val = string_val->value;
    } else {
        logger(LL_ERROR, __func__, "Invalid constant type %d", constant->nodetype);
        free(constant_val);
        return NULL;
    }
    return constant_val;
}

void simple_condition(db_t *db,
                      struct filter_condition_ast *root,
                      row_likedlist_t* rll,
                      schema_t *schema,
                      struct response *resp) {
    struct filter_expr_ast *filter_expr_ptr = (struct filter_expr_ast *) root->l;
    struct attr_name_ast *attr_ptr = (struct attr_name_ast *) filter_expr_ptr->attr_name;
    field_t sel_field;

    if (sch_get_field(schema, attr_ptr->attr_name, &sel_field) == SCHEMA_NOT_FOUND) {
        logger(LL_ERROR, __func__, "Field not found %s", attr_ptr->attr_name);
        return;
    }

    condition_t condition = get_condition(filter_expr_ptr->cmp);
    struct constant_val *constant_val = get_constant(filter_expr_ptr->constant);

    if (constant_val == NULL) {
        logger(LL_ERROR, __func__, "Failed to get constant");
        return;
    }

    void *value_ptr;
    switch (constant_val->type) {
        case DT_INT: value_ptr = &constant_val->int_val; break;
        case DT_FLOAT: value_ptr = &constant_val->float_val; break;
        case DT_BOOL: value_ptr = &constant_val->bool_val; break;
        case DT_VARCHAR: value_ptr = &constant_val->string_val; break;
        default:
            logger(LL_ERROR, __func__, "Invalid type %d", constant_val->type);
            resp->status = -1;
            resp->message = strdupf("Invalid type %d", constant_val->type);
            free(constant_val);
            return;
    }
    if(sel_field.type != constant_val->type){
        logger(LL_ERROR, __func__, "Invalid field type %d", constant_val->type);
        resp->status = -1;
        resp->message = strdupf("Invalid field type %d", filter_expr_ptr->constant->nodetype);
        free(constant_val);
        return;
    }

    rll_filter(db, rll, &sel_field, condition, value_ptr, constant_val->type);
    free(constant_val);
}

//table_t *complex_condition(db_t *db, struct filter_condition_ast *root, table_t *table, schema_t *schema) {
//    struct filter_condition_ast *condition_ast_ptr = (struct filter_condition_ast *) root;
//    table_t *result_table = NULL;
//    if (condition_ast_ptr->r != NULL && condition_ast_ptr->logic != -1) {
//        table_t *right_table = complex_condition(db, condition_ast_ptr, table, schema);
//        table_t *left_table = complex_condition(db, condition_ast_ptr, table, schema);
//        if (left_table == NULL || right_table == NULL) {
//            logger(LL_ERROR, __func__, "Failed to filter");
//            return NULL;
//        }
//        result_table = tab_join_on_field(db, left_table, right_table, schema, condition_ast_ptr->logic);
//        tab_drop(db, left_table);
//        tab_drop(db, right_table);
//    } else {
//        result_table = simple_condition(db, condition_ast_ptr, table, schema);
//    }
//
//}

void remove_rows(row_likedlist_t* rll){
    row_node_t *current = rll->head;
    while (current != NULL)
    {
        row_node_t *next = current->next;
        rst_node_t *rst_current = current->rst_head;
        while (rst_current != NULL)
        {
            rst_node_t *rst_next = rst_current->next;
            tab_delete_row(rst_current->table, &rst_current->rowix);
            rst_current = rst_next;
        }
        current = next;
    }
}


row_likedlist_t *filter(db_t *db, struct ast *root, row_likedlist_t* rll, schema_t *schema, struct response *resp) {
    row_likedlist_t *result_list = rll;
    struct filter_ast *filter_ast_ptr = (struct filter_ast *) root;
    struct filter_condition_ast *condition_ast_ptr = (struct filter_condition_ast *) filter_ast_ptr->conditions_tree_root;
    if (condition_ast_ptr->r != NULL && condition_ast_ptr->logic != -1) {

    } else {
        simple_condition(db, condition_ast_ptr, rll, schema, resp);
    }
    return result_list;
}


row_likedlist_t *for_stmt(db_t *db, struct ast *root, struct response *resp) {
    struct for_ast *for_ast_ptr = (struct for_ast *) root;
    int64_t tabix = mtab_find_table_by_name(db->meta_table_idx, for_ast_ptr->tabname);
    if (tabix == TABLE_FAIL) {
        logger(LL_ERROR, __func__, "Failed to find table %s", for_ast_ptr->tabname);
        resp->status = -1;
        resp->message = strdupf("Failed to find table %s", for_ast_ptr->tabname);
        return NULL;
    }
    table_t *table = tab_load(tabix);
    if (table == NULL) {
        logger(LL_ERROR, __func__, "Failed to load table %s", for_ast_ptr->tabname);
        resp->status = -1;
        resp->message = strdupf("Failed to load table %s", for_ast_ptr->tabname);
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
                filtered_list = filter(db, list_ast->value, filtered_list, schema, resp);
                if (filtered_list == NULL) {
                    logger(LL_ERROR, __func__, "Failed to filter");
                    resp->status = -1;
                    resp->message = strdupf("Failed to filter");
                    return NULL;
                }
                if(resp->status == -1)
                    return NULL;
                break;
            }
            default: {
                logger(LL_ERROR, __func__, "Invalid type %d", list_ast->value->nodetype);
                resp->status = -1;
                resp->message = strdupf("Invalid type %d", list_ast->value->nodetype);
                return NULL;
            }
        }
        temp = (struct list_ast *) list_ast->next;
    }
    return filtered_list;
}


int for_op(db_t *db, struct ast *root, struct response *resp) {
    struct for_ast *for_ast_ptr = (struct for_ast *) root;
    int64_t tabix = mtab_find_table_by_name(db->meta_table_idx, for_ast_ptr->tabname);
    if (tabix == TABLE_FAIL) {
        logger(LL_ERROR, __func__, "Failed to find table %s", for_ast_ptr->tabname);
        resp->status = -1;
        resp->message = strdupf("Failed to find table %s", for_ast_ptr->tabname);
        return -1;
    }
    table_t *table = tab_load(tabix);
    if (table == NULL) {
        logger(LL_ERROR, __func__, "Failed to load table %s", for_ast_ptr->tabname);
        resp->status = -1;
        resp->message = strdupf("Failed to load table %s", for_ast_ptr->tabname);
        return -1;
    }
    schema_t *schema = sch_load(table->schidx);
    struct list_ast *temp = (struct list_ast *) for_ast_ptr->nonterm_list_head;
    reverseList(&temp);
    row_likedlist_t *filtered_list = tab_table2rll(db, table);
    row_likedlist_t *second_rll = NULL;
    while (temp != NULL) {
        struct list_ast *list_ast = (struct list_ast *) temp;
        switch (list_ast->value->nodetype) {
            case NT_FILTER: {
                filtered_list = filter(db, list_ast->value, filtered_list, schema, resp);
                if (filtered_list == NULL) {
                    logger(LL_ERROR, __func__, "Failed to filter");
                    resp->status = -1;
                    resp->message = strdupf("Failed to filter");
                    return -1;
                }
                if(resp->status == -1)
                    return -1;
                break;
            }
            case NT_FOR: {
                second_rll = for_stmt(db, list_ast->value, resp);
                if (second_rll == NULL) {
                    return -1;
                }
                break;
            }
            default: {
                logger(LL_ERROR, __func__, "Invalid type %d", list_ast->value->nodetype);
                resp->status = -1;
                resp->message = strdupf("Invalid type %d", list_ast->value->nodetype);
                return -1;
            }
        }
        temp = (struct list_ast *) list_ast->next;
    }

    struct ast *terminal = for_ast_ptr->terminal;
    switch (terminal->nodetype) {
        case NT_RETURN: {
            struct return_ast *return_ast_ptr = (struct return_ast *) terminal;
            struct ast *return_value = return_ast_ptr->value;
            switch (return_value->nodetype) {
                case NT_ATTR_NAME: {
                    struct attr_name_ast *attr_name_ast_ptr = (struct attr_name_ast *) return_value;
                    char *var = attr_name_ast_ptr->variable;

                    if (strcmp(for_ast_ptr->var, var) == 0) {
                        table_t* filtered_table = tab_rll2table(db, filtered_list,"TEMP");
                        tab_print(db, filtered_table, schema);
                        resp->status = 0;
                        resp->message = strdup("Selected successfully");
                        resp->table = filtered_table;

                    } else {
                        table_t* second_table = tab_rll2table(db, second_rll,"TEMP");
                        tab_print(db, second_table, second_rll->schema);
                        resp->status = 0;
                        resp->message = strdup("Selected successfully");
                        resp->table = second_table;

                    }
                    break;
                }
                case NT_MERGE: {
                    if(filtered_list->schema == NULL || second_rll->schema == NULL)
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
            struct remove_ast *remove_ast_ptr = (struct remove_ast *) terminal;
            char *remove_table = remove_ast_ptr->tabname;
            struct attr_name_ast *attr_name_ast_ptr = (struct attr_name_ast *) remove_ast_ptr->attr;
            char *var = attr_name_ast_ptr->variable;
            if (strcmp(for_ast_ptr->var, var) == 0) {
                remove_rows(filtered_list);
            } else {
                remove_rows(second_rll);
            }
            resp->status = 0;
            resp->message = strdup("Removed successfully");
            break;
        }
        default:{
            logger(LL_ERROR, __func__, "Invalid type %d", terminal->nodetype);
            resp->status = -1;
            resp->message = strdupf("Invalid type %d", terminal->nodetype);
        }
    }
    if(filtered_list!= NULL) row_likedlist_free(filtered_list);
    if(second_rll!= NULL) row_likedlist_free(second_rll);
    return 0;
}

int drop(db_t *db, struct ast *root, struct response *resp) {
    struct drop_ast *drop_ast_ptr = (struct drop_ast *) root;
    int64_t tabix = mtab_find_table_by_name(db->meta_table_idx, drop_ast_ptr->name);
    if (tabix == -1) {
        logger(LL_ERROR, __func__, "Failed to find table %s", drop_ast_ptr->name);
        resp->status = -1;
        resp->message = strdupf("Failed to find table %s", drop_ast_ptr->name);
        return -1;
    }
    table_t *table = tab_load(tabix);
    if (table == NULL) {
        logger(LL_ERROR, __func__, "Failed to load table %s", drop_ast_ptr->name);
        resp->status = -1;
        resp->message = strdupf("Failed to load table %s", drop_ast_ptr->name);
        return -1;
    }
    if (tab_drop(db, table) == -1) {
        logger(LL_ERROR, __func__, "Failed to drop table %s", drop_ast_ptr->name);
        resp->status = -1;
        resp->message = strdupf("Failed to drop table %s", drop_ast_ptr->name);
        return -1;
    }
    resp->status = 0;
    resp->message = strdupf("Table %s dropped successfully", drop_ast_ptr->name);
    return 0;
}

int insert(db_t *db, struct ast *root, struct response *resp) {
    struct insert_ast *insert_ast_ptr = (struct insert_ast *) root;
    int64_t tabix = mtab_find_table_by_name(db->meta_table_idx, insert_ast_ptr->tabname);
    if (tabix == TABLE_FAIL) {
        logger(LL_ERROR, __func__, "Failed to find table %s", insert_ast_ptr->tabname);
        resp->status = -1;
        resp->message = strdup("Failed to find table");
        return -1;
    }
    table_t *table = tab_load(tabix);
    if (table == NULL) {
        logger(LL_ERROR, __func__, "Failed to load table %s", insert_ast_ptr->tabname);
        resp->status = -1;
        resp->message = strdup("Failed to load table");
        return -1;
    }
    struct ast *temp = insert_ast_ptr->list;
    schema_t *schema = sch_load(table->schidx);
    uint8_t *row = malloc(schema->slot_size);
    if (row == NULL) {
        logger(LL_ERROR, __func__, "Failed to allocate memory");
        resp->status = -1;
        resp->message = strdup("Failed to allocate memory");
        return -1;
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
                    logger(LL_ERROR, __func__, "Invalid type %d", pair_ast->value->nodetype);
                    resp->status = -1;
                    resp->message = strdupf("Invalid type %d", pair_ast->value->nodetype);
                    return -1;
                }
                struct nint *integer_val = (struct nint *) pair_ast->value;
                int64_t int_val = (int64_t) integer_val->value;
                memcpy(row + fieldi.offset, &int_val, fieldi.size);
                break;
            }
            case DT_FLOAT: {
                if (pair_ast->value->nodetype != NT_FLOATVAL) {
                    logger(LL_ERROR, __func__, "Invalid type %d", pair_ast->value->nodetype);
                    resp->status = -1;
                    resp->message = strdupf("Invalid type %d", pair_ast->value->nodetype);
                    return -1;
                }
                struct nfloat *float_val = (struct nfloat *) pair_ast->value;
                double double_val = (double) float_val->value;
                memcpy(row + fieldi.offset, &double_val, fieldi.size);
                break;
            }
            case DT_BOOL: {
                if (pair_ast->value->nodetype != NT_BOOLVAL) {
                    logger(LL_ERROR, __func__, "Invalid type %d", pair_ast->value->nodetype);
                    resp->status = -1;
                    resp->message = strdupf("Invalid type %d", pair_ast->value->nodetype);
                    return -1;
                }
                struct nint *bool_val = (struct nint *) pair_ast->value;
                bool boolval = (bool) bool_val->value;
                memcpy(row + fieldi.offset, &(boolval), fieldi.size);
                break;
            }
            case DT_VARCHAR: {
                if (pair_ast->value->nodetype != NT_STRINGVAL) {
                    logger(LL_ERROR, __func__, "Invalid type %d", pair_ast->value->nodetype);
                    resp->status = -1;
                    resp->message = strdupf("Invalid type %d", pair_ast->value->nodetype);
                    return -1;
                }
                struct nstring *string_val = (struct nstring *) pair_ast->value;
                vch_ticket_t ticket = vch_add(db->varchar_mgr_idx, string_val->value);
                memcpy(row + fieldi.offset, &ticket, fieldi.size);
                break;
            }
            default: {
                logger(LL_ERROR, __func__, "Invalid type %d", fieldi.type);
                resp->status = -1;
                resp->message = strdupf("Invalid type %d", fieldi.type);
                return -1;
            }

        }
    }
    tab_insert(table, schema, row);
    resp->status = 0;
    resp->message = strdup("Row inserted successfully");
    free(row);
    return 0;
}

int create(db_t *db, struct ast *root, struct response *resp) {
    struct create_ast *create_ast = (struct create_ast *) root;
    schema_t *schema = sch_init();
    if (schema == NULL) {
        logger(LL_ERROR, __func__, "Failed to create schema");
        resp->status = -1;
        resp->message = strdup("Failed to create schema");
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
    table_t *table = tab_init(db, create_ast->name, schema);
    if (table == NULL) {
        logger(LL_ERROR, __func__, "Failed to create table %s", create_ast->name);
        resp->status = -1;
        resp->message = strdup("Failed to create table");
        return -1;
    }
    resp->status = 0;
    resp->message = strdupf("Table %s created successfully", create_ast->name);
    return 0;
}

int reqexe(db_t *db, struct ast *root, struct response *resp) {
    if (!root) {
        logger(LL_ERROR, __func__, "Root is NULL");
        resp->status = -1;
        resp->message = strdup("Root is NULL");
        return -1;
    }
    switch (root->nodetype) {
        case NT_CREATE: {
            create(db, root, resp);
            break;
        }
        case NT_INSERT: {
            insert(db, root, resp);
            break;
        }
        case NT_FOR: {
            for_op(db, root, resp);
            break;
        }
        case NT_DROP: {
            drop(db, root, resp);
            break;
        }
        default: {
            logger(LL_ERROR, __func__, "Invalid root type %d", root->nodetype);
            resp->status = -1;
            resp->message = strdupf("Invalid root type %d", root->nodetype);
            return -1;
        }
    }
    return 0;
}
