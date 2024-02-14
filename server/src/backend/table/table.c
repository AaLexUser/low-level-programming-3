#include "table.h"
#include <inttypes.h>
#include <stdio.h>

/**
 * @brief       Initialize table and add it to the metatable
 * @param[in]   db: pointer to db
 * @param[in]   name: name of the table
 * @param[in]   schema: pointer to schema
 * @return      index of the table on success, TABLE_FAIL on failure
 */

table_t *tab_init(db_t *db, const char *name, schema_t *schema) {
    table_t *table = NULL;
    if ((table = tab_base_init(name, schema)) == NULL) {
        logger(LL_ERROR, __func__, "Unable to init table");
        return NULL;
    }
    mtab_add(db->meta_table_idx, name, table_index(table));
    return table;
}

/**
 * @brief       Get row by value in column
 * @param[in]   db: pointer to db
 * @param[in]   table: pointer to the table
 * @param[in]   schema: pointer to the schema
 * @param[in]   field: pointer to the field
 * @param[in]   value: pointer to the value
 * @param[in]   type: type of the value
 * @return      chblix_t of row on success, CHBLIX_FAIL on failure
 */

chblix_t tab_get_row(db_t *db, table_t *table, schema_t *schema, field_t *field, void *value, datatype_t type) {
    if (table == NULL) {
        logger(LL_ERROR, __func__, "Invalid argument, table is NULL");
        return CHBLIX_FAIL;
    }

    if (schema == NULL) {
        logger(LL_ERROR, __func__, "Invalid argument, schema is NULL");
        return CHBLIX_FAIL;
    }
    void *element = malloc(field->size);
    tab_for_each_element(table, chunk, chblix, element, field) {
        if (comp_eq(db, type, element, value)) {
            free(element);
            return chblix;
        }
    }
    free(element);
    return CHBLIX_FAIL;
}

/**
 * @brief       Print table
 * @param[in]   db: pointer to db
 * @param[in]   table: index of the table
 * @param[in]   schema: pointer to the schema
 */

void tab_print(db_t *db, table_t *table, schema_t *schema) {
    if (table == NULL) {
        logger(LL_ERROR, __func__, "Invalid argument, table is NULL");
        return;
    }

    if (schema == NULL) {
        logger(LL_ERROR, __func__, "Invalid argument, schema is NULL");
        return;
    }
    sch_for_each(schema, chunka, fielda, sch_chblixa, table->schidx) {
        printf("%-25s\t", fielda.name);
    }
    printf("\n");
    void *row = malloc(schema->slot_size);
    tab_for_each_row(table, chunk, chblix, row, schema) {
        sch_for_each(schema, chunk2, field, sch_chblix, table->schidx) {
            switch (field.type) {
                case DT_INT: {
                    int64_t val = *(int64_t *) ((char *) row + field.offset);
                    printf("%-25"PRId64"\t", val);
                    break;
                }
                case DT_FLOAT: {
                    double val = *(double *) ((char *) row + field.offset);
                    printf("%-25.2f\t", val);
                    break;
                }
                case DT_CHAR: {
                    char *val = (char *) ((char *) row + field.offset);
                    printf("%-25s\t", val);
                    break;
                }
                case DT_BOOL: {
                    bool val = *(bool *) ((char *) row + field.offset);
                    printf("%-25d\t", val);
                    break;
                }
                case DT_VARCHAR: {
                    vch_ticket_t *vch = (vch_ticket_t *) ((char *) row + field.offset);
                    char *str = malloc(vch->size);
                    vch_get(db->varchar_mgr_idx, vch, str);
                    printf("%-25s\t", str);
                    free(str);
                    break;
                }

                default:
                    logger(LL_ERROR, __func__, "Unknown type %d", field.type);
                    break;
            }
        }
        printf("\n");
        fflush(stdout);
    }
    free(row);
    fflush(stdout);
}

/**
 * @brief       Inner join two tables
 * @param[in]   db: pointer to db
 * @param[in]   left: pointer to the left table
 * @param[in]   left_schema: pointer to the schema of the left table
 * @param[in]   right: pointer to the right table
 * @param[in]   right_schema: pointer to the schema of the right table
 * @param[in]   join_field_left: join field of the left table
 * @param[in]   join_field_right: join field of the right table
 * @param[in]   name: name of the new table
 * @return      pointer to the new table on success, NULL on failure
 */

