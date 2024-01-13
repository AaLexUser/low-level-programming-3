#include "comparator.h"

/**
 * @brief       Compare two values
 * @param[in]   db: pointer to database
 * @param[in]   type: type of the values
 * @param[in]   val1: pointer to the first value
 * @param[in]   val2: pointer to the second value
 * @return      data_t: the result of the comparison
 */

data_t comp_cmp(db_t* db, datatype_t type, void* val1, void* val2){
    data_t data;
    switch (type) {
        case DT_INT: {
            data.int_val = (*(int64_t *) val1 - *(int64_t *) val2);
            break;
        }
        case DT_FLOAT: {
            data.float_val = *(float *) val1 - *(float *) val2;
            break;
        }
        case DT_CHAR: {
            data.int_val = strcmp((char*)val1, (char*)val2);
            break;
        }
        case DT_BOOL: {
            data.int_val =  *(bool *) val1 - *(bool *) val2;
            break;
        }
        case DT_VARCHAR: {
            vch_ticket_t* vch1 = val1;
            vch_ticket_t* vch2 = val2;
            char* str1 = malloc(vch1->size);
            vch_get(db->varchar_mgr_idx, vch1, str1);
            char* str2 = malloc(vch2->size);
            vch_get(db->varchar_mgr_idx, vch2, str2);
            int res = strcmp(str1, str2);
            free(str1);
            free(str2);
            data.int_val = res;
            break;
        }
        case DT_UNKNOWN:
            data.int_val = 0;
            break;
    }
    return data;
}

/**
 * @brief       Compare two values on equality
 * @param       db: pointer to db
 * @param       type: type of the values
 * @param       val1: pointer to the first value
 * @param       val2: pointer to the second value
 * @return      1 if equal, 0 if not
 */

bool comp_eq(db_t* db, datatype_t type, void* val1, void* val2){
    data_t data = comp_cmp(db, type, val1, val2);
    switch (type) {
        case DT_INT: {
            return data.int_val == 0;
        }
        case DT_FLOAT: {
            return data.float_val == 0;
        }
        case DT_CHAR :
        case DT_BOOL:
        case DT_VARCHAR: {
            return data.int_val == 0;
        }
        case DT_UNKNOWN:
            return 0;
    }
}

/**
 * @brief       Compare two values on inequality
 * @param       db: pointer to db
 * @param       type: type of the values
 * @param       val1: pointer to the first value
 * @param       val2: pointer to the second value
 * @return      1 if not equal, 0 if equal
 */

bool comp_neq(db_t* db, datatype_t type, void* val1, void* val2) {
    return !comp_eq(db, type, val1, val2);
}

/**
 * @brief       Compare two values on less
 * @param       db: pointer to db
 * @param       type: type of the values
 * @param       val1: pointer to the first value
 * @param       val2: pointer to the second value
 * @return      1 if less, 0 if not
 */

bool comp_lt(db_t* db, datatype_t type, void* val1, void* val2) {
    data_t data = comp_cmp(db, type, val1, val2);
    switch (type) {
        case DT_INT: {
            return data.int_val < 0;
        }
        case DT_FLOAT: {
            return data.float_val < 0;
        }
        case DT_CHAR:
        case DT_BOOL:
        case DT_VARCHAR: {
            return data.int_val < 0;
        }
        case DT_UNKNOWN:
            return 0;
    }
}

/**
 * @brief       Compare two values on less or equal
 * @param       db: pointer to db
 * @param       type: type of the values
 * @param       val1: pointer to the first value
 * @param       val2: pointer to the second value
 * @return      1 if less or equal, 0 if not
 */

bool comp_le(db_t* db, datatype_t type, void* val1, void* val2) {
    return comp_lt(db, type, val1, val2) || comp_eq(db, type, val1, val2);
}

/**
 * @brief       Compare two values on greater
 * @param       db: pointer to db
 * @param       type: type of the values
 * @param       val1: pointer to the first value
 * @param       val2: pointer to the second value
 * @return      1 if greater, 0 if not
 */

bool comp_gt(db_t* db, datatype_t type, void* val1, void* val2) {
    return !comp_le(db, type, val1, val2);
}

/**
 * @brief       Compare two values on greater or equal
 * @param       db: pointer to db
 * @param       type: type of the values
 * @param       val1: pointer to the first value
 * @param       val2: pointer to the second value
 * @return      1 if greater or equal, 0 if not
 */

bool comp_ge(db_t* db, datatype_t type, void* val1, void* val2) {
    return !comp_lt(db, type, val1, val2);
}

/**
 * @brief       Compare two values
 * @param       db: pointer to db
 * @param[in]   type: type of data
 * @param[in]   val1: value 1
 * @param[in]   val2: value 2
 * @param[in]   cond: comparison condition
 * @return      true, or false depends on comparison condition
 */

bool comp_compare(db_t* db, datatype_t type, void* val1, void* val2, condition_t cond) {
    switch (cond) {
        case COND_EQ: {
            return comp_eq(db, type, val1, val2);
        }
        case COND_NEQ: {
            return comp_neq(db, type, val1, val2);
        }
        case COND_LT: {
            return comp_lt(db, type, val1, val2);
        }
        case COND_LTE: {
            return comp_le(db, type, val1, val2);
        }
        case COND_GT: {
            return comp_gt(db, type, val1, val2);
        }
        case COND_GTE: {
            return comp_ge(db, type, val1, val2);
        }
        default:
            return 0;
    }
}


