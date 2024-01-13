#include "table-gen.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief       Generate empty table
 * @param[in]   mtabidx: meta table index
 * @param[in]   gentabMgr_ptr: pointer to generate table manager
 * @return      return tablix on success, TABLE_FAIL otherwise
 */

table_t* gen_empty_table(db_t* db, gentab_mgr* gentabMgr_ptr, schema_t* schema){
    int64_t number_of_fields = 1 + arc4random_uniform(80);
    for(int64_t i = 0; i < number_of_fields; ++i){
        int64_t size = 8 + i%10;
        char* field_name = malloc(size);
        char str[size];
        snprintf(str, size, "FIELD_%llu", i);
        strncpy(field_name, str, size);
        int64_t field_size =  1 + arc4random_uniform(100);
        datatype_t type = arc4random_uniform(5);
        switch (type) {
            case DT_VARCHAR:
                sch_add_varchar_field(schema, field_name);
                break;
            case DT_CHAR:
                sch_add_char_field(schema, field_name, field_size);
                break;
            case DT_INT:
                sch_add_int_field(schema, field_name);
                break;
            case DT_FLOAT:
                sch_add_float_field(schema, field_name);
                break;
            case DT_BOOL:
                sch_add_bool_field(schema, field_name);
                break;
            default:
                break;
        }
        free(field_name);
    }
    int64_t size = 6 + gentabMgr_ptr->next_index % 10;
    char* table_name = malloc(size);
    char str[size];
    snprintf(str, size, "GEN_%llu", gentabMgr_ptr->next_index);
    strncpy(table_name, str, size);
    gentabMgr_ptr->next_index++;
    table_t* table = tab_init(db, table_name, schema);
    free(table_name);
    return table;
}

/**
 * @brief       Generate random element
 * @param[in]   field: pointer to field
 * @return      pointer to element on success, NULL otherwise
 */

int gen_fill_element(db_t* db, field_t* field, uint8_t* element){
    switch (field->type) {
        case DT_INT: {
            int64_t rand_num = INT16_MIN + arc4random_uniform(INT16_MAX - INT16_MIN + 1);
            memcpy(element, &rand_num, sizeof(int64_t));
            break;
        }
        case DT_FLOAT: {
            float rand_float =((float)arc4random() / (float)UINT32_MAX) * (INT16_MAX - INT16_MIN) + INT16_MIN;
            memcpy(element, &rand_float, sizeof(float));
            break;
        }
        case DT_BOOL: {
            int r = (int)arc4random();
            bool rand_bool = r % 2 == 0;
            memcpy(element, &rand_bool, sizeof(bool));
            break;
        }
        case DT_CHAR: {
            char* rand_char = malloc(field->size);
            for(int64_t i = 0; i < field->size; ++i){
                rand_char[i] = (char)((uint32_t)('a') + arc4random() % 26);
            }
            memcpy(element, rand_char, field->size);
            free(rand_char);
            break;
        }
        case DT_VARCHAR: {
            size_t size = 1 + arc4random_uniform(257);
            char* rand_varchar = malloc(size);
            for(int64_t i = 0; i < size; ++i){
                rand_varchar[i] = (char)((uint32_t)'a' + arc4random_uniform(26));
            }
            vch_ticket_t ticket = vch_add(db->varchar_mgr_idx, rand_varchar);
            if(chblix_cmp(&ticket.block, &CHBLIX_FAIL) == 0){
                logger(LL_ERROR, __func__, "Failed to add varchar");
                return -1;
            }

            memcpy(element, &ticket, sizeof(vch_ticket_t));
            free(rand_varchar);
            break;
        }
        default:
            return -1;
    }
    return 0;
}

/**
 * @brief       Generate random row
 * @param       schema: pointer to schema
 * @return      pointer to row on success, NULL otherwise
 */

int gen_fill_row(db_t* db, schema_t* schema, uint8_t* row){
    sch_for_each(schema, chunk, field, chblix, schema_index(schema)){
        uint8_t* element = malloc(field.size);
        if(gen_fill_element(db, &field, element)!=0){
            logger(LL_ERROR, __func__, "Failed to generate element");
            free(element);
            return -1;
        }
        memcpy(row + field.offset, element, field.size);
        free(element);
    }
    return 0;
}

/**
 * @brief       Generate random rows
 * @param[in]   tablix: index of the table
 * @param[in]   number: number of rows to generate
 * @return      TABLE_SUCCESS on success, TABLE_FAIL otherwise
 */

int gen_fill_rows(db_t* db, table_t* table, schema_t* schema, int64_t number){
    for(int64_t i = 0; i < number; ++i){
        uint8_t* row = malloc(schema->slot_size);
        gen_fill_row(db, schema, row);
        if(row == NULL){
            logger(LL_ERROR, __func__, "Failed to generate row");
            return TABLE_FAIL;
        }
        chblix_t res = tab_insert(table, schema, row);
        if(chblix_cmp(&res, &CHBLIX_FAIL) == 0){
            logger(LL_ERROR, __func__, "Failed to insert row");
            return TABLE_FAIL;
        }
        free(row);
    }
    return TABLE_SUCCESS;
}

table_t* gen_table(db_t* db, gentab_mgr* gentabMgr_ptr, int64_t min_number_of_rows){
    schema_t* schema = sch_init();
    table_t* table = gen_empty_table(db, gentabMgr_ptr, schema);
    if(table == NULL){
        logger(LL_ERROR, __func__, "Failed to generate empty table");
        return NULL;
    }
    int64_t number_of_rows = min_number_of_rows + arc4random_uniform(min_number_of_rows * 10 - min_number_of_rows + 1);
    if(gen_fill_rows(db, table, schema, number_of_rows) == TABLE_FAIL){
        logger(LL_ERROR, __func__, "Failed to generate rows");
        return NULL;
    }
    return table;
}

