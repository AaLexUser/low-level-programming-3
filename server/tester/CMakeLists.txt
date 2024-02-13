file(GLOB_RECURSE common_sources CONFIGURE_DEPENDS
        src/*.c
        src/*.h
)

add_library(test_stubs OBJECT ${common_sources})
target_include_directories(test_stubs PUBLIC src)
target_link_libraries(test_stubs PUBLIC db m)

file(GLOB_RECURSE test_sources CONFIGURE_DEPENDS
        tests/file_manager.c
        tests/caching.c
        tests/pstack.c
        tests/pager.c
        tests/page_pool.c
        tests/linked_pages.c
        tests/parray.c
        tests/linked_block.c
        tests/schema.c
        tests/table.c
        tests/hashtable.c
        tests/test_hashtable.c
)

foreach(test_source IN LISTS test_sources)
    string(REPLACE "/" ";" name_components ${test_source})
    list(GET name_components -1 name)
    string(REGEX REPLACE "\.c$" "" name ${name})
    list(APPEND test_targets test_${name})

    add_executable(test_${name} ${test_source})
    target_link_libraries(test_${name} PRIVATE test_stubs)

    add_test(NAME test_${name} COMMAND test_${name})
endforeach()

set(CMAKE_CTEST_ARGUMENTS --output-junit ${PROJECT_SOURCE_DIR}/report.xml --output-on-failure -C $<CONFIG>)
add_custom_target(check
        COMMAND ${CMAKE_CTEST_COMMAND} ${CMAKE_CTEST_ARGUMENTS}
        DEPENDS ${test_targets})
