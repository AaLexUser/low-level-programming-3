#include "../src/test.h"
#include "core/io/pager.h"
#include "backend/table/schema.h"
#include "backend/table/table.h"
#ifdef LOGGER_LEVEL
#undef LOGGER_LEVEL
#endif
#define LOGGER_LEVEL 2

static void insert_data(table_t* table, schema_t* schema, int64_t count){
    tab_row(
            char NAME[10];
            char SURNAME[10];
            int64_t CREDIT;
            float DEBIT;
            bool STUDENT;
    );
    for(int i = 0; i < count; i++){
        strncpy(row.NAME, "Alex", 10);
        strncpy(row.SURNAME, "Smith", 10);
        row.CREDIT = 10;
        row.DEBIT = 10.5f;
        row.STUDENT = true;
        chblix_t res = tab_insert(table, schema, &row);
        strncpy(row.NAME, "Nick", 10);
        strncpy(row.SURNAME, "Johnson", 10);
        row.CREDIT = 20;
        row.DEBIT = 20.5f;
        row.STUDENT = false;
        res = tab_insert(table, schema, &row);
        strncpy(row.NAME, "John", 10);
        strncpy(row.SURNAME, "Doe", 10);
        row.CREDIT = 30;
        row.DEBIT = 30.5f;
        row.STUDENT = true;
        res = tab_insert(table, schema, &row);
    }
}

static schema_t* init_schema(void){
    schema_t* schema = sch_init();
    if(schema == NULL){
        exit(EXIT_FAILURE);
    }
    const size_t char_count = 10;
    sch_add_char_field(schema, "NAME", char_count);
    sch_add_char_field(schema, "SURNAME", char_count);
    sch_add_int_field(schema, "CREDIT");
    sch_add_float_field(schema, "DEBIT");
    sch_add_bool_field(schema, "STUDENT");
    return schema;
}

table_t* table_bank(db_t* db, int count){
    schema_t* schema = init_schema();
    table_t* table = tab_init(db, "BANK", schema);
    if(table == NULL){
        logger(LL_ERROR, __func__, "Unable to init table");
        exit(EXIT_FAILURE);
    }
    insert_data(table, schema, count);
    return table;
}

table_t* table_student(db_t* db, int count){
    schema_t* schema =  sch_init();
    sch_add_int_field(schema, "ID");
    sch_add_char_field(schema, "NAME", 10);
    sch_add_float_field(schema, "SCORE");
    sch_add_bool_field(schema, "PASS");
    table_t* table = tab_init(db, "STUDENTS", schema);
    tab_row(
            int64_t ID;
            char NAME[10];
            float SCORE;
            bool PASS;
    );
    row.ID = 1;
    strncpy(row.NAME,"John", 10);
    row.SCORE = 10.5f;
    row.PASS = true;
    chblix_t res = tab_insert(table, schema, &row);

    row.ID = 2;
    strncpy(row.NAME,"Nick", 10);
    row.SCORE = 20.5f;
    row.PASS = false;
    res = tab_insert(table, schema, &row);

    row.ID = 3;
    strncpy(row.NAME,"Alex", 10);
    row.SCORE = 30.5f;
    row.PASS = true;
    res = tab_insert(table,schema,  &row);

    row.ID = 4;
    strncpy(row.NAME,"Smith", 10);
    row.SCORE = 40.5f;
    row.PASS = false;
    res = tab_insert(table,schema,  &row);

    return table;
}


DEFINE_TEST(create_add_foreach){
    db_t* db = db_init("test.db");
    schema_t* schema = init_schema();
    table_t* table = tab_init(db, "test", schema);
    insert_data(table, schema, 1);
    field_t field;
    sch_get_field(schema, "CREDIT", &field);
    int64_t* element = malloc(field.size);
    int64_t ans[3];
    int counter = 0;
    tab_for_each_element(table,chunk, chblix, element, (&field)){
        ans[counter++] = *element;
    }
    assert(ans[0] == 10);
    assert(ans[1] == 20);
    assert(ans[2] == 30);
    free(element);
    db_drop();
}