table_t *tab_join_on_field(
        db_t *db,
        table_t *left,
        schema_t *left_schema,
        table_t *right,
        schema_t *right_schema,
        field_t *join_field_left,
        field_t *join_field_right,
        const char *name) {

    /* Check if tables are NULL */
    if (left == NULL) {
        logger(LL_ERROR, __func__, "Invalid argument, left table is NULL");
        return NULL;
    }
    if (right == NULL) {
        logger(LL_ERROR, __func__, "Invalid argument, right table is NULL");
        return NULL;
    }

    /* Check if schemas are NULL */
    if (left_schema == NULL) {
        logger(LL_ERROR, __func__, "Invalid argument, left schema is NULL");
        return NULL;
    }
    if (right_schema == NULL) {
        logger(LL_ERROR, __func__, "Invalid argument, right schema is NULL");
        return NULL;
    }

    /* Create new schema */
    schema_t *new_schema = sch_init();
    if (new_schema == NULL) {
        logger(LL_ERROR, __func__, "Failed to create new schema");
        return NULL;
    }
    sch_for_each(left_schema, chunk, left_field_t, left_chblix, left->schidx) {
        if (sch_add_field(new_schema, left_field_t.name, left_field_t.type, (int64_t) left_field_t.size) ==
            SCHEMA_FAIL) {
            logger(LL_ERROR, __func__, "Failed to add field %s", left_field_t.name);
            return NULL;
        }
    }
    sch_for_each(right_schema, chunk2, right_field_t, right_chblix, right->schidx) {
        if (sch_add_field(new_schema, right_field_t.name, right_field_t.type, (int64_t) right_field_t.size) ==
            SCHEMA_FAIL) {
            logger(LL_ERROR, __func__, "Failed to add field %s", right_field_t.name);
            return NULL;
        }
    }

    /* Create new table */
    table_t *table = tab_init(db, name, new_schema);
    if (table == NULL) {
        logger(LL_ERROR, __func__, "Failed to create new table");
        return NULL;
    }

    /* Create new row */
    void *row = malloc(new_schema->slot_size);

    void *left_row = malloc(left_schema->slot_size);
    void *right_row = malloc(right_schema->slot_size);



    /* Join */
    void *elleft = malloc(join_field_left->size);
    void *elright = malloc(join_field_right->size);
    tab_for_each_row(left, left_chunk, leftt_chblix, left_row, left_schema) {
        memcpy(elleft, (char *) left_row + join_field_left->offset, join_field_left->size);
        tab_for_each_row(right, right_chunk, rightt_chblix, right_row, right_schema) {
            memcpy(elright, (char *) right_row + join_field_right->offset, join_field_right->size);
            if (comp_eq(db, join_field_left->type, elleft, elright)) {
                memcpy(row, left_row, left_schema->slot_size);
                memcpy((char *) row + left_schema->slot_size, right_row, right_schema->slot_size);
                chblix_t rowix = tab_insert(table, new_schema, row);
                if (chblix_cmp(&rowix, &CHBLIX_FAIL) == 0) {
                    logger(LL_ERROR, __func__, "Failed to insert row");
                    return NULL;
                }
            }
        }
    }
    free(elleft);
    free(elright);
    free(row);
    free(left_row);
    free(right_row);
    return table;
}

table_t *tab_join(db_t *db,
                  table_t *left,
                  schema_t *left_schema,
                  table_t *right,
                  schema_t *right_schema,
                  const char *name) {
    /* Check if tables are NULL */
    if (left == NULL) {
        logger(LL_ERROR, __func__, "Invalid argument, left table is NULL");
        return NULL;
    }
    if (right == NULL) {
        logger(LL_ERROR, __func__, "Invalid argument, right table is NULL");
        return NULL;
    }

    /* Check if schemas are NULL */
    if (left_schema == NULL) {
        logger(LL_ERROR, __func__, "Invalid argument, left schema is NULL");
        return NULL;
    }
    if (right_schema == NULL) {
        logger(LL_ERROR, __func__, "Invalid argument, right schema is NULL");
        return NULL;
    }

    /* Create new schema */
    schema_t *new_schema = sch_init();
    if (new_schema == NULL) {
        logger(LL_ERROR, __func__, "Failed to create new schema");
        return NULL;
    }
    sch_for_each(left_schema, chunk, left_field_t, left_chblix, left->schidx) {
        if (sch_add_field(new_schema, left_field_t.name, left_field_t.type, (int64_t) left_field_t.size) ==
            SCHEMA_FAIL) {
            logger(LL_ERROR, __func__, "Failed to add field %s", left_field_t.name);
            return NULL;
        }
    }
    sch_for_each(right_schema, chunk2, right_field_t, right_chblix, right->schidx) {
        if (sch_add_field(new_schema, right_field_t.name, right_field_t.type, (int64_t) right_field_t.size) ==
            SCHEMA_FAIL) {
            logger(LL_ERROR, __func__, "Failed to add field %s", right_field_t.name);
            return NULL;
        }
    }

    /* Create new table */
    table_t *table = tab_init(db, name, new_schema);
    if (table == NULL) {
        logger(LL_ERROR, __func__, "Failed to create new table");
        return NULL;
    }

    /* Create new row */
    void *row = malloc(new_schema->slot_size);

    void *left_row = malloc(left_schema->slot_size);
    void *right_row = malloc(right_schema->slot_size);

    tab_for_each_row(left, left_chunk, leftt_chblix, left_row, left_schema) {
        tab_for_each_row(right, right_chunk, rightt_chblix, right_row, right_schema) {
            memcpy(row, left_row, left_schema->slot_size);
            memcpy((char *) row + left_schema->slot_size, right_row, right_schema->slot_size);
            chblix_t rowix = tab_insert(table, new_schema, row);
            if (chblix_cmp(&rowix, &CHBLIX_FAIL) == 0) {
                logger(LL_ERROR, __func__, "Failed to insert row");
                return NULL;
            }
        }
    }
    free(row);
    free(left_row);
    free(right_row);
    return table;
}


