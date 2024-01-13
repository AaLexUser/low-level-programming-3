#include "materializer.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    char _name[MAX_NAME_LENGTH];
    int64_t _index;
} tab_row_t;

int64_t materializer_init(void){
    schema_t* schema = sch_init();
    sch_add_char_field(schema, "NAME", MAX_NAME_LENGTH);
    sch_add_int_field(schema, "INDEX");
    matertab_t* material_tab = (matertab_t*)tab_base_init("MATERIALIZER", schema);
    material_tab->next_index = 0;
    return material_tab->table.ppl_header.lp_header.page_index;
}

table_t* materializer_materialize(int64_t mater_idx, schema_t* input_schema){
    matertab_t* material_tab = (matertab_t*)tab_load(mater_idx);
    if (!material_tab) {
        logger(LL_ERROR, __func__, "tab_load returned NULL");
        return NULL;
    }
    schema_t* material_schema = sch_load(material_tab->table.schidx);
    if (!material_schema) {
        logger(LL_ERROR, __func__, "sch_load returned NULL");
        return NULL;
    }
    tab_row_t row;
    snprintf(row._name, sizeof(row._name),"MATER_%"PRId64, material_tab->next_index);
    table_t* new_table = tab_base_init(row._name, input_schema);
    if (!new_table) {
        logger(LL_ERROR, __func__, "tab_base_init returned NULL");
        return NULL;
    }
    row._index = table_index(new_table);
    chblix_t res = tab_insert(&material_tab->table, material_schema, &row);

    if(chblix_cmp(&res, &CHBLIX_FAIL) == 0){
        logger(LL_ERROR, __func__, "Failed to insert row ");
        return NULL;
    }
    material_tab->next_index++;
    return new_table;
}