DEFINE_TEST(update) {
    db_t* db = db_init("test.db");
    schema_t* schema = init_schema();
    table_t* table = tab_init(db, "test", schema);
    insert_data(table, schema, 1);
    field_t field;
    sch_get_field(schema, "CREDIT", &field);
    int64_t element = 30;
    chblix_t res = tab_get_row(db,table, schema, &field, &element, DT_INT);
    assert(chblix_cmp(&res, &CHBLIX_FAIL) != 0);
    int64_t new_element = 100;
    assert(tab_update_element(table, &res, &(field), &new_element) == TABLE_SUCCESS);
    int64_t read_element;
    assert(tab_get_element(table_index(table), &res, &(field), &read_element) == TABLE_SUCCESS);
    assert(read_element == 100);
    db_drop();
}

DEFINE_TEST(delete){
    db_t* db = db_init("test.db");
    schema_t* schema = init_schema();
    table_t* table = tab_init(db, "test", schema);

    insert_data(table, schema, 1);
    field_t field;
    sch_get_field(schema, "CREDIT", &field);
    int64_t element = 30;
    chblix_t res = tab_get_row(db,table, schema, &field, &element, DT_INT);
    chunk_t* chunk = ppl_load_chunk(res.chunk_idx);
    assert(chblix_cmp(&res, &CHBLIX_FAIL) != 0);
    assert(tab_delete_nova(table, chunk, &res) == TABLE_SUCCESS);
    res = tab_get_row(db,table, schema, &field, &element, DT_INT);
    assert(chblix_cmp(&res, &CHBLIX_FAIL) == 0);
    db_drop();
}

DEFINE_TEST(get_table_after_close){
    db_t* db = db_init("test.db");
    table_t* table = table_student(db, 1);
    db_close();
    db = db_init("test.db");
    int64_t tabix = mtab_find_table_by_name(db->meta_table_idx, "STUDENTS");
    table_t *table2 = tab_load(tabix);
    assert(table2 != NULL);
    db_drop();
}

DEFINE_TEST(varchar){
    db_t* db = db_init("test.db");

    char* big_string = "This is a big string\n "
                       "with multiple lines\n "
                       "and it is very long\n"
                       "and it is very long\n";

    schema_t* schema = sch_init();
    sch_add_varchar_field(schema, "NAME");
    sch_add_varchar_field(schema, "SURNAME");
    sch_add_varchar_field(schema, "BIG_STRING");
    table_t* table = tab_init(db, "test", schema);
    tab_row(
            vch_ticket_t NAME;
            vch_ticket_t SURNAME;
            vch_ticket_t BIG_STRING;
    );
    row.NAME = vch_add(db->varchar_mgr_idx, "Alex");
    row.SURNAME = vch_add(db->varchar_mgr_idx, "Smith");
    row.BIG_STRING = vch_add(db->varchar_mgr_idx, big_string);
    chblix_t res = tab_insert(table, schema, &row);
    assert(chblix_cmp(&res, &CHBLIX_FAIL) != 0);
    field_t field;
    sch_get_field(schema, "BIG_STRING", &field);
    vch_ticket_t* element = malloc(field.size);
    tab_for_each_element(table, chunk, chblix, element, &field){
        char* str = malloc(element->size);
        vch_get(db->varchar_mgr_idx, element, str);
        assert(!strcmp(big_string, str));
        free(str);
    }
    free(element);
    db_drop();
}