/**
 * @brief       Select row form table on condition
 * @param[in]   db: pointer to db
 * @param[in]   sel_table: pointer to table from which the selection is made
 * @param[in]   sel_schema: pointer to schema of the table from which the selection is made
 * @param[in]   select_field: the field by which the selection is performed
 * @param[in]   name: name of new table that will be created
 * @param[in]   condition: comparison condition
 * @param[in]   value: value to compare with
 * @param[in]   type: the type of value to compare with
 * @return      pointer to new table on success, NULL on failure
 */

table_t *tab_select_op(db_t *db,
                       table_t *sel_table,
                       schema_t *sel_schema,
                       field_t *select_field,
                       const char *name,
                       condition_t condition,
                       void *value,
                       datatype_t type) {
    /* Create new schema */
    schema_t *schema = sch_init();
    if (schema == NULL) {
        logger(LL_ERROR, __func__, "Failed to create new schema");
        return NULL;
    }
    sch_for_each(sel_schema, sch_chunk, field, chblix, sel_table->schidx) {
        if (sch_add_field(schema, field.name, field.type, (int64_t) field.size) == SCHEMA_FAIL) {
            logger(LL_ERROR, __func__, "Failed to add field %s", field.name);
            return NULL;
        }
    }

    /* Create new table */
    table_t *table = tab_init(db, name, schema);
    if (table == NULL) {
        logger(LL_ERROR, __func__, "Failed to create new table");
        return NULL;
    }

    /* Create new row */
    void *row = malloc(schema->slot_size);

    /* Check if datatype of field equals datatype of value */
    if (type != select_field->type) {
        free(row);
        return NULL;
    }

    void *el_row = malloc(sel_schema->slot_size);
    void *el = malloc(select_field->size);
    void *comp_val = malloc(select_field->size);
    memcpy(comp_val, value, select_field->size);

    /* Select */
    tab_for_each_row(sel_table, tab_chunk, sel_chblix, el_row, sel_schema) {
        memcpy(el, (char *) el_row + select_field->offset, select_field->size);
        if (comp_compare(db, type, el, comp_val, condition)) {
            memcpy(row, el_row, schema->slot_size);
            chblix_t rowix = tab_insert(table, schema, row);
            if (chblix_cmp(&rowix, &CHBLIX_FAIL) == 0) {
                logger(LL_ERROR, __func__, "Failed to insert row");
                return NULL;
            }
        }
    }
    free(comp_val);
    free(row);
    free(el_row);
    free(el);
    return table;
}

/**
 * @brief       Filters rows from a table based on a condition
 * @param[in]   db: pointer to the database
 * @param[in]   sel_table: pointer to the table from which the selection is made
 * @param[in]   sel_schema: pointer to the schema of the table from which the selection is made
 * @param[in]   select_field: the field by which the selection is performed
 * @param[in]   condition: comparison condition
 * @param[in]   value: value to compare with
 * @param[in]   type: the type of value to compare with
 * @return      row_likedlist_t: a linked list of rows that satisfy the condition
 */

row_likedlist_t *tab_filter(db_t *db,
                            table_t *sel_table,
                            schema_t *sel_schema,
                            field_t *select_field,
                            condition_t condition,
                            void *value,
                            datatype_t type) {
    row_likedlist_t *list = row_likedlist_init(sel_schema);
    if (list == NULL) {
        logger(LL_ERROR, __func__, "Failed to create new row_likedlist");
        return NULL;
    }

    /* Check if datatype of field equals datatype of value */
    if (type != select_field->type) {
        return NULL;
    }

    void *el_row = malloc(sel_schema->slot_size);
    void *el = malloc(select_field->size);
    void *comp_val = malloc(select_field->size);
    memcpy(comp_val, value, select_field->size);

    /* Select */
    tab_for_each_row(sel_table, tab_chunk, sel_chblix, el_row, sel_schema) {
        memcpy(el, (char *) el_row + select_field->offset, select_field->size);
        if (comp_compare(db, type, el, comp_val, condition)) {
            row_likedlist_add(list, &sel_chblix, el_row, sel_schema, sel_table);
        }
    }
    free(comp_val);
    free(el_row);
    free(el);
    return list;
}

