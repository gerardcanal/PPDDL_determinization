cmake_minimum_required (VERSION 2.8.11)
project (PPDDL_PARSER)

find_package(BISON)
find_package(FLEX)

##BISON_TARGET(PPDDL_PARSER parser.yy ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp COMPILE_FLAGS -d)
#BISON_TARGET(PPDDL_PARSER parser.yy ${CMAKE_CURRENT_SOURCE_DIR}/parser.tab.cc COMPILE_FLAGS -d)
BISON_TARGET(PPDDL_PARSER parser.yy ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.cc COMPILE_FLAGS -d)

##FLEX_TARGET(PPDDL_SCANNER tokenizer.ll  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)
#FLEX_TARGET(PPDDL_SCANNER tokenizer.ll ${CMAKE_CURRENT_SOURCE_DIR}/lex.yy.cc)
FLEX_TARGET(PPDDL_SCANNER tokenizer.ll ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.cc)
ADD_FLEX_BISON_DEPENDENCY(PPDDL_SCANNER PPDDL_PARSER)

set(PPDDL_PARSER_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/actions.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/domains.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/effects.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/expressions.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/formulas.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/functions.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/predicates.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/problems.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/rational.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/requirements.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/terms.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/types.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/parser_main.cc
        CACHE INTERNAL "" FORCE)

set(PPDDL_PARSER_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/actions.h
        ${CMAKE_CURRENT_SOURCE_DIR}/domains.h
        ${CMAKE_CURRENT_SOURCE_DIR}/effects.h
        ${CMAKE_CURRENT_SOURCE_DIR}/expressions.h
        ${CMAKE_CURRENT_SOURCE_DIR}/formulas.h
        ${CMAKE_CURRENT_SOURCE_DIR}/functions.h
        ${CMAKE_CURRENT_SOURCE_DIR}/predicates.h
        ${CMAKE_CURRENT_SOURCE_DIR}/problems.h
        ${CMAKE_CURRENT_SOURCE_DIR}/rational.h
        ${CMAKE_CURRENT_SOURCE_DIR}/refcount.h
        ${CMAKE_CURRENT_SOURCE_DIR}/requirements.h
        ${CMAKE_CURRENT_SOURCE_DIR}/terms.h
        ${CMAKE_CURRENT_SOURCE_DIR}/types.h
        CACHE INTERNAL "" FORCE)

set(PPDDL_PARSER_ALL ${PPDDL_PARSER_SOURCES}
                     ${PPDDL_PARSER_HEADERS}
                     ${BISON_PPDDL_PARSER_OUTPUTS}
                     ${FLEX_PPDDL_SCANNER_OUTPUTS}
                     CACHE INTERNAL "" FORCE)
# CACHE to ensure visibility when included by other cmakefiles, INTERNAL is to make it not visible from cmake-gui. FORCE is to make sure it gets updated

include_directories(${CMAKE_CURRENT_SOURCE_DIR}) # redundant here
include_directories(${CMAKE_CURRENT_BINARY_DIR}) # To find generated header files from BISON

set(PPDDL_PARSER_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "" FORCE) # To be used in parent CMakeFiles

add_executable(ppddl_parser ${PPDDL_PARSER_ALL})