DEFINE_TEST(several_tables){
    db_t* db = db_init("test.db");

    schema_t* schema = sch_init();
    sch_add_varchar_field(schema, "NAME");
    sch_add_varchar_field(schema, "SURNAME");
    tab_row(
            vch_ticket_t NAME;
            vch_ticket_t SURNAME;
            vch_ticket_t BIG_STRING;
    );
    sch_add_varchar_field(schema, "BIG_STRING");
    table_t* table1 = tab_init(db, "BIG_STR", schema);


    schema_t* schema2 = sch_init();
    sch_add_int_field(schema2, "ID");
    sch_add_varchar_field(schema2, "NAME");
    sch_add_float_field(schema2, "SCORE");
    sch_add_bool_field(schema2, "PASS");
    table_t* table2 = tab_init(db, "STUDENTS", schema2);

    int64_t read_tablix1 = mtab_find_table_by_name(db->meta_table_idx, "BIG_STR");
    assert(read_tablix1 == table_index(table1));

    int64_t read_tablix2 = mtab_find_table_by_name(db->meta_table_idx, "STUDENTS");
    assert(read_tablix2 == table_index(table2));

    db_drop();
}

DEFINE_TEST(print){
    db_t* db = db_init("test.db");
    schema_t* schema = init_schema();
    table_t* table = tab_init(db, "test", schema);
    insert_data(table, schema, 2);
    tab_print(db, table, schema);
    db_drop();
}

DEFINE_TEST(join){
    db_t* db = db_init("test.db");
    table_t* left = table_bank(db, 1);
    schema_t* left_schema = sch_load(left->schidx);
    tab_print(db, left, left_schema);
    table_t* right = table_student(db, 1);
    schema_t* right_schema = sch_load(right->schidx);
    tab_print(db, right, right_schema);
    field_t left_field;
    sch_get_field(left_schema, "NAME", &left_field);
    field_t right_field;
    sch_get_field(right_schema, "NAME", &right_field);
    table_t* join_tablix = tab_join_on_field(db,
                                             left,
                                             left_schema,
                                             right,
                                             right_schema,
                                             &left_field,
                                             &right_field, "JOIN");
    assert(join_tablix != NULL);
    tab_print(db, join_tablix, sch_load(join_tablix->schidx));
    db_drop();
}

DEFINE_TEST(select){
    db_t* db = db_init("test.db");
    table_t* table = table_student(db, 1);
    schema_t *schema = sch_load(table->schidx);
    float value = 20.0f;
    field_t sel_field;
    assert(sch_get_field(schema, "SCORE", &sel_field) == SCHEMA_SUCCESS);
    table_t* sel_table_t = tab_select_op(db, table, schema, &sel_field, "SELECT",  COND_GT, &value, DT_FLOAT);
    tab_print(db,sel_table_t, sch_load(sel_table_t->schidx));
    schema_t* sel_schema = sch_load(sel_table_t->schidx);
    field_t* field = malloc(sizeof(field_t));
    assert(sch_get_field(sel_schema,  "SCORE", field) == SCHEMA_SUCCESS);
    float element;
    tab_for_each_element(sel_table_t,chunk, chblix, &element, field){
        assert(element > value);
    }
    tab_drop(db, sel_table_t);

    sel_table_t = tab_select_op(db, table, schema, &sel_field, "SELECT", COND_GTE, &value, DT_FLOAT);
    assert(sch_get_field(sel_schema,  "SCORE", field) == SCHEMA_SUCCESS);
    tab_for_each_element(sel_table_t, chunk2, chblix2, &element, field){
        assert(element >= value);
    }
    tab_drop(db, sel_table_t);

    sel_table_t = tab_select_op(db,table, schema, &sel_field, "SELECT", COND_LT, &value, DT_FLOAT);
    sel_schema = sch_load(sel_table_t->schidx);
    assert(sch_get_field(sel_schema,  "SCORE", field) == SCHEMA_SUCCESS);
    tab_for_each_element(sel_table_t, chunk3, chblix3, &element, field){
        assert(element < value);
    }
    tab_drop(db, sel_table_t);

    sel_table_t = tab_select_op(db,table, schema, &sel_field, "SELECT", COND_LTE, &value, DT_FLOAT);
    sel_schema = sch_load(sel_table_t->schidx);
    assert(sch_get_field(sel_schema,  "SCORE", field) == SCHEMA_SUCCESS);
    tab_for_each_element(sel_table_t, chunk4, chblix4, &element, field){
        assert(element <= value);
    }
    tab_drop(db, sel_table_t);

    value = 10.5f;
    sel_table_t = tab_select_op(db,table, schema, &sel_field, "SELECT", COND_NEQ, &value, DT_FLOAT);
    sel_schema = sch_load(sel_table_t->schidx);
    assert(sch_get_field(sel_schema,  "SCORE", field) == SCHEMA_SUCCESS);
    tab_for_each_element(sel_table_t, chunk5, chblix5, &element, field){
        assert(element != value);
    }

    free(field);

    tab_drop(db, sel_table_t);
    db_drop();
}