row_likedlist_t *rll_filter_var(db_t *db,
                                row_likedlist_t *right_list,
                                field_t *right_field,
                                condition_t condition,
                                row_likedlist_t *left_list,
                                field_t *left_field,
                                datatype_t type) {
    if (type != right_field->type) {
        return NULL;
    }

    /* Create new schema */
    schema_t *new_schema = sch_init();
    if (new_schema == NULL) {
        logger(LL_ERROR, __func__, "Failed to create new schema");
        return NULL;
    }

    sch_for_each(left_list->schema, chunk2, field2, chblix2, schema_index(left_list->schema)) {
        if (sch_add_field(new_schema, field2.name, field2.type, (int64_t) field2.size) == SCHEMA_FAIL) {
            logger(LL_ERROR, __func__, "Failed to add field %s", field2.name);
            return NULL;
        }
    }

    sch_for_each(right_list->schema, chunk, field, chblix, schema_index(right_list->schema)) {
        if (sch_add_field(new_schema, field.name, field.type, (int64_t) field.size) == SCHEMA_FAIL) {
            logger(LL_ERROR, __func__, "Failed to add field %s", field.name);
            return NULL;
        }
    }

    row_likedlist_t *list = row_likedlist_init(new_schema);
    if(list == NULL){
        logger(LL_ERROR, __func__, "Failed to create new row_likedlist");
        return NULL;
    }

    void *el1 = malloc(right_field->size);
    void *el2 = malloc(left_field->size);
    void *row = malloc(new_schema->slot_size);

    for(row_node_t *current_left = left_list->head; current_left!= NULL; current_left = current_left->next){
        memcpy(el1, (char *) current_left->row + left_field->offset, left_field->size);
        for(row_node_t* current_right = right_list->head; current_right != NULL; current_right = current_right->next){
            memcpy(el2, (char *) current_right->row + right_field->offset, right_field->size);
            if(comp_compare(db, type, el1, el2, condition)){
                memcpy(row, current_left->row, left_list->schema->slot_size);
                memcpy((char *) row + left_list->schema->slot_size, current_right->row, right_list->schema->slot_size);
                row_likedlist_add(list, &current_left->rst_head->rowix, row, current_left->rst_head->schema, current_left->rst_head->table);
                row_node_t *current_row = list->tail;
                rst_node_t *current_rst_left = current_left->rst_head->next;
                while(current_rst_left != NULL){
                    row_likedlist_add_rst(&current_rst_left->rowix, current_row, current_rst_left->schema, current_rst_left->table);
                    current_rst_left = current_rst_left->next;
                }
                rst_node_t *current_rst_right = current_right->rst_head;
                while(current_rst_right != NULL){
                    row_likedlist_add_rst(&current_rst_right->rowix, current_row, current_rst_right->schema, current_rst_right->table);
                    current_rst_right = current_rst_right->next;
                }
            }
        }
    }

    free(el1);
    free(el2);
    free(row);
    return list;
}

row_likedlist_t *rll_filter(db_t *db,
                            row_likedlist_t *rll,
                            field_t *select_field,
                            condition_t condition,
                            void *value,
                            datatype_t type) {
    /* Check if datatype of field equals datatype of value */
    if (type != select_field->type) {
        return NULL;
    }

    row_likedlist_t *list = row_likedlist_init(rll->schema);

    void *el_row = malloc(rll->schema->slot_size);
    void *el = malloc(select_field->size);
    void *comp_val = malloc(select_field->size);
    memcpy(comp_val, value, select_field->size);

    /* Select */
    row_node_t *current = rll->head;
    while (current != NULL) {
        memcpy(el, (char *) current->row + select_field->offset, select_field->size);
        if (comp_compare(db, type, el, comp_val, condition)) {
            row_likedlist_add(list, &current->rst_head->rowix, current->row, current->rst_head->schema,
                              current->rst_head->table);
            row_node_t *current_row = list->tail;
            rst_node_t *current_rst = current->rst_head->next;
            while (current_rst != NULL) {
                row_likedlist_add_rst(&current_rst->rowix, current_row, current_rst->schema, current_rst->table);
                current_rst = current_rst->next;
            }
            current = current->next;
        } else {
            current = current->next;
        }
    }
    free(comp_val);
    free(el_row);
    free(el);
    return list;
}

