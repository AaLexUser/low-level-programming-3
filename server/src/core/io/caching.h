#pragma once

#include "file.h"

#if __linux__
#include <bits/types/time_t.h>
#endif

enum CH_Status {CH_SUCCESS = 0, CH_FAIL = -1, CH_DELETED = -2};
#define KB (1024u)
#define MB (1024u * KB)
#define GB (1024u * MB)

#define CH_MAX_MEMORY_USAGE  (50*PAGE_SIZE)

typedef struct caching{
    file_t file;
    size_t size, used, max_used, capacity;
    uint32_t* usage_count;
    time_t* last_used;
    void** cached_page_ptr;
    char* flags;
} caching_t;



#define ch_for_each_cached(index, ch) for ( \
int64_t index = ch_nearest_cached_index((ch)->flags, ch->capacity, ch_begin());\
(index) != ch_end(ch) && ch_cached(ch, index);                                  \
(index)++, (index) = ch_nearest_cached_index(ch->flags, ch->capacity, (index)) \
)                                \

off_t ch_file_size(caching_t* ch);
/**
* @brief   Get current max page index
* @param[in]   ch: pointer to caching_t
* @return  max page index
*/
#define ch_max_page_index(ch)  (ch->file.max_page_index)
uint64_t ch_number_pages(caching_t* ch);
uint64_t ch_page_index(off_t page_offset);
off_t ch_page_offset(uint64_t page_index);
int ch_init(const char* file_name, caching_t* ch);
size_t ch_size(caching_t* ch);
size_t ch_used(caching_t* ch);
void* ch_cached_page(caching_t* ch, size_t index);
size_t ch_usage_memory_space(caching_t* ch);
int ch_page_status(caching_t* ch, size_t index);
int ch_reserve(caching_t* ch, size_t new_capacity);
int ch_put(caching_t* ch, int64_t page_index, void* mapped_page_ptr);
void* ch_get(caching_t* ch, int64_t page_index);
int ch_remove(caching_t* ch, int64_t index);
int64_t ch_new_page(caching_t* ch);
int ch_load_page(caching_t* ch, int64_t page_index, void** page);
void ch_use_again(caching_t* ch, int64_t page_index);
int ch_write(caching_t* ch, int64_t page_index, void* src, size_t size, off_t offset);
int ch_clear_page(caching_t* ch, int64_t page_index);
int ch_copy_read(caching_t* ch, int64_t page_index, void* dest, size_t size, off_t offset);
void* ch_read(caching_t* ch, int64_t page_index, off_t offset);
uint64_t ch_begin(void);
uint64_t ch_end(caching_t* ch);
int ch_destroy(caching_t* ch);
int ch_delete(caching_t* ch);
int ch_close(caching_t* ch);
uint32_t ch_find_least_used_count(caching_t* ch);
time_t ch_find_least_used_time(caching_t* ch);
uint64_t ch_unmap_some_pages(caching_t* ch);
int ch_delete_last_page(caching_t* ch);
int ch_delete_page(caching_t* ch, int64_t page_index);
bool ch_cached(caching_t *ch, int64_t index);
int64_t ch_nearest_cached_index(const char *flags, size_t capacity, int64_t index);
int ch_print_cached_pages(caching_t* ch);
int ch_print_valid_pages(caching_t* ch);


