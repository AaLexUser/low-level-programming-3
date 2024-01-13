cmake_minimum_required(VERSION 3.10)
project(lab3-client C)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -DYYDEBUG=1")

find_package(LibXml2 REQUIRED)
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

# Generate the parser and lexer files
BISON_TARGET(MyParser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c)
FLEX_TARGET(MyLexer lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.c)
ADD_FLEX_BISON_DEPENDENCY(MyLexer MyParser)

# Include the binary directory to include lexer.c in parser.c
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Define the source files
set(SOURCE_FILES
    main.c
    ${BISON_MyParser_OUTPUTS}
    ${FLEX_MyLexer_OUTPUTS}
    ast.c
    xml.c
    ast.h
    xml.h
    network.h
    network.c
)


# Create the executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Link the libraries
target_link_libraries(${PROJECT_NAME} ${LIBXML2_LIBRARIES})