static int rrl_validate_join_context(row_likedlist_t *left, row_likedlist_t *right) {
    if (left == NULL) {
        logger(LL_ERROR, __func__, "Invalid argument, left table is NULL");
        return -1;
    }

    if (right == NULL) {
        logger(LL_ERROR, __func__, "Invalid argument, right table is NULL");
        return -1;
    }

    if (left->schema == NULL) {
        logger(LL_ERROR, __func__, "Invalid argument, left schema is NULL");
        return -1;
    }

    if (right->schema == NULL) {
        logger(LL_ERROR, __func__, "Invalid argument, right schema is NULL");
        return -1;
    }
    return 0;
}

row_likedlist_t *rll_join(db_t *db,
                          row_likedlist_t *left,
                          row_likedlist_t *right) {
    if (rrl_validate_join_context(left, right) == -1) {
        return NULL;
    }
    /* Create new schema */
    schema_t *new_schema = sch_init();
    if (new_schema == NULL) {
        logger(LL_ERROR, __func__, "Failed to create new schema");
        return NULL;
    }

    sch_for_each(left->schema, chunk, left_field_t, left_chblix, schema_index(left->schema)) {
        if (sch_add_field(new_schema, left_field_t.name, left_field_t.type, (int64_t) left_field_t.size) ==
            SCHEMA_FAIL) {
            logger(LL_ERROR, __func__, "Failed to add field %s", left_field_t.name);
            return NULL;
        }
    }
    sch_for_each(right->schema, chunk2, right_field_t, right_chblix, schema_index(right->schema)) {
        if (sch_add_field(new_schema, right_field_t.name, right_field_t.type, (int64_t) right_field_t.size) ==
            SCHEMA_FAIL) {
            logger(LL_ERROR, __func__, "Failed to add field %s", right_field_t.name);
            return NULL;
        }
    }

    row_likedlist_t *list = row_likedlist_init(new_schema);
    if (list == NULL) {
        logger(LL_ERROR, __func__, "Failed to create new row_likedlist");
        return NULL;
    }

    /* Create new row */
    void *row = malloc(new_schema->slot_size);

    row_node_t *current_left = left->head;

    /* Join */
    while (current_left != NULL) {
        row_node_t *current_right = right->head;
        while (current_right != NULL) {
            memcpy(row, current_left->row, left->schema->slot_size);
            memcpy((char *) row + left->schema->slot_size, current_right->row, right->schema->slot_size);
            row_likedlist_add(list, &current_left->rst_head->rowix, row, current_left->rst_head->schema,
                              current_left->rst_head->table);
            row_node_t *current_row = list->tail;
            rst_node_t *current_rst_left = current_left->rst_head->next;
            while (current_rst_left != NULL) {
                row_likedlist_add_rst(&current_rst_left->rowix, current_row, current_rst_left->schema,
                                      current_rst_left->table);
                current_rst_left = current_rst_left->next;
            }
            rst_node_t *current_rst_right = current_right->rst_head;
            while (current_rst_right != NULL) {
                row_likedlist_add_rst(&current_rst_right->rowix, current_row, current_rst_right->schema,
                                      current_rst_right->table);
                current_rst_right = current_rst_right->next;
            }
            current_right = current_right->next;
        }
        current_left = current_left->next;
    }
    free(row);
    return list;
}

row_likedlist_t *rll_join_or(row_likedlist_t *left,
                             row_likedlist_t *right) {
    if (rrl_validate_join_context(left, right) == -1) {
        return NULL;
    }
    /* Create new schema */
    schema_t *new_schema = sch_init();
    if (new_schema == NULL) {
        logger(LL_ERROR, __func__, "Failed to create new schema");
        return NULL;
    }

    sch_for_each(left->schema, chunk, left_field_t, left_chblix, schema_index(left->schema)) {
        if (sch_add_field(new_schema, left_field_t.name, left_field_t.type, (int64_t) left_field_t.size) ==
            SCHEMA_FAIL) {
            logger(LL_ERROR, __func__, "Failed to add field %s", left_field_t.name);
            return NULL;
        }
    }

    row_likedlist_t *list = row_likedlist_init(new_schema);
    if (list == NULL) {
        logger(LL_ERROR, __func__, "Failed to create new row_likedlist");
        return NULL;
    }

    /* Create new row */
    void *row = malloc(new_schema->slot_size);


    /* Join */
    for (row_node_t *current_left = left->head; current_left != NULL; current_left = current_left->next) {
        memcpy(row, current_left->row, left->schema->slot_size);
        row_likedlist_add(list, &current_left->rst_head->rowix, row, current_left->rst_head->schema,
                          current_left->rst_head->table);
        row_node_t *current_row = list->tail;
        rst_node_t *current_rst_left = current_left->rst_head->next;
        while (current_rst_left != NULL) {
            row_likedlist_add_rst(&current_rst_left->rowix, current_row, current_rst_left->schema,
                                  current_rst_left->table);
            current_rst_left = current_rst_left->next;
        }
    }

    for (row_node_t *current_right = right->head; current_right != NULL; current_right = current_right->next) {
        row_node_t *current_left = left->head;
        for (; current_left != NULL; current_left = current_left->next) {
            if (chblix_cmp(&current_right->rst_head->rowix, &current_left->rst_head->rowix) == 0) {
                break;
            }
        }
        if (current_left != NULL) {
            continue;
        }
        memcpy(row, current_right->row, right->schema->slot_size);
        row_likedlist_add(list, &current_right->rst_head->rowix, row, current_right->rst_head->schema,
                          current_right->rst_head->table);
        for (rst_node_t *current_rst_right = current_right->rst_head->next;
             current_rst_right != NULL; current_rst_right = current_rst_right->next) {
            row_likedlist_add_rst(&current_rst_right->rowix, list->tail, current_rst_right->schema,
                                  current_rst_right->table);
        }
    }
    free(row);
    return list;
}

