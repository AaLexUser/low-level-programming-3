#include "constant_value.h"

struct constant_val *init_constant(db_t *db, struct ast *constant) {
    if (constant->nodetype < 0 || constant->nodetype >= sizeof(datatype_lookup) / sizeof(datatype_lookup[0])) {
        logger(LL_ERROR, __func__, "Invalid constant type %d", constant->nodetype);
        return NULL;
    }

    struct constant_val *constant_val = malloc(sizeof(struct constant_val));
    if (constant_val == NULL) {
        logger(LL_ERROR, __func__, "Failed to allocate memory for constant");
        return NULL;
    }

    constant_val->db = db;
    constant_val->type = datatype_lookup[constant->nodetype];

    switch (constant_val->type) {
        case DT_INT:
            constant_val->int_val = ((struct nint *) constant)->value;
            break;
        case DT_FLOAT:
            constant_val->float_val = ((struct nfloat *) constant)->value;
            break;
        case DT_BOOL:
            constant_val->bool_val = ((struct nint *) constant)->value;
            break;
        case DT_VARCHAR:
            constant_val->varchar_val = vch_add(db->varchar_mgr_idx, ((struct nstring *) constant)->value);
            break;
        default:
            logger(LL_ERROR, __func__, "Invalid constant type %d", constant->nodetype);
            free(constant_val);
            return NULL;
    }

    return constant_val;
}