#include "hashtable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "roundup.h"
/**
 * Initializes a new hashtable
 * @return      pointer to the new hashtable
 */
struct hashtable* ht_init(){
    struct hashtable* ht = malloc(sizeof(struct hashtable));
    if (ht == NULL) {
        return NULL;
    }
    ht->size = 0;
    ht->used = 0;
    ht->max_used = 0;
    ht->capacity = 0;
    ht->flags = NULL;
    ht->keys = NULL;
    ht->values = NULL;
    ht->hfunc = ht_str_hash;
    ht->cfunc = ht_str_eq;
    return ht;
}

/**
 * Frees the memory allocated for the hashtable.
 * @note This includes the memory for the flags, keys, values, and the hashtable itself.
 * @param ht    pointer to the hashtable to be freed
 */
void ht_free(struct hashtable* ht){
    free(ht->flags);
    free(ht->keys);
    free(ht->values);
    free(ht);
}

/**
 * Clears the hashtable by resetting its size, used, max_used, and capacity to 0.
 * @param ht    pointer to the hashtable to be cleared
 */
void ht_clear(struct hashtable* ht){
    ht->size = 0;
    ht->used = 0;
    ht->max_used = 0;
    ht->capacity = 0;
    if(ht->flags) memset(ht->flags, 0, ht->capacity);
}

int ht_reserve(struct hashtable* ht, size_t new_capacity){
    if(new_capacity <= ht->max_used){
        return 0;
    }
    size_t ht_new_capacity = new_capacity;
    roundupsize(ht_new_capacity);
    if(ht_new_capacity < new_capacity){ /* Integer overflow */
        return -1;
    }
    size_t ht_new_max_used = ( ht_new_capacity >> 1 ) + ( ht_new_capacity >> 2);
    if ( ht_new_max_used < new_capacity) {
        ht_new_capacity <<= 1;
        if (ht_new_capacity < new_capacity){ /* Integer overflow */
            return -1;
        }
        ht_new_max_used = (ht_new_capacity >> 1) + (ht_new_capacity >> 2);
    }
    char *ht_new_flags = malloc(ht_new_capacity);
    if(!ht_new_flags){
        return -1;
    }
    char** ht_new_keys = malloc(ht_new_capacity * sizeof(char*));
    if(!ht_new_keys) {
        free(ht_new_flags);
        return -1;
    }
    void** ht_new_values = malloc(ht_new_capacity * sizeof(void*));
    if(!ht_new_values) {
        free(ht_new_keys);
        free(ht_new_flags);
        return -1;
    }
    memset(ht_new_flags, 0, ht_new_capacity);
    memset(ht_new_keys, 0, ht_new_capacity * sizeof(char*));
    memset(ht_new_values, 0, ht_new_capacity * sizeof(void*));

    size_t ht_mask = ht_new_capacity - 1;
    for(size_t i = 0; i < ht->capacity; i++){
        if(ht->flags[i] != 1) continue;
        size_t index = ht->hfunc((const char*) ht->keys[i]) & ht_mask;
        size_t ht_step = 0;
        while(ht_new_flags[index]){
            index = ( index + ++ht_step ) & ht_mask;
        }
        ht_new_flags[index] = 1;
        ht_new_keys[index] = ht->keys[i];
        ht_new_values[index] = ht->values[i];
    }
    free(ht->values);
    free(ht->keys);
    free(ht->flags);
    ht->flags = ht_new_flags;
    ht->keys = ht_new_keys;
    ht->values = ht_new_values;
    ht->capacity = ht_new_capacity;
    ht->used = ht->size;
    ht->max_used = ht_new_max_used;
    return 0;
}

/**
 * Retrieves a value from the hashtable by key.
 * @param ht    pointer to the hashtable
 * @param key   key to search for in the hashtable
 * @return      value corresponding to the key, or NULL if the key is not found
 */
void* ht_get(struct hashtable* ht, char* key){
    if(!ht->size){
        return NULL;
    }
    size_t ht_mask = ht->capacity - 1;
    size_t index = ht->hfunc(key) & ht_mask;
    size_t ht_step = 0;
    while(ht->flags[index] == 2 || ht->flags[index] && !ht->cfunc(ht->keys[index], key)){
        index = (index + ++ht_step) & ht_mask;
    }
    return ht->flags[index] ? ht->values[index] : NULL;
}

/**
 * Inserts a key-value pair into the hashtable.
 * @param ht    pointer to the hashtable
 * @param key   key to be inserted into the hashtable
 * @param value value to be associated with the key
 * @return      1 if the key-value pair was inserted, 0 if the key already exists, -1 if there was an error
 */
int ht_put(struct hashtable* ht, char* key, void* value){
    size_t ht_new_size = ht->used ? ht->used + 1 : 2;
    if(ht_new_size < ht->used){ /* Integer overflow */
        return -1;
    }
    int ht_res = ht_reserve(ht, ht_new_size);
    if(ht_res == -1){
        return -1;
    }
    size_t ht_mask = ht->capacity - 1;
    size_t index = ht->hfunc(key) & ht_mask;
    size_t ht_step = 0;
    while (ht->flags[index] == 1 && !ht->cfunc(ht->keys[index], key)){
        index = (index + ++ht_step) & ht_mask;
    }
    if(ht->flags[index] == 1){
        return 0;
    } else {
        if(ht->flags[index] == 0){
            ht->used++;
        }
        ht->flags[index] = 1;
        ht->keys[index] = key;
        ht->values[index] = value;
        ht->size++;
        return 1;
    }
}

int ht_delete(struct hashtable* ht, size_t index){
    ht->flags[index] = 2;
    ht->size--;
}

bool ht_str_eq(const char* a, const char* b){
    return strcmp(a, b) == 0;
}

size_t ht_str_hash(const char* s){
    size_t h = (size_t) *s;
    if(h){
        for(++s; *s;++s){
            h = (h << 5) - h + (size_t) *s;
        }
    }
    return h;
}