row_likedlist_t *rll_join_and(row_likedlist_t *left,
                              row_likedlist_t *right) {
    if (rrl_validate_join_context(left, right) == -1) {
        return NULL;
    }
    /* Create new schema */
    schema_t *new_schema = sch_init();
    if (new_schema == NULL) {
        logger(LL_ERROR, __func__, "Failed to create new schema");
        return NULL;
    }

    sch_for_each(left->schema, chunk, left_field_t, left_chblix, schema_index(left->schema)) {
        if (sch_add_field(new_schema, left_field_t.name, left_field_t.type, (int64_t) left_field_t.size) ==
            SCHEMA_FAIL) {
            logger(LL_ERROR, __func__, "Failed to add field %s", left_field_t.name);
            return NULL;
        }
    }

    row_likedlist_t *list = row_likedlist_init(new_schema);
    if (list == NULL) {
        logger(LL_ERROR, __func__, "Failed to create new row_likedlist");
        return NULL;
    }

    /* Create new row */
    void *row = malloc(new_schema->slot_size);

    /* Join */
    for (row_node_t *current_left = left->head; current_left != NULL; current_left = current_left->next) {
        for (row_node_t *current_right = right->head; current_right != NULL; current_right = current_right->next) {
            if (chblix_cmp(&current_left->rst_head->rowix, &current_right->rst_head->rowix) == 0) {
                memcpy(row, current_left->row, left->schema->slot_size);
                row_likedlist_add(list, &current_left->rst_head->rowix, row, current_left->rst_head->schema,
                                  current_left->rst_head->table);
                row_node_t *current_row = list->tail;
                rst_node_t *current_rst_left = current_left->rst_head->next;
                while (current_rst_left != NULL) {
                    row_likedlist_add_rst(&current_rst_left->rowix, current_row, current_rst_left->schema,
                                          current_rst_left->table);
                    current_rst_left = current_rst_left->next;
                }
                rst_node_t *current_rst_right = current_right->rst_head;
                while (current_rst_right != NULL) {
                    row_likedlist_add_rst(&current_rst_right->rowix, current_row, current_rst_right->schema,
                                          current_rst_right->table);
                    current_rst_right = current_rst_right->next;
                }
            }
        }
    }
    free(row);
    return list;
}

/**
 * @brief       Converts a linked list of rows into a table
 * @param[in]   db: pointer to the database
 * @param[in]   row_ll: pointer to the linked list of rows
 * @param[in]   name: name of the new table
 * @return      table_t*: pointer to the new table on success, NULL on failure
 *
 */

table_t *tab_rll2table(db_t *db, row_likedlist_t *row_ll, const char *name) {
    /* Create new schema */
    schema_t *schema = sch_init();
    if (schema == NULL) {
        logger(LL_ERROR, __func__, "Failed to create new schema");
        return NULL;
    }
    sch_for_each(row_ll->schema, chunk, field, chblix, schema_index(row_ll->schema)) {
        if (sch_add_field(schema, field.name, field.type, (int64_t) field.size) == SCHEMA_FAIL) {
            logger(LL_ERROR, __func__, "Failed to add field %s", field.name);
            return NULL;
        }
    }

    table_t *table = tab_init(db, name, schema);
    if (table == NULL) {
        logger(LL_ERROR, __func__, "Failed to create new table");
        return NULL;
    }

    row_node_t *current = row_ll->head;
    uint8_t *row_dest = malloc(row_ll->schema->slot_size);
    while (current != NULL) {
        chblix_t rowix = tab_insert(table, row_ll->schema, current->row);
        if (chblix_cmp(&rowix, &CHBLIX_FAIL) == 0) {
            logger(LL_ERROR, __func__, "Failed to insert row");
            return NULL;
        }
        current = current->next;
    }
    free(row_dest);
    return table;
}

