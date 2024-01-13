#pragma once

#include "core/page_pool/linked_blocks.h"
#include "utils/logger.h"
#include <string.h>

#define VCH_BLOCK_SIZE 30

typedef struct vch_ticket{
    chblix_t block;
    int64_t size;
}vch_ticket_t;

int64_t vch_init(void);
vch_ticket_t vch_add(int64_t vachar_mgr_idx, char* varchar);
int vch_get(int64_t vachar_mgr_idx, vch_ticket_t* ticket, char* varchar);
int vch_delete(int64_t vachar_mgr_idx, vch_ticket_t* ticket);
