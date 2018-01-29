cmake_minimum_required (VERSION 2.8.11)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    message("-- Compiling with c++11!!!")
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    message("-- c++11 feature not found! Not adding the '-std=c++11' compile flag!!")
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/PPDDLParser/CMakeLists.txt)
#set_source_files_properties(${FLEX_BISON_OUTPUTS} PROPERTIES GENERATED TRUE)

# Get VALFiles absolute paths
foreach(ITEM ${VAL_SOURCES})
    get_filename_component(VAL_ABSOLUTE_SOURCE "${ITEM}" REALPATH BASE_DIR "${CMAKE_SOURCE_DIR}")
    SET(VAL_ABSOLUTE_SOURCES ${VAL_ABSOLUTE_SOURCES} ${VAL_ABSOLUTE_SOURCE})
endforeach()


include_directories(${PPDDL_PARSER_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src/PPDDLDeterminization)
add_executable(parser2 ${PPDDL_PARSER_ALL}
        MLODeterminizator.cpp
        MLODeterminizator.h
        PPDDLDeterminizator.h PPDDLParserInterface.cpp PPDDLParserInterface.h
        ${VAL_ABSOLUTE_SOURCES} VALConversion.cpp VALConversion.h)
