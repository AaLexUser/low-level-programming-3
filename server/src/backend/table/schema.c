#include "schema.h"

/**
 * @brief       Initialize a schema
 * @return      pointer to schema on success, NULL on failure
 */

void* sch_init(void){
    int64_t schidx = lb_ppl_init(sizeof(field_t) - sizeof(linked_block_t));
    schema_t* sch = (schema_t*)lb_ppl_load(schidx);
    if(sch == NULL) {
        logger(LL_ERROR, __func__, "Failed to load schema %ld", schidx);
        return NULL;
    }
    sch->slot_size = 0;
    return sch;
}

/**
 * @brief       Add a field
 * @param[in]   schema: pointer to schema
 * @param[in]   name: name of the field
 * @param[in]   type: type of the field
 * @param[in]   size: size of the type
 * @return      SCHEMA_SUCCESS on success, SCHEMA_FAIL on failure
 */

int sch_add_field(schema_t* schema, const char* name, datatype_t type, int64_t size){
    if(schema == NULL) {
        logger(LL_ERROR, __func__, "Invalid argument: schema is NULL");
        return SCHEMA_FAIL;
    }

    chblix_t fieldix = lb_alloc_m(&schema->ppl_header, sizeof(field_t));
    if(chblix_cmp(&fieldix, &CHBLIX_FAIL) == 0){
        logger(LL_ERROR, __func__, "Failed to allocate field %s", name);
        return SCHEMA_FAIL;
    }
    field_t field;
    if(sch_field_load(schema_index(schema), &fieldix, &field) == LB_FAIL){
        logger(LL_ERROR, __func__, "Failed to load field %s", name);
        return SCHEMA_FAIL;
    }
    strncpy(field.name, name, MAX_NAME_LENGTH);
    field.type = type;
    field.size = size;
    field.offset = schema->slot_size;
    schema->slot_size += size;
    if(sch_field_update(schema_index(schema), &fieldix, &field) == LB_FAIL){
        logger(LL_ERROR, __func__, "Failed to update field %s", name);
        return SCHEMA_FAIL;
    }
    return SCHEMA_SUCCESS;
}

/**
 * @brief       Get a field
 * @param[in]   schema: pointer to the schema
 * @param[in]   name: name of the field
 * @param[out]  field: pointer to destination field
 * @return      SCHEMA_SUCCESS on success, SCHEMA_FAIL on failure
 */

int sch_get_field(schema_t* schema, const char* name, field_t* field){
    if(schema == NULL) {
        logger(LL_ERROR, __func__, "Invalid argument: schema os NULL");
        return SCHEMA_FAIL;
    }

    sch_for_each(schema, chunk, fieldi, chblix, schema->ppl_header.lp_header.page_index){
        if(strcmp(fieldi.name, name) == 0){
            *field = fieldi;
            return SCHEMA_SUCCESS;
        }
    }

    logger(LL_ERROR, __func__, "Failed to find field %s", name);
    return SCHEMA_NOT_FOUND;
}

/**
 * @brief       Delete a field
 * @warning     This function delete field, but dont touch offsets in other fields in schema.
 * @param[in]   schema: pointer to schema
 * @param[in]   name: name of the field
 * @return      SCHEMA_SUCCESS on success, SCHEMA_FAIL on failure
 */

int sch_delete_field(schema_t* schema, const char* name){
    if(schema == NULL) {
        logger(LL_ERROR, __func__, "Invalid argument: schema is NULL");
        return SCHEMA_FAIL;
    }

    sch_for_each(schema, chunk, field, chblix, schema_index(schema)){
        if(strcmp(field.name, name) == 0){
            if(lb_dealloc(schema_index(schema), &field.lb_header.chblix) == LB_FAIL){
                logger(LL_ERROR, __func__, "Failed to deallocate field %s", name);
                return SCHEMA_FAIL;
            }
            return SCHEMA_SUCCESS;
        }
    }

    logger(LL_ERROR, __func__, "Failed to find field %s", name);
    return SCHEMA_FAIL;
}

