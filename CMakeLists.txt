project(HDF5mm CXX)
enable_language(C)
cmake_minimum_required(VERSION 3.1)

# Require a compiler that supports C++11 standard.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_EXAMPLES "Build examples" ON)
option(REQUIRE_PARALLEL_HDF5 "Require parallel HDF5" ON)
option(EXPORT_COMPILE_COMMANDS "Export compile_commands.json" ON)

if(${EXPORT_COMPILE_COMMANDS})
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

include_directories(include)

if(${REQUIRE_PARALLEL_HDF5})
    # Find MPI.
    find_package(MPI REQUIRED)
    include_directories(${MPI_C_INCLUDE_DIRS})

    # This variable is read by the FindHDF5 cmake module.
    set(HDF5_PREFER_PARALLEL true)
endif()

# Find HDF5 libs.
find_package(HDF5 1.8.8 REQUIRED C)
if(${REQUIRE_PARALLEL_HDF5} AND NOT ${HDF5_IS_PARALLEL})
    message(SEND_ERROR "Found HDF5 library is not parallel.")
endif()
include_directories(${HDF5_INCLUDE_DIRS})

if(${BUILD_EXAMPLES})
    add_subdirectory(examples)
endif()
