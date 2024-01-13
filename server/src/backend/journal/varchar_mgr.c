#include "varchar_mgr.h"

/**
 * @brief       Initialize the varchar manager
 * @return      index of the varchar manager index on success, TABLE_FAIL on failure
 */

int64_t vch_init(void){
    int64_t vch_vachar_mgr_idx =  lb_ppl_init(VCH_BLOCK_SIZE);
    return vch_vachar_mgr_idx;
}

/**
 * @brief       Add a varchar
 * @param[in]   vachar_mgr_idx: varchar manager index
 * @param[in]   varchar: string to add
 * @return      vch_ticket_t of varchar on success, CHBLIX_FAIL on failure
 */

vch_ticket_t vch_add(int64_t vachar_mgr_idx, char* varchar){
    vch_ticket_t ticket;
    page_pool_t* vch = lb_ppl_load(vachar_mgr_idx);
    ticket.block = lb_alloc(vch);
    ticket.size = (int64_t)strlen(varchar)+1;
    lb_write(
            vch,
            &ticket.block,
            varchar,
            ticket.size,
            0
    );
    return ticket;
}

/**
 * @brief       Get a varchar
 * @param[in]   vachar_mgr_idx: varchar manager index
 * @param[in]   ticket: ticket of varchar
 * @param[out]  varchar: string destination
 * @return      LB_SUCCESS on success, LB_FAIL on failure
 */

int vch_get(int64_t vachar_mgr_idx, vch_ticket_t* ticket, char* varchar){
    logger(LL_DEBUG, __func__, "ticket->block: %ld", ticket->block);
    return lb_read(
            vachar_mgr_idx,
            &ticket->block,
            varchar,
            ticket->size,
            0
            );
}

/**
 * @brief       Delete a varchar
 * @param[in]   vachar_mgr_idx: varchar manager index
 * @param[in]   ticket: ticket of varchar
 * @return      LB_SUCCESS on success, LB_FAIL on failure
 */

int vch_delete(int64_t vachar_mgr_idx, vch_ticket_t* ticket){
    return lb_dealloc(vachar_mgr_idx, &ticket->block);
}


