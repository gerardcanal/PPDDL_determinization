cmake_minimum_required (VERSION 2.8.0)
project(ppddl_determinization)

# IF ROSPLAN is not installed, use with cmake -DVAL_DIR=<path_to_VALfiles>

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
####### ADD CMAKE TO COMPILE THE PARSER ################################################################################
########################################################################################################################
include(${CMAKE_CURRENT_SOURCE_DIR}/src/PPDDLParser/CMakeLists.txt)
#set_source_files_properties(${FLEX_BISON_OUTPUTS} PROPERTIES GENERATED TRUE)


########################################################################################################################
### LOCATE VAL FILES AND SET VARIABLES #################################################################################
########################################################################################################################
if (NOT DEFINED VAL_DIR)    # Force the user to tell us which OpenCV they want (otherwise find_package can find the wrong one, cache it and changes to OpenCV_DIR are ignored)
    execute_process(COMMAND rospack find rosplan_planning_system OUTPUT_VARIABLE VAL_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(VAL_DIR ${VAL_DIR}/src/VALfiles)
endif(NOT DEFINED VAL_DIR )

IF(NOT EXISTS ${VAL_DIR})
    message(FATAL_ERROR "Failed to find VAL files in ${VAL_DIR}. Double check that \"VAL_DIR\" is set to the root build directory of VAL files.")
ELSE(NOT EXISTS ${VAL_DIR})
    message("-- Found VALfiles: ${VAL_DIR}")
endif(NOT EXISTS ${VAL_DIR})

# Get VALfiles absolute paths
set(VAL_SOURCES
        DebugWriteController.cpp
        FuncAnalysis.cpp
        SASActions.cpp
        SimpleEval.cpp
        TIM.cpp
        TimSupport.cpp
        ToFunction.cpp
        TypedAnalyser.cpp
        ValidatorAPI.cpp
        instantiation.cpp
        pddl+.cpp
        ptree.cpp
        typecheck.cpp
        Validator.cpp
        Action.cpp
        Proposition.cpp
        FuncExp.cpp
        RepairAdvice.cpp
        State.cpp
        Plan.cpp
        Ownership.cpp
        Environment.cpp
        Events.cpp
        Polynomial.cpp
        TrajectoryConstraints.cpp
        RobustAnalyse.cpp
        PrettyPrinter.cpp
        random.cpp
        LaTeXSupport.cpp
        Utils.cpp)

foreach(ITEM ${VAL_SOURCES})
get_filename_component(VAL_ABSOLUTE_SOURCE "${ITEM}" REALPATH BASE_DIR "${VAL_DIR}")
set(VAL_ABSOLUTE_SOURCES ${VAL_ABSOLUTE_SOURCES} ${VAL_ABSOLUTE_SOURCE})
endforeach()
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-conversion-null") # Disable auto_ptr warnings in VALfiles

include_directories(include src ${VAL_DIR}) # We add the src as include because the PPDDLParser headers are there

########################################################################################################################
######## CREATE LIBRARY AND EXECUTABLE #################################################################################
########################################################################################################################
add_library(ppddl_determinization SHARED
                include/PPDDLDeterminizator.h src/PPDDLDeterminizator.cpp
                src/PPDDLParserInterface.cpp include/PPDDLParserInterface.h
                src/PDDLPrinter.cpp include/PDDLPrinter.h
                src/VALConversion.cpp include/VALConversion.h
                ${VAL_ABSOLUTE_SOURCES}
                ${PPDDL_PARSER_ALL}
                src/Strategies/MLODeterminizator.cpp include/Strategies/MLODeterminizator.h
                src/Strategies/AODeterminizator.cpp include/Strategies/AODeterminizator.h
                src/Strategies/TLDeterminizator.cpp include/Strategies/TLDeterminizator.h)


add_executable(ppddl_determinizer src/ppddl_determinizer_main.cpp)
target_link_libraries(ppddl_determinizer ppddl_determinization)
