#include "metatab.h"
#include "backend/table/schema.h"
#include "utils/logger.h"

/**
 * @brief       Initialize the metatable
 * @param[out]  table: pointer to table;
 * @return      pointer to table on success, NULL otherwise
 */

table_t* mtab_init(void){
    schema_t* schema = sch_init();
    sch_add_char_field(schema, "NAME", MAX_NAME_LENGTH);
    sch_add_int_field(schema, "INDEX");
    table_t* table = tab_base_init("METATABLE", schema);
    tab_row(
            char NAME[MAX_NAME_LENGTH];
            int64_t INDEX;
            );
    strncpy(row.NAME, "METATABLE", MAX_NAME_LENGTH);
    row.INDEX = table_index(table);
    chblix_t res = tab_insert(table, schema, &row);
    if(chblix_cmp(&res, &CHBLIX_FAIL) == 0){
        logger(LL_ERROR, __func__, "Failed to insert row ");
        return NULL;
    }
    return table;
}

/**
 * @brief       Find table index by name
 * @param       metatab_idx: index of metatab
 * @param       name: name of the table
 * @return      index of the table on success, TABLE_FAIL on failure
 */

int64_t mtab_find_table_by_name(int64_t metatab_idx, const char* name){
    table_t* meta_table = tab_load(metatab_idx);
    if(meta_table == NULL){
        logger(LL_ERROR, __func__, "Invalid argument: meta_table is NULL");
        return TABLE_FAIL;
    }
    tab_row(
            char NAME[MAX_NAME_LENGTH];
            int64_t INDEX;
            );
    schema_t* schema = sch_load(meta_table->schidx);
    tab_for_each_row(meta_table, chunk, chblix, &row, schema){
        if(strcmp(name,row.NAME) == 0){
            return row.INDEX;
        }
    }
    return TABLE_FAIL;
}

/**
 * @brief       Add a table to the metatable
 * @param[in]   metatab_idx: index of metatab
 * @param[in]   name: name of the table
 * @param[in]   index: index of the table
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int mtab_add(int64_t metatab_idx, const char* name, int64_t index){
    table_t* meta_table = tab_load(metatab_idx);
    if(meta_table == NULL){
        logger(LL_ERROR, __func__, "Invalid argument: meta_table is NULL");
        return TABLE_FAIL;
    }

    tab_row(
            char NAME[MAX_NAME_LENGTH];
            int64_t INDEX;
    );
    strncpy(row.NAME, name, MAX_NAME_LENGTH);
    row.INDEX = index;
    schema_t* schema = sch_load(meta_table->schidx);
    chblix_t res = tab_insert(meta_table, schema, &row);
    if(chblix_cmp(&res, &CHBLIX_FAIL) == 0){
        logger(LL_ERROR, __func__, "Failed to insert row ");
        return TABLE_FAIL;
    }
    return TABLE_SUCCESS;
}

/**
 * @brief       Delete a table from the metatable
 * @param[in]   metatab_idx: index of metatab
 * @param[in]   index: index of the table
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int mtab_delete(int64_t metatab_idx, int64_t index){
    table_t* meta_table = tab_load(metatab_idx);
    if(meta_table == NULL){
        logger(LL_ERROR, __func__, "Invalid argument: meta_table is NULL");
        return TABLE_FAIL;
    }
    tab_row(
            char NAME[MAX_NAME_LENGTH];
            int64_t INDEX;
    );
    schema_t* schema = sch_load(meta_table->schidx);
    tab_for_each_row(meta_table, chunk, rowix, &row, schema) {
        if(row.INDEX == index){
            if (tab_delete_nova(meta_table,chunk, &rowix) == TABLE_FAIL) {
                logger(LL_ERROR, __func__, "Failed to delete row ");
                return TABLE_FAIL;
            }
            return TABLE_SUCCESS;
        }
    }
    return TABLE_SUCCESS;
}