DEFINE_TEST(update_row_op){
    db_t* db = db_init("test.db");
    assert(db != NULL);
    table_t* table = table_student(db, 1);
    float value = 20.5f;
    schema_t* schema = sch_load(table->schidx);
    tab_row(
            int64_t ID;
            char NAME[10];
            float SCORE;
            bool PASS;
            );
    row.ID = 10;
    strncpy(row.NAME,"Nick", 10);
    row.SCORE = 10.5f;
    row.PASS = true;
    field_t field;
    sch_get_field(schema, "SCORE", &field);
    int res = tab_update_row_op(db,table, schema, &field, COND_EQ, &value, DT_FLOAT, &row);
    assert(res == TABLE_SUCCESS);
    bool flag = false;
    tab_for_each_row(table, chunk, chblix, &row, schema){
        if(row.ID == 10){
            assert(row.SCORE == 10.5f);
            flag = true;
        }
    }
    assert(flag);
    tab_drop(db,table);
    db_drop();
}

DEFINE_TEST(update_element_op){
    db_t* db = db_init("test.db");
    assert(db != NULL);
    table_t* table = table_student(db, 1);
    char* value = malloc(10);
    strncpy(value, "Nick", 10);
    schema_t* schema = sch_load(table->schidx);
    tab_row(
            int64_t ID;
            char NAME[10];
            float SCORE;
            bool PASS;
    );
    float element = 10.5f;
    int res = tab_update_element_op(db,table_index(table), &element, "SCORE", "NAME", COND_EQ, value, DT_CHAR);
    assert(res == TABLE_SUCCESS);
    field_t field;
    sch_get_field(schema, "SCORE", &field);
    bool flag = false;
    tab_for_each_row(table, chunk, chblix, &row, schema){
        if(strcmp(row.NAME, "Nick") == 0){
            assert(row.SCORE == 10.5f);
            flag = true;
        }
    }
    assert(flag);
    free(value);
    tab_drop(db,table);
    db_drop();
}

DEFINE_TEST(delete_op){
    db_t* db = db_init("test.db");
    assert(db != NULL);
    table_t* table = table_student(db, 1);
    char* value = malloc(10);
    strncpy(value, "Nick", 10);
    schema_t* schema = sch_load(table->schidx);
    tab_row(
            int64_t ID;
            char NAME[10];
            float SCORE;
            bool PASS;
    );
    field_t delete_field;
    assert(sch_get_field(schema, "NAME", &delete_field) == SCHEMA_SUCCESS);
    int res = tab_delete_op(db,table, schema, &delete_field, COND_EQ, value);
    assert(res == TABLE_SUCCESS);
    field_t field;
    sch_get_field(schema, "SCORE", &field);
    tab_for_each_row(table, chunk2, chblix, &row, schema){
        if(strcmp(row.NAME, "Nick") == 0){
            assert(false);
        }
    }
    free(value);
    tab_drop(db,table);
    db_drop();
}




int main(){
    RUN_SINGLE_TEST(create_add_foreach);
    RUN_SINGLE_TEST(update);
    RUN_SINGLE_TEST(delete);
    RUN_SINGLE_TEST(get_table_after_close);
    RUN_SINGLE_TEST(varchar);
    RUN_SINGLE_TEST(several_tables);
    RUN_SINGLE_TEST(print);
    RUN_SINGLE_TEST(join);
    RUN_SINGLE_TEST(select);
    RUN_SINGLE_TEST(update_row_op);
    RUN_SINGLE_TEST(update_element_op);
    RUN_SINGLE_TEST(delete_op);
}
