#if(CMAKE_CONFIGURATION_TYPES MATCHES ASan)
#    find_program(CLANG_TIDY clang-tidy)
#    message(STATUS "Clang-tidy: ${CLANG_TIDY}")
#    if(CLANG_TIDY)
#        file(STRINGS ${PROJECT_SOURCE_DIR}/clang-tidy-checks.txt clang_tidy_checks)
#        list(JOIN clang_tidy_checks "," clang_tidy_checks_str)
#        set(CMAKE_C_CLANG_TIDY
#                ${CLANG_TIDY}
#                -header-filter=${CMAKE_SOURCE_DIR}
#                -checks=${clang_tidy_checks_str}
#                -warnings-as-errors=*
#        )
#    endif()
#endif()

# set(sources
#         core/io/file.c
#         utils/logger.c
#         core/io/caching.c
#         core/io/pager.c
#         core/page_pool/page_pool.c
#         core/io/linked_pages.c
#         backend/utils/parray.c
#         core/page_pool/linked_blocks.c
#         backend/utils/parray64.c
#         backend/table/schema.c
#         backend/table/table_base.c
#         backend/table/table.c
#         backend/journal/metatab.c
#         backend/journal/materializer.c
#         backend/journal/varchar_mgr.c
#         backend/comparator/comparator.c
#         backend/db/db.c
#         backend/network/network.c

# )

find_package(LibXml2 REQUIRED)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.c *.h)
list(FILTER sources EXCLUDE REGEX main.c)

add_library(db STATIC ${sources})
target_include_directories(db PUBLIC .)
target_link_libraries(db PUBLIC m ${LIBXML2_LIBRARIES})
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(db PRIVATE LOGGER_LEVEL=0)
else()
    target_compile_definitions(db PRIVATE LOGGER_LEVEL=2)
endif ()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/main.c)
    add_executable(db_exe main.c)
    target_link_libraries(db_exe PRIVATE db)
    set_target_properties(db_exe PROPERTIES OUTPUT_NAME db)
endif()

