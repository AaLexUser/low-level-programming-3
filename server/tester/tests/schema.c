#include "../src/test.h"
#include "core/io/pager.h"
#include "backend/table/schema.h"

DEFINE_TEST(create_add_foreach_sch){
    assert(pg_init("test.db") == PAGER_SUCCESS);
    schema_t* schema = sch_init();
    sch_add_char_field(schema, "NAME", 10);
    sch_add_int_field(schema, "CREDIT");
    sch_add_float_field(schema, "DEBIT");
    sch_add_bool_field(schema, "STUDENT");
    sch_for_each(schema, chunk, field, chblix, schema_index(schema)){
        printf("%s\n",field.name);
    }
    pg_delete();
}

DEFINE_TEST(delete_field){
    assert(pg_init("test.db") == PAGER_SUCCESS);
    schema_t* schema = sch_init();
    if(schema == NULL){
       exit(EXIT_FAILURE);
    }
    sch_add_char_field(schema, "NAME", 10);
    sch_add_int_field(schema, "CREDIT");
    sch_add_float_field(schema, "DEBIT");
    sch_add_bool_field(schema, "STUDENT");
    sch_delete_field(schema, "STUDENT");
    field_t temp;
    int res = sch_get_field(schema, "STUDENT", &temp);
    assert(res == SCHEMA_NOT_FOUND);
    res = sch_get_field(schema, "DEBIT", &temp);
    assert(res == SCHEMA_SUCCESS);
    pg_delete();
}

int main(){
    RUN_SINGLE_TEST(create_add_foreach_sch);
    RUN_SINGLE_TEST(delete_field);
}