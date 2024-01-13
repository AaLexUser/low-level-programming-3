#pragma once
#include "../data_type.h"
#include "backend/journal/varchar_mgr.h"
#include "core/page_pool/linked_blocks.h"
#include "core/page_pool/page_pool.h"
#include <stdint.h>
#include <string.h>

#define MAX_NAME_LENGTH 128
typedef struct field{
    linked_block_t lb_header;
    char name[MAX_NAME_LENGTH];
    datatype_t type;
    uint64_t size;
    uint64_t offset;
} field_t;

typedef struct schema{
    page_pool_t ppl_header;
    int64_t slot_size;
} schema_t;

typedef enum {SCHEMA_SUCCESS = 0, SCHEMA_FAIL = -1, SCHEMA_NOT_FOUND = -2} schema_status_t;

/**
 * @brief       Load a schema
 * @param[in]   schidx: index of the schema
 * @return      pointer to the schema on success, NULL on failure
 */

#define sch_load(schidx) ((schema_t*)lb_ppl_load(schidx))

/**
 * @brief       Delete a schema
 * @param[in]   schidx: index of the schema
 * @return      SCHEMA_SUCCESS on success, SCHEMA_FAIL on failure
 */

#define sch_delete(schidx) (lb_ppl_destroy((schidx)))

/**
 * @brief      Load field
 * @param[in]  schidx: index of the schema
 * @param[in]  fieldix: index of the field
 * @param[out] field: pointer to the field
 * @return     LB_SUCCESS on success, LB_FAIL on failure
 */

#define sch_field_load(schidx, fieldix, field) (lb_load((schidx),(fieldix), (linked_block_t*)(field)))

/**
 * @brief      Update field
 * @param[in]  schidx: index of the schema
 * @param[in]  fieldix: index of the field
 * @param[out] field: pointer to the field
 * @return     LB_SUCCESS on success, LB_FAIL on failure
 */

#define sch_field_update(schidx, fieldix, field) (lb_update((schidx), (fieldix), (linked_block_t*)(field)))



#define sch_add_int_field(schema, name) sch_add_field((schema), name, DT_INT, sizeof(int64_t))
#define sch_add_char_field(schema, name, size) sch_add_field((schema), name, DT_CHAR, size)
#define sch_add_varchar_field(schema, name) sch_add_field((schema), name, DT_VARCHAR, sizeof(vch_ticket_t))
#define sch_add_float_field(schema, name) sch_add_field((schema), name, DT_FLOAT, sizeof(float))
#define sch_add_bool_field(schema, name) sch_add_field((schema), name, DT_BOOL, sizeof(bool))

#define schema_index(schema) ((schema)->ppl_header.lp_header.page_index)


/**
 * @brief       For each field in a schema
 * @param[in]   sch: pointer to the schema
 * @param[in]   chunk: chunk
 * @param[in]   field: pointer to the field
 * @param[in]   chblix: chblix of the row
 * @param[in]   schidx: index of the schema
 */

#define sch_for_each(sch,chunk, field, chblix, schidx) \
    field_t field;                               \
    chunk_t* chunk = ppl_load_chunk(sch->ppl_header.head);                  \
    chblix_t chblix = lb_pool_start((page_pool_t*)sch, &chunk);\
    sch_field_load(schidx, &chblix, &field);\
    for(;\
    chblix_cmp(&chblix, &CHBLIX_FAIL) != 0 &&\
    sch_field_load(schidx, &chblix, &field) != LB_FAIL; \
    ++chblix.block_idx,  chblix = lb_nearest_valid_chblix((page_pool_t*)sch, chblix, &chunk))

void* sch_init(void);
int sch_add_field(schema_t* schema, const char* name, datatype_t type, int64_t size);
int sch_get_field(schema_t* schema, const char* name, field_t* field);
int sch_delete_field(schema_t* schema, const char* name);
