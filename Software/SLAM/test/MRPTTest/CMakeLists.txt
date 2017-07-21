project(MRPTTest)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})

if(COMMAND cmake_policy)
cmake_policy(SET CMP0003 NEW) # Required by CMake 2.7+
endif(COMMAND cmake_policy)

# --------------------------------------------------------------------------
# The list of "libs" which can be included can be found in:
# http://www.mrpt.org/Libraries
#
# The dependencies of a library are automatically added, so you only
# need to specify the top-most libraries your code depends on.
# --------------------------------------------------------------------------
FIND_PACKAGE( MRPT REQUIRED base slam gui maps obs opengl) # WARNING: Add all the MRPT libs used by your program: "gui", "obs", "slam",etc.

# Declare the target (an executable)

TARGET_LINK_LIBRARIES(MRPTTest ${MRPT_LIBS})

INCLUDE_DIRECTORIES(/home/jeroen/Desktop/sweep-sdk/libsweep/include)
INCLUDE_DIRECTORIES(/home/jeroen/Desktop/sweep-sdk/libsweep/include/sweep)

target_link_libraries(MRPTTest /home/jeroen/Desktop/sweep-sdk/libsweep/libsweep.so)


# Set optimized building:
IF(CMAKE_COMPILER_IS_GNUCXX AND NOT CMAKE_BUILD_TYPE MATCHES "Debug")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++11")
ENDIF(CMAKE_COMPILER_IS_GNUCXX AND NOT CMAKE_BUILD_TYPE MATCHES "Debug")