row_likedlist_t *tab_table2rll(db_t *db, table_t *table) {
    schema_t *schema = sch_load(table->schidx);
    row_likedlist_t *list = row_likedlist_init(schema);
    if (list == NULL) {
        logger(LL_ERROR, __func__, "Failed to create new row_likedlist");
        return NULL;
    }
    void *row = malloc(schema->slot_size);
    tab_for_each_row(table, tab_chunk, chblix, row, schema) {
        row_likedlist_add(list, &chblix, row, schema, table);
    }
    free(row);
    return list;
}

/**
 * @brief       Drop a table
 * @param[in]   db: pointer to db
 * @param[in]   table: pointer of the table
 * @return      PPL_SUCCESS on success, PPL_FAIL on failure
 */

int tab_drop(db_t *db, table_t *table) {
    if (mtab_delete(db->meta_table_idx, table_index(table)) == TABLE_FAIL) {
        logger(LL_ERROR, __func__, "Failed to delete table %"PRId64, table_index(table));
        return PPL_FAIL;
    }
    sch_delete(table->schidx);
    return lb_ppl_destroy(table_index(table));
}

/**
 * @brief       Update row in table
 * @param[in]   db: pointer to db
 * @param[in]   table: pointer to table
 * @param[in]   schema: pointer to schema
 * @param[in]   field: pointer to field
 * @param[in]   condition: comparison condition
 * @param[in]   value: value to compare with
 * @param[in]   type: the type of value to compare with
 * @param[in]   row: pointer to new row which will replace the old one
 * @return
 */

int tab_update_row_op(db_t *db,
                      table_t *table,
                      schema_t *schema,
                      field_t *field,
                      condition_t condition,
                      void *value,
                      datatype_t type,
                      void *row) {

    void *el_row = malloc(schema->slot_size);
    void *el = malloc(field->size);
    void *comp_val = malloc(field->size);
    memcpy(comp_val, value, field->size);
    int64_t counter = 0;

    /* Update */
    tab_for_each_row(table, upd_chunk, upd_chblix, el_row, schema) {
        counter++;
        memcpy(el, (char *) el_row + field->offset, field->size);
        if (comp_compare(db, type, el, comp_val, condition)) {
            memcpy(el_row, row, schema->slot_size);
            if (tab_update_row(table, schema, &upd_chblix, el_row) == TABLE_FAIL) {
                logger(LL_ERROR, __func__, "Failed to update row");
                return TABLE_FAIL;
            }
        }
        if (counter == 488) {
            printf("stop");
        }
    }
    printf("counter %"PRId64"\n", counter);
    free(comp_val);
    free(el_row);
    free(el);
    return TABLE_SUCCESS;
}


/**
 * @brief       Update element in table
 * @param[in]   db: pointer to db
 * @param[in]   tablix: index of the table
 * @param[in]   element: element to write
 * @param[in]   field_name: name of the element field
 * @param[in]   field_comp: name of the field compare with
 * @param[in]   condition: comparison condition
 * @param[in]   value: value to compare with
 * @param[in]   type: the type of value to compare with
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int tab_update_element_op(db_t *db,
                          int64_t tablix,
                          void *element,
                          const char *field_name,
                          const char *field_comp,
                          condition_t condition,
                          void *value,
                          datatype_t type) {
    /* Load table */
    table_t *upd_tab = tab_load(tablix);
    if (upd_tab == NULL) {
        logger(LL_ERROR, __func__, "Failed to load table %"PRId64, tablix);
        return TABLE_FAIL;
    }

    /* Load schema */
    schema_t *upd_schema = sch_load(upd_tab->schidx);
    if (upd_schema == NULL) {
        logger(LL_ERROR, __func__, "Failed to load schema %"PRId64, upd_tab->schidx);
        return TABLE_FAIL;
    }

    /* Load compare field */
    field_t comp_field;
    if (sch_get_field(upd_schema, field_comp, &comp_field) == SCHEMA_FAIL) {
        logger(LL_ERROR, __func__, "Failed to get field %s", field_comp);
        return TABLE_FAIL;
    }

    /* Load update field */
    field_t upd_field;
    if (sch_get_field(upd_schema, field_name, &upd_field) == SCHEMA_FAIL) {
        logger(LL_ERROR, __func__, "Failed to get field %s", field_comp);
        return TABLE_FAIL;
    }

    /* Check if datatype of field equals datatype of value */
    if (type != comp_field.type) {
        return TABLE_FAIL;
    }

    void *el_row = malloc(upd_schema->slot_size);
    void *el = malloc(comp_field.size);
    void *upd_el = malloc(upd_field.size);
    void *comp_val = malloc(comp_field.size);
    memcpy(comp_val, value, comp_field.size);

    /* Update */
    tab_for_each_row(upd_tab, upd_chunk, upd_chblix, el_row, upd_schema) {
        memcpy(el, (char *) el_row + comp_field.offset, comp_field.size);
        if (comp_compare(db, type, el, comp_val, condition)) {
            memcpy(upd_el, element, upd_field.size);
            if (tab_update_element(upd_tab, &upd_chblix, &upd_field, upd_el) == TABLE_FAIL) {
                logger(LL_ERROR, __func__, "Failed to update row");
                return TABLE_FAIL;
            }
        }
    }
    free(upd_el);
    free(comp_val);
    free(el_row);
    free(el);
    return TABLE_SUCCESS;
}

