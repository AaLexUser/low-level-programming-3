#include "table_base.h"
#include "utils/logger.h"
#include <stdio.h>

/**
 * @brief       Initialize a table
 * @param[in]   name: name of the table
 * @param[in]   schema: pointer to schema
 * @param[out]  table: pointer to allocated table
 * @return      index of the table on success, TABLE_FAIL on failure
 */

table_t* tab_base_init(const char* name, schema_t* schema){
    if(schema == NULL){
        logger(LL_ERROR, __func__, "Invalid argument, schema is NULL");
        return NULL;
    }
    int64_t tablix = lb_ppl_init((int64_t)schema->slot_size);
    if(tablix == LB_FAIL){
        logger(LL_ERROR, __func__, "Failed to initialize table %s", name);
        return NULL;
    }
    table_t* table = (table_t*)lb_ppl_load(tablix);
    if(table == NULL){
        logger(LL_ERROR, __func__, "Failed to load table %s", name);
        return NULL;
    }
    table->schidx = schema_index(schema);
    strncpy(table->name, name, MAX_NAME_LENGTH);
    return table;
}

/**
 * @brief       Insert a row
 * @param[in]   table: pointer to table
 * @param[in]   schema: pointer to schema
 * @param[in]   src: source
 * @return      chblix_t of row on success, CHBLIX_FAIL on failure
 */

chblix_t tab_insert(table_t* table, schema_t* schema, void* src){
    if(table == NULL){
        logger(LL_ERROR, __func__, "Invalid argument: table is NULL");
        return CHBLIX_FAIL;
    }

    chblix_t rowix = lb_alloc(&table->ppl_header);
//    printf("c: %lld | b: %lld | ", rowix.chunk_idx, rowix.block_idx);

    if(chblix_cmp(&rowix, &CHBLIX_FAIL) == 0){
        logger(LL_ERROR, __func__, "Failed to allocate row");
        return CHBLIX_FAIL;
    }

    if(lb_write(&table->ppl_header, &rowix, src, schema->slot_size, 0) == LB_FAIL){
        logger(LL_ERROR, __func__, "Failed to write row");
        return CHBLIX_FAIL;
    }

    return rowix;

}

/**
 * @brief       Select a row
 * @param[in]   tablix: index of the table
 * @param[in]   rowix: chblix of the row
 * @param[out]  dest: destination
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int tab_select_row(int64_t tablix, chblix_t* rowix, void* dest){
    table_t* table = tab_load(tablix);
    if(table == NULL){
        logger(LL_ERROR, __func__, "Failed to load table %ld", tablix);
        return TABLE_FAIL;
    }

    schema_t* schema = sch_load(table->schidx);
    if(schema == NULL){
        logger(LL_ERROR, __func__, "Failed to load schema %ld", table->schidx);
        return TABLE_FAIL;
    }

    if(lb_read(tablix, rowix, dest, schema->slot_size, 0) == LB_FAIL){
        logger(LL_ERROR, __func__, "Failed to read row");
        return TABLE_FAIL;
    }
    return TABLE_SUCCESS;
}

/**
 * @brief       Delete a row
 * @param       table: pointer to table
 * @param       chunk: pointer to chunk
 * @param       rowix: chblix of the row
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int tab_delete_nova(table_t* table, chunk_t* chunk, chblix_t* rowix){
    /* Loading Linked Block */
    linked_block_t* lb = malloc(table->ppl_header.block_size); /* Don't forget to free it */
    if (lb_load_nova_pppp(&table->ppl_header,chunk, rowix, lb) == LB_FAIL) {
        logger(LL_ERROR, __func__, "Unable to read block");
        free(lb);
        return TABLE_FAIL;
    }
    if(lb_dealloc_nova(&table->ppl_header, lb) == LB_FAIL){
        logger(LL_ERROR, __func__, "Failed to deallocate row");
        return TABLE_FAIL;
    }
    free(lb);
    return TABLE_SUCCESS;
}

int tab_delete_row(table_t* table, chblix_t* rowix){
    chunk_t* chunk = ppl_load_chunk(rowix->chunk_idx);
    if(tab_delete_nova(table, chunk, rowix) == TABLE_FAIL){
        logger(LL_ERROR, __func__, "Failed to delete row");
        return TABLE_FAIL;
    }
    return TABLE_SUCCESS;
}

/**
 * @brief       Update a row
 * @param[in]   table: pointer to table
 * @param[in]   schema: pointer to schema
 * @param[in]   rowix: chblix of the row
 * @param[in]   row: row to be written
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int tab_update_row(table_t* table, schema_t* schema, chblix_t* rowix, void* row){
    if(table == NULL){
        logger(LL_ERROR, __func__, "Invalid argument: table is NULL");
        return TABLE_FAIL;
    }

    if(schema == NULL){
        logger(LL_ERROR, __func__, "Invalid argument: schema is NULL");
        return TABLE_FAIL;
    }

    if(lb_write(&table->ppl_header, rowix, row, schema->slot_size, 0) == LB_FAIL){
        logger(LL_ERROR, __func__, "Failed to write row");
        return TABLE_FAIL;
    }
    return TABLE_SUCCESS;
}

/**
 * @brief       Update an element
 * @param[in]   table: pointer to table
 * @param[in]   rowix: chblix of the row
 * @param[in]   field: pointer to the field
 * @param[in]   element: pointer to the element to be written
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int tab_update_element(table_t* table, chblix_t* rowix, field_t* field, void* element){
    if(lb_write(&table->ppl_header, rowix, element, (int64_t) field->size, (int64_t) field->offset) == LB_FAIL){
        logger(LL_ERROR, __func__, "Failed to write row");
        return TABLE_FAIL;
    }
    return TABLE_SUCCESS;
}


/**
 * @brief       Get an element from row
 * @param[in]   tablix: index of the table
 * @param[in]   rowix: chblix of the row
 * @param[in]   field: pointer to the field
 * @param[out]  element: pointer to the element to be written
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int tab_get_element(int64_t tablix, chblix_t* rowix, field_t* field, void* element){
    table_t* table = tab_load(tablix);
    if(table == NULL){
        logger(LL_ERROR, __func__, "Failed to load table %ld", tablix);
        return TABLE_FAIL;
    }

    schema_t* schema = sch_load(table->schidx);
    if(schema == NULL){
        logger(LL_ERROR, __func__, "Failed to load schema %ld", table->schidx);
        return TABLE_FAIL;
    }

    if(lb_read(tablix, rowix, element, (int64_t)field->size, (int64_t)field->offset) == LB_FAIL){
        logger(LL_ERROR, __func__, "Failed to read row");
        return TABLE_FAIL;
    }
    return TABLE_SUCCESS;
}





