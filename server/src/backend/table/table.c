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

table_t* tab_init(db_t* db, const char* name, schema_t* schema){
    table_t* table = NULL;
    if((table = tab_base_init(name, schema)) == NULL){
        logger(LL_ERROR, __func__, "Unable to init table");
        return NULL;
    }
    mtab_add(db->meta_table_idx, name,table_index(table));
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

chblix_t tab_get_row(db_t* db, table_t* table, schema_t* schema, field_t* field, void* value, datatype_t type){
    if(table == NULL){
        logger(LL_ERROR, __func__, "Invalid argument, table is NULL");
        return CHBLIX_FAIL;
    }

    if(schema == NULL){
        logger(LL_ERROR, __func__, "Invalid argument, schema is NULL");
        return CHBLIX_FAIL;
    }
    void* element = malloc(field->size);
    tab_for_each_element(table, chunk, chblix, element, field){
        if(comp_eq(db, type, element, value)){
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

void tab_print(db_t* db, table_t* table, schema_t* schema){
    if(table == NULL){
        logger(LL_ERROR, __func__, "Invalid argument, table is NULL");
        return;
    }

    if(schema == NULL){
        logger(LL_ERROR, __func__, "Invalid argument, schema is NULL");
        return;
    }
    void* row = malloc(schema->slot_size);
    tab_for_each_row(table, chunk, chblix,  row, schema){
        sch_for_each(schema,chunk2, field, sch_chblix, table->schidx){
            switch(field.type){
                case DT_INT: {
                    printf("%"PRId64"\t", *(int64_t*)((char*)row + field.offset));
                    break;
                }
                case DT_FLOAT: {
                    printf("%f\t", *(float *) ((char *) row + field.offset));
                    break;
                }
                case DT_CHAR: {
                    printf("%s\t", (char*)((char*)row + field.offset));
                    break;
                }
                case DT_BOOL: {
                    printf("%d\t", *(bool*)((char*)row + field.offset));
                    break;
                }
                case DT_VARCHAR: {
                    vch_ticket_t* vch = (vch_ticket_t*)((char*)row + field.offset);
                    char* str = malloc(vch->size);
                    vch_get(db->varchar_mgr_idx, vch, str);
                    printf("%s\t", str);
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

table_t* tab_join(
        db_t* db,
        table_t* left,
        schema_t* left_schema,
        table_t* right,
        schema_t* right_schema,
        field_t* join_field_left,
        field_t* join_field_right,
        const char* name){

    /* Check if tables are NULL */
    if(left == NULL){
        logger(LL_ERROR, __func__, "Invalid argument, left table is NULL");
        return NULL;
    }
    if(right == NULL){
        logger(LL_ERROR, __func__, "Invalid argument, right table is NULL");
        return NULL;
    }

    /* Check if schemas are NULL */
    if(left_schema == NULL){
        logger(LL_ERROR, __func__, "Invalid argument, left schema is NULL");
        return NULL;
    }
    if(right_schema == NULL){
        logger(LL_ERROR, __func__, "Invalid argument, right schema is NULL");
        return NULL;
    }

    /* Create new schema */
    schema_t* new_schema = sch_init();
    if(new_schema == NULL){
        logger(LL_ERROR, __func__, "Failed to create new schema");
        return NULL;
    }
    sch_for_each(left_schema, chunk, left_field_t, left_chblix, left->schidx){
        if(sch_add_field(new_schema, left_field_t.name, left_field_t.type, (int64_t)left_field_t.size) == SCHEMA_FAIL){
            logger(LL_ERROR, __func__, "Failed to add field %s", left_field_t.name);
            return NULL;
        }
    }
    sch_for_each(right_schema,chunk2, right_field_t, right_chblix, right->schidx){
        if(sch_add_field(new_schema, right_field_t.name, right_field_t.type, (int64_t)right_field_t.size) == SCHEMA_FAIL){
            logger(LL_ERROR, __func__, "Failed to add field %s", right_field_t.name);
            return NULL;
        }
    }

    /* Create new table */
    table_t* table = tab_init(db, name, new_schema);
    if(table == NULL){
        logger(LL_ERROR, __func__, "Failed to create new table");
        return NULL;
    }

    /* Create new row */
    void* row = malloc(new_schema->slot_size);

    void* left_row = malloc(left_schema->slot_size);
    void* right_row = malloc(right_schema->slot_size);



    /* Join */
    void* elleft = malloc(join_field_left->size);
    void* elright = malloc(join_field_right->size);
    tab_for_each_row(left, left_chunk, leftt_chblix, left_row, left_schema){
        memcpy(elleft, (char*)left_row + join_field_left->offset, join_field_left->size);
        tab_for_each_row(right, right_chunk,rightt_chblix, right_row, right_schema){
            memcpy(elright, (char*)right_row + join_field_right->offset, join_field_right->size);
            if(comp_eq(db, join_field_left->type, elleft, elright)){
                memcpy(row, left_row, left_schema->slot_size);
                memcpy((char*)row + left_schema->slot_size, right_row, right_schema->slot_size);
                chblix_t rowix = tab_insert(table, new_schema, row);
                if(chblix_cmp(&rowix, &CHBLIX_FAIL) == 0){
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

table_t* tab_select_op(db_t* db,
                            table_t* sel_table,
                            schema_t* sel_schema,
                            field_t* select_field,
                            const char* name,
                            condition_t condition,
                            void* value,
                            datatype_t type) {
    /* Create new schema */
    schema_t* schema = sch_init();
    if(schema == NULL){
        logger(LL_ERROR, __func__, "Failed to create new schema");
        return NULL;
    }
    sch_for_each(sel_schema, sch_chunk, field, chblix, sel_table->schidx){
        if(sch_add_field(schema, field.name, field.type, (int64_t)field.size) == SCHEMA_FAIL){
            logger(LL_ERROR, __func__, "Failed to add field %s", field.name);
            return NULL;
        }
    }

    /* Create new table */
    table_t* table = tab_init(db, name, schema);
    if(table == NULL){
        logger(LL_ERROR, __func__, "Failed to create new table");
        return NULL;
    }

    /* Create new row */
    void* row = malloc(schema->slot_size);

    /* Check if datatype of field equals datatype of value */
    if(type != select_field->type){
        free(row);
        return NULL;
    }

    void* el_row = malloc(sel_schema->slot_size);
    void* el = malloc(select_field->size);
    void* comp_val = malloc(select_field->size);
    memcpy(comp_val, value, select_field->size);

    /* Select */
    tab_for_each_row(sel_table, tab_chunk, sel_chblix, el_row, sel_schema){
        memcpy(el, (char*)el_row + select_field->offset, select_field->size);
        if(comp_compare(db, type, el,comp_val, condition)){
            memcpy(row, el_row, schema->slot_size);
            chblix_t rowix = tab_insert(table, schema, row);
            if(chblix_cmp(&rowix, &CHBLIX_FAIL) == 0){
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
 * @brief       Drop a table
 * @param[in]   db: pointer to db
 * @param[in]   table: pointer of the table
 * @return      PPL_SUCCESS on success, PPL_FAIL on failure
 */

int tab_drop(db_t* db, table_t* table){
    if (mtab_delete(db->meta_table_idx,table_index(table)) == TABLE_FAIL) {
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

int tab_update_row_op(db_t* db,
                    table_t* table,
                    schema_t* schema,
                    field_t* field,
                    condition_t condition,
                    void* value,
                    datatype_t type,
                    void* row){

    void* el_row = malloc(schema->slot_size);
    void* el = malloc(field->size);
    void* comp_val = malloc(field->size);
    memcpy(comp_val, value, field->size);
    int64_t counter = 0;

    /* Update */
    tab_for_each_row(table, upd_chunk,upd_chblix, el_row, schema){
        counter++;
        memcpy(el, (char*)el_row + field->offset, field->size);
        if(comp_compare(db, type, el, comp_val, condition)){
            memcpy(el_row, row, schema->slot_size);
            if(tab_update_row(table,schema, &upd_chblix, el_row) == TABLE_FAIL){
                logger(LL_ERROR, __func__, "Failed to update row");
                return TABLE_FAIL;
            }
        }
        if(counter == 488){
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

int tab_update_element_op(db_t* db,
                            int64_t tablix,
                            void* element,
                            const char* field_name,
                            const char* field_comp,
                            condition_t condition,
                            void* value,
                            datatype_t type) {
    /* Load table */
    table_t* upd_tab = tab_load(tablix);
    if(upd_tab == NULL){
        logger(LL_ERROR, __func__, "Failed to load table %"PRId64, tablix);
        return TABLE_FAIL;
    }

    /* Load schema */
    schema_t* upd_schema = sch_load(upd_tab->schidx);
    if(upd_schema == NULL){
        logger(LL_ERROR, __func__, "Failed to load schema %"PRId64, upd_tab->schidx);
        return TABLE_FAIL;
    }

    /* Load compare field */
    field_t comp_field;
    if(sch_get_field(upd_schema, field_comp, &comp_field) == SCHEMA_FAIL){
        logger(LL_ERROR, __func__, "Failed to get field %s", field_comp);
        return TABLE_FAIL;
    }

    /* Load update field */
    field_t upd_field;
    if(sch_get_field(upd_schema, field_name, &upd_field) == SCHEMA_FAIL){
        logger(LL_ERROR, __func__, "Failed to get field %s", field_comp);
        return TABLE_FAIL;
    }

    /* Check if datatype of field equals datatype of value */
    if(type != comp_field.type){
        return TABLE_FAIL;
    }

    void* el_row = malloc(upd_schema->slot_size);
    void* el = malloc(comp_field.size);
    void* upd_el = malloc(upd_field.size);
    void* comp_val = malloc(comp_field.size);
    memcpy(comp_val, value, comp_field.size);

    /* Update */
    tab_for_each_row(upd_tab, upd_chunk,upd_chblix, el_row, upd_schema){
        memcpy(el, (char*)el_row + comp_field.offset, comp_field.size);
        if(comp_compare(db, type, el, comp_val, condition)){
            memcpy(upd_el, element, upd_field.size);
            if(tab_update_element(upd_tab, &upd_chblix, &upd_field, upd_el) == TABLE_FAIL){
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

int tab_delete_op(db_t* db,
                   table_t* table,
                   schema_t* schema,
                   field_t* field_comp,
                   condition_t condition,
                   void* value){

    void* el_row = malloc(schema->slot_size);
    void* el = malloc(field_comp->size);
    void* comp_val = malloc(field_comp->size);
    memcpy(comp_val, value, field_comp->size);
    int64_t counter = 0;

    /* Delete */
    tab_for_each_row(table, del_chunk, del_chblix, el_row, schema){
        counter++;
        memcpy(el, (char*)el_row + field_comp->offset, field_comp->size);
//        int64_t* id = el;
//        printf("c: %lld | b: %lld | id: %lld\n", del_chblix.chunk_idx, del_chblix.block_idx, *id);
        if(comp_compare(db, field_comp->type, el, comp_val, condition)){
            chblix_t temp = del_chblix;
            bool flag = false;
            if(del_chunk->num_of_free_blocks + 1 == del_chunk->capacity){
                int64_t next_chunk = del_chunk->next_page;
                temp = (chblix_t){.block_idx = -1, .chunk_idx=next_chunk};
                flag = true;
            }
            if(tab_delete_nova(table, del_chunk, &del_chblix) == TABLE_FAIL){
                logger(LL_ERROR, __func__, "Failed to delete row");
                return TABLE_FAIL;
            }
            if(flag){
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

table_t* tab_projection(db_t* db,
                   table_t* table,
                   schema_t* schema,
                   field_t* fields,
                   int64_t num_of_fields,
                   const char* name){

    /* Create new schema */
    schema_t* new_schema = sch_init();
    if(new_schema == NULL){
        logger(LL_ERROR, __func__, "Failed to create new schema");
        return NULL;
    }
    for(int64_t i = 0; i < num_of_fields; ++i){
        if(sch_add_field(new_schema, fields[i].name, fields[i].type, (int64_t)fields[i].size) == SCHEMA_FAIL){
            logger(LL_ERROR, __func__, "Failed to add field %s", fields[i].name);
            return NULL;
        }
    }

    /* Create new table */
    table_t* new_table = tab_init(db, name, new_schema);
    if(new_table == NULL){
        logger(LL_ERROR, __func__, "Failed to create new table");
        return NULL;
    }

    /* Create new row */
    void* row = malloc(new_schema->slot_size);

    /* Projection */

    tab_for_each_row(table, chunk, chblix, row, schema){
        for(int64_t i = 0; i < num_of_fields; ++i){
            memcpy((char*)row + fields[i].offset, (char*)row + fields[i].offset, fields[i].size);
        }
        chblix_t rowix = tab_insert(new_table, new_schema, row);
        if(chblix_cmp(&rowix, &CHBLIX_FAIL) == 0){
            logger(LL_ERROR, __func__, "Failed to insert row");
            return NULL;
        }
    }
    free(row);
    return new_table;
}

