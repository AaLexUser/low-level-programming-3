#include "test_utils.h"
#include "utils/hashtable.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// Test case 1: Test ht_init() function
void test_ht_init() {
    struct hashtable* ht = ht_init();
    if (ht != NULL) {
        printf("ht_init() test passed.\n");
    } else {
        printf("ht_init() test failed.\n");
    }
    ht_free(ht);
}

// Test case 2: Test ht_put() and ht_get() functions
void test_ht_put_get() {
    struct hashtable* ht = ht_init();
    char* key = "key";
    int value = 10;
    int result = ht_put(ht, key, &value);
    if (result == 1) {
        int* retrieved_value = (int*)ht_get(ht, key);
        if (retrieved_value != NULL && *retrieved_value == value) {
            printf("ht_put() and ht_get() test passed.\n");
        } else {
            printf("ht_get() test failed.\n");
        }
    } else {
        printf("ht_put() test failed.\n");
    }
    ht_free(ht);
}

// Test case 3: Test ht_put() with existing key
void test_ht_put_existing_key() {
    struct hashtable* ht = ht_init();
    char* key = "key";
    int value1 = 10;
    int value2 = 20;
    ht_put(ht, key, &value1);
    int result = ht_put(ht, key, &value2);
    if (result == 0) {
        int* retrieved_value = (int*)ht_get(ht, key);
        if (retrieved_value != NULL && *retrieved_value == value1) {
            printf("ht_put() with existing key test passed.\n");
        } else {
            printf("ht_put() with existing key test failed.\n");
        }
    } else {
        printf("ht_put() with existing key test failed.\n");
    }
    ht_free(ht);
}

// Test case 4: Test ht_delete() function
void test_ht_delete() {
    struct hashtable* ht = ht_init();
    char* key = "key";
    int value = 10;
    ht_put(ht, key, &value);
    ht_delete(ht, 0);
    int* retrieved_value = (int*)ht_get(ht, key);
    if (retrieved_value == NULL) {
        printf("ht_delete() test passed.\n");
    } else {
        printf("ht_delete() test failed.\n");
    }
    ht_free(ht);
}

// Test case 5: Test ht_clear() function
void test_ht_clear() {
    struct hashtable* ht = ht_init();
    char* key = "key";
    int value = 10;
    ht_put(ht, key, &value);
    ht_clear(ht);
    int* retrieved_value = (int*)ht_get(ht, key);
    if (retrieved_value == NULL && ht->size == 0 && ht->used == 0 && ht->max_used == 0 && ht->capacity == 0) {
        printf("ht_clear() test passed.\n");
    } else {
        printf("ht_clear() test failed.\n");
    }
    ht_free(ht);
}

// Test case 6: Test ht_reserve() function
void test_ht_reserve() {
    struct hashtable* ht = ht_init();
    int value = 10;
    for (int i = 0; i < 100; i++) {
        char key[10];
        memset(key, 0, sizeof(key));
        snprintf(key, sizeof(key), "key%d", i);
        char* dupkey = strdup(key);
        ht_put(ht, dupkey, &value);
    }
    int result = ht_reserve(ht, 1000);
    if (result == 0 && ht->capacity >= 1000) {
        printf("ht_reserve() test passed.\n");
    } else {
        printf("ht_reserve() test failed.\n");
    }
    ht_free(ht);
}

// Test case 7: Test ht_str_eq() function
void test_ht_str_eq() {
    const char* str1 = "hello";
    const char* str2 = "world";
    bool result1 = ht_str_eq(str1, str1);
    bool result2 = ht_str_eq(str1, str2);
    if (result1 && !result2) {
        printf("ht_str_eq() test passed.\n");
    } else {
        printf("ht_str_eq() test failed.\n");
    }
}

// Test case 8: Test ht_str_hash() function
void test_ht_str_hash() {
    const char* str = "hello";
    size_t hash = ht_str_hash(str);
    if (hash != 0) {
        printf("ht_str_hash() test passed.\n");
    } else {
        printf("ht_str_hash() test failed.\n");
    }
}

// Run all the test cases
void run_hashtable_tests() {
    run_test_group("Hashtable Tests", (test_in_group[]){
        {"test_ht_init", test_ht_init},
        {"test_ht_put_get", test_ht_put_get},
        {"test_ht_put_existing_key", test_ht_put_existing_key},
        {"test_ht_delete", test_ht_delete},
        {"test_ht_clear", test_ht_clear},
        {"test_ht_reserve", test_ht_reserve},
        {"test_ht_str_eq", test_ht_str_eq},
        {"test_ht_str_hash", test_ht_str_hash},
    }, 8);
}

int main()
{
    run_hashtable_tests();
    return 0;
}
