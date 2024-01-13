#pragma once
#include "journal/varchar_mgr.h"
#include <stdbool.h>
#include <stdlib.h>

typedef enum datatype {DT_UNKNOWN = -1,
                       DT_INT = 0,
                       DT_FLOAT = 1,
                       DT_VARCHAR = 2,
                       DT_CHAR = 3,
                       DT_BOOL = 4} datatype_t;

typedef union data {
    int64_t int_val;
    float float_val;
    char* char_val;
    bool bool_val;
    vch_ticket_t* vch_val;
} data_t;