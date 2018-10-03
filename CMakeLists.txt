cmake_minimum_required (VERSION 2.8.0)
#project(ppddl_determinization)

# IF ROSPLAN is not installed, use with cmake -DVAL_DIR=<path_to_VALfiles>

########################################################################################################################
####### ADD CMAKE TO COMPILE THE PARSER ################################################################################
########################################################################################################################
include(${CMAKE_CURRENT_SOURCE_DIR}/src/PPDDLParser/CMakeLists.txt)
project(ppddl_determinization) # To avoid the project inside the PPDDLParser CMakeLists take over
#set_source_files_properties(${FLEX_BISON_OUTPUTS} PROPERTIES GENERATED TRUE)

########################################################################################################################
#### CHECK FOR CXX11 SUPPORT ###########################################################################################
########################################################################################################################
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    message("-- Compiling with c++11!!!")
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    message("-- c++11 support not found! Not adding the '-std=c++11' compile flag!!")
endif()


########################################################################################################################
### LOCATE VAL FILES AND SET VARIABLES #################################################################################
########################################################################################################################
if (NOT DEFINED VAL_DIR)    # Ask the user to tell us which where is VAL  (otherwise try to get the one from ROSPlan)
    execute_process(COMMAND rospack find rosplan_dependencies OUTPUT_VARIABLE VAL_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(VAL_DIR ${VAL_DIR}/VAL)
endif(NOT DEFINED VAL_DIR )

IF(NOT EXISTS ${VAL_DIR})
    message(FATAL_ERROR "Failed to find VAL files in ${VAL_DIR}. Double check that \"VAL_DIR\" is set to the root build directory of VAL files.")
ELSE(NOT EXISTS ${VAL_DIR})
    message("-- Found VALfiles: ${VAL_DIR}")
endif(NOT EXISTS ${VAL_DIR})

# Get VALfiles absolute paths
set(VAL_SOURCES
        DebugWriteController.cpp
        pddl+.cpp
        ptree.cpp
        PrettyPrinter.cpp
        )

foreach(ITEM ${VAL_SOURCES})
get_filename_component(VAL_ABSOLUTE_SOURCE "${ITEM}" REALPATH BASE_DIR "${VAL_DIR}/src")
set(VAL_ABSOLUTE_SOURCES ${VAL_ABSOLUTE_SOURCES} ${VAL_ABSOLUTE_SOURCE})
endforeach()
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-conversion-null") # Disable auto_ptr warnings in VALfiles

include_directories(include src ${VAL_DIR}/include) # We add the src as include because the PPDDLParser headers are there

########################################################################################################################
######## CREATE LIBRARY AND EXECUTABLE #################################################################################
########################################################################################################################
add_library(ppddl_determinization SHARED
                include/PPDDLDeterminizator.h src/PPDDLDeterminizator.cpp
                src/PPDDLParserInterface.cpp include/PPDDLParserInterface.h
                src/PDDLPrinter.cpp include/PDDLPrinter.h
                src/VALConversion.cpp include/VALConversion.h
                ${PPDDL_PARSER_ALL}
                src/Strategies/MLODeterminizator.cpp include/Strategies/MLODeterminizator.h
                src/Strategies/AODeterminizator.cpp include/Strategies/AODeterminizator.h
                src/Strategies/TLDeterminizator.cpp include/Strategies/TLDeterminizator.h
                src/PPDDLDeterminizatorFactory.cpp include/PPDDLDeterminizatorFactory.h)
target_sources(ppddl_determinization INTERFACE ${VAL_ABSOLUTE_SOURCES})


add_executable(ppddl_determinizer src/ppddl_determinizer_main.cpp)
target_link_libraries(ppddl_determinizer ppddl_determinization)
