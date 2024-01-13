#pragma once
#include <inttypes.h>
#include <stdint.h>

typedef struct linked_page{
    int64_t next_page;
    int64_t page_index;
    int64_t mem_start;
} linked_page_t;

typedef enum {LP_SUCCESS = 0, LP_FAIL = -1} linked_page_status_t;

int64_t lp_init_m(int64_t mem_start);
int64_t lp_init(void);
int64_t lp_useful_space_size(linked_page_t* linkedPage);
linked_page_t* lp_load(int64_t page_index);
int lp_delete(int64_t page_index);
int lp_delete_last(int64_t page_index);
int lp_write_page(linked_page_t *lp, void* src, int64_t size, int64_t src_offset);
linked_page_t* lp_load_next(linked_page_t* lp);
linked_page_t* lp_go_to(int64_t start_page_index, int64_t start_idx, int64_t stop_idx);
int lp_write(int64_t page_index, void *src, int64_t size, int64_t src_offset);
int lp_read_copy_nova(linked_page_t* lp, void* dest, int64_t size, int64_t src_offset);
int lp_read_copy_page(linked_page_t* lp, void* dest, int64_t size, int64_t src_offset);
int lp_read_copy(int64_t page_index, void* dest, int64_t size, int64_t src_offset);
