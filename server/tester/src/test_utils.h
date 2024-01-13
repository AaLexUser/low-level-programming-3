#pragma once

#define _GNU_SOURCE

#include <stdio.h>
#if defined(__unix__)|| defined(__linux__)
#include <sys/mman.h>
#endif
#if  defined(__WIN32__)
#include <windows.h>
#endif
#include <assert.h>

#define DEFINE_TEST(_name) static void test_##_name()
#define DEFINE_TEST_GROUP(_name) static const test_in_group tests_##_name[] =
#define RUN_SINGLE_TEST(_name) do { \
    puts(" Run test \"" #_name "\"..."); \
    test_##_name(); \
} while (0)

#define RUN_TEST_GROUP(_name) run_test_group( \
    #_name, \
    tests_##_name, \
    sizeof(tests_##_name) / sizeof(*tests_##_name) \
)

#define TEST(_name) test_##_name

#define TEST_IN_GROUP(_name) { .name = #_name, .test = test_##_name, }

typedef void (*test_function)();
typedef struct {
    const char * name;
    test_function test;
} test_in_group;

inline void run_test_group(const char * name, const test_in_group * tests, size_t amount) {
    printf(" Run test group \"%s\":\n", name);

    for (size_t i = 0; i < amount; ++i) {
        printf("  %zu. Run test \"%s\"...\n", i + 1, tests[i].name);
        tests[i].test();
    }
}
