#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef size_t (*hashfunc)(const char*);
typedef bool (*eq_func)(const char*, const char*);

typedef struct hashtable {
    size_t size, used, max_used, capacity;
    char* flags;
    char** keys;
    void** values;
    hashfunc hfunc;
    eq_func cfunc;
} hmap_t;

struct hashtable* ht_init();
void ht_free(struct hashtable* ht);
void ht_clear(struct hashtable* ht);
int ht_reserve(struct hashtable* ht, size_t new_capacity);
void* ht_get(struct hashtable* ht, char* key);
int ht_put(struct hashtable* ht, char* key, void* value);
int ht_delete(struct hashtable* ht, size_t index);
bool ht_str_eq(const char* a, const char* b);
size_t ht_str_hash(const char* s);