/**
 * @brief       Delete row from table
 * @param[in]   db: pointer to db
 * @param[in]   table: pointer to table
 * @param[in]   schema: pointer to schema
 * @param[in]   field_comp: pointer to field to compare
 * @param[in]   condition: comparison condition
 * @param[in]   value: value to compare with
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int tab_delete_op(db_t *db,
                  table_t *table,
                  schema_t *schema,
                  field_t *field_comp,
                  condition_t condition,
                  void *value) {

    void *el_row = malloc(schema->slot_size);
    void *el = malloc(field_comp->size);
    void *comp_val = malloc(field_comp->size);
    memcpy(comp_val, value, field_comp->size);
    int64_t counter = 0;

    /* Delete */
    tab_for_each_row(table, del_chunk, del_chblix, el_row, schema) {
        counter++;
        memcpy(el, (char *) el_row + field_comp->offset, field_comp->size);
//        int64_t* id = el;
//        printf("c: %lld | b: %lld | id: %lld\n", del_chblix.chunk_idx, del_chblix.block_idx, *id);
        if (comp_compare(db, field_comp->type, el, comp_val, condition)) {
            chblix_t temp = del_chblix;
            bool flag = false;
            if (del_chunk->num_of_free_blocks + 1 == del_chunk->capacity) {
                int64_t next_chunk = del_chunk->next_page;
                temp = (chblix_t) {.block_idx = -1, .chunk_idx=next_chunk};
                flag = true;
            }
            if (tab_delete_nova(table, del_chunk, &del_chblix) == TABLE_FAIL) {
                logger(LL_ERROR, __func__, "Failed to delete row");
                return TABLE_FAIL;
            }
            if (flag) {
                del_chblix = temp;
                del_chunk = ppl_load_chunk(del_chblix.chunk_idx);
            }
        }
    }
    free(comp_val);
    free(el_row);
    free(el);
//    int64_t* val = value;
//    printf("value %llu, counter %llu\n", *val, counter);
    fflush(stdout);


    return TABLE_SUCCESS;

}


/** @brief       Create a table on a subset of fields
 *  @param[in]   db: pointer to db
 *  @param[in]   table: pointer to table
 *  @param[in]   schema: pointer to schema
 *  @param[in]   fields: pointer to fields
 *  @param[in]   num_of_fields: number of fields
 *  @param[in]   name: name of the new table
 *  @return      pointer to new table on success, NULL on failure
 */

table_t *tab_projection(db_t *db,
                        table_t *table,
                        schema_t *schema,
                        field_t *fields,
                        int64_t num_of_fields,
                        const char *name) {

    /* Create new schema */
    schema_t *new_schema = sch_init();
    if (new_schema == NULL) {
        logger(LL_ERROR, __func__, "Failed to create new schema");
        return NULL;
    }
    for (int64_t i = 0; i < num_of_fields; ++i) {
        if (sch_add_field(new_schema, fields[i].name, fields[i].type, (int64_t) fields[i].size) == SCHEMA_FAIL) {
            logger(LL_ERROR, __func__, "Failed to add field %s", fields[i].name);
            return NULL;
        }
    }

    /* Create new table */
    table_t *new_table = tab_init(db, name, new_schema);
    if (new_table == NULL) {
        logger(LL_ERROR, __func__, "Failed to create new table");
        return NULL;
    }

    /* Create new row */
    void *row = malloc(new_schema->slot_size);

    /* Projection */

    tab_for_each_row(table, chunk, chblix, row, schema) {
        for (int64_t i = 0; i < num_of_fields; ++i) {
            memcpy((char *) row + fields[i].offset, (char *) row + fields[i].offset, fields[i].size);
        }
        chblix_t rowix = tab_insert(new_table, new_schema, row);
        if (chblix_cmp(&rowix, &CHBLIX_FAIL) == 0) {
            logger(LL_ERROR, __func__, "Failed to insert row");
            return NULL;
        }
    }
    free(row);
    return new_table;
}



