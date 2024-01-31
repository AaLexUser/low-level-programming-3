#ifndef CONSTANT_VALUE_H
#define CONSTANT_VALUE_H

#include "exe_utils.h"

struct constant_val {
    db_t* db;
    datatype_t type;
    union {
        int64_t int_val;
        double float_val;
        bool bool_val;
        vch_ticket_t varchar_val;
    };
};


struct constant_val *init_constant(db_t *db, struct ast *constant);
#define GET_VALUE_PTR(cv, type) ((type == DT_INT) ? &((cv)->int_val) : \
                                  (type == DT_FLOAT) ? &((cv)->float_val) : \
                                  (type == DT_BOOL) ? &((cv)->bool_val) : \
                                  (type == DT_VARCHAR) ? &((cv)->varchar_val) : \
                                  NULL)

#endif