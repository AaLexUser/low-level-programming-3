#include "../src/test.h"
#include "backend/db/db.h"
#include "backend/journal/metatab.h"
#include "generator/table-gen.h"

DEFINE_TEST(generate_table){
    db_t* db = db_init("test.db");
    assert(db != NULL);
    gentab_mgr gentabMgr;
    gentabMgr.next_index = 0;
    table_t* table = gen_table(db, &gentabMgr, 100);
    schema_t* schema = sch_load(table->schidx);
    assert(table != NULL);
    tab_print(db, table, schema);
    assert(tab_drop(db, table) == TABLE_SUCCESS);
    db_drop();
}

int main(){
    RUN_SINGLE_TEST(generate_table);
    return 0;
}