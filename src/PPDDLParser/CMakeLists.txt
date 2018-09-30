cmake_minimum_required (VERSION 2.8.11)
project(PPDDL_PARSER)

# NOTE: Here I use ${CMAKE_CURRENT_LIST_DIR} to obtain the path to this CMakeLists.txt folder, and therefore to be able
#       to include the file from a top-level CMakeLists.txt as well as to run this CMakeLists.txt by itself to compile
#       the parser.

# Compile FLEX and BISON
find_package(BISON)
find_package(FLEX)

BISON_TARGET(PPDDL_PARSER ${CMAKE_CURRENT_LIST_DIR}/parser.yy ${CMAKE_CURRENT_BINARY_DIR}/ppddl_parser.tab.cc COMPILE_FLAGS -d)

FLEX_TARGET(PPDDL_SCANNER ${CMAKE_CURRENT_LIST_DIR}/tokenizer.ll ${CMAKE_CURRENT_BINARY_DIR}/ppddl_lex.yy.cc)
ADD_FLEX_BISON_DEPENDENCY(PPDDL_SCANNER PPDDL_PARSER)

# Define variables
set(FLEX_BISON_OUTPUTS ${BISON_PPDDL_PARSER_OUTPUTS}  ${FLEX_PPDDL_SCANNER_OUTPUTS} CACHE INTERNAL "" FORCE)

set(PPDDL_PARSER_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/actions.cc
        ${CMAKE_CURRENT_LIST_DIR}/domains.cc
        ${CMAKE_CURRENT_LIST_DIR}/effects.cc
        ${CMAKE_CURRENT_LIST_DIR}/expressions.cc
        ${CMAKE_CURRENT_LIST_DIR}/formulas.cc
        ${CMAKE_CURRENT_LIST_DIR}/functions.cc
        ${CMAKE_CURRENT_LIST_DIR}/predicates.cc
        ${CMAKE_CURRENT_LIST_DIR}/problems.cc
        ${CMAKE_CURRENT_LIST_DIR}/rational.cc
        ${CMAKE_CURRENT_LIST_DIR}/requirements.cc
        ${CMAKE_CURRENT_LIST_DIR}/terms.cc
        ${CMAKE_CURRENT_LIST_DIR}/types.cc
        CACHE INTERNAL "" FORCE)

set(PPDDL_PARSER_HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/actions.h
        ${CMAKE_CURRENT_LIST_DIR}/domains.h
        ${CMAKE_CURRENT_LIST_DIR}/effects.h
        ${CMAKE_CURRENT_LIST_DIR}/expressions.h
        ${CMAKE_CURRENT_LIST_DIR}/formulas.h
        ${CMAKE_CURRENT_LIST_DIR}/functions.h
        ${CMAKE_CURRENT_LIST_DIR}/predicates.h
        ${CMAKE_CURRENT_LIST_DIR}/problems.h
        ${CMAKE_CURRENT_LIST_DIR}/rational.h
        ${CMAKE_CURRENT_LIST_DIR}/refcount.h
        ${CMAKE_CURRENT_LIST_DIR}/requirements.h
        ${CMAKE_CURRENT_LIST_DIR}/terms.h
        ${CMAKE_CURRENT_LIST_DIR}/types.h
        CACHE INTERNAL "" FORCE)

set(PPDDL_PARSER_ALL ${PPDDL_PARSER_SOURCES}
                     ${PPDDL_PARSER_HEADERS}
                     ${FLEX_BISON_OUTPUTS}
                     CACHE INTERNAL "" FORCE)
# CACHE to ensure visibility when included by other cmakefiles, INTERNAL is to make it not visible from cmake-gui. FORCE is to make sure it gets updated


include_directories(${CMAKE_CURRENT_LIST_DIR}) # redundant here
include_directories(${CMAKE_CURRENT_BINARY_DIR}) # To find generated header files from BISON

set(PPDDL_PARSER_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "" FORCE) # To be used in parent CMakeFiles

add_executable(ppddl_parser_example ${CMAKE_CURRENT_LIST_DIR}/parser_main.cc ${PPDDL_PARSER_ALL})