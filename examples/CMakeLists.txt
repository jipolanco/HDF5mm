cmake_minimum_required(VERSION 3.1)
project(ExampleHDF5mm CXX)
enable_language(C)

# Require a compiler that supports C++11 standard.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(../include)

# Find MPI.
find_package(MPI REQUIRED)
include_directories(${MPI_C_INCLUDE_DIRS})

# Find parallel HDF5 libs.
set(HDF5_PREFER_PARALLEL true)
find_package(HDF5 1.8.8 REQUIRED C)
if(NOT ${HDF5_IS_PARALLEL})
    message(SEND_ERROR "Found HDF5 library is not parallel.")
endif()
include_directories(${HDF5_INCLUDE_DIRS})

add_executable(test_hdf5 test_hdf5.cpp)
target_link_libraries(test_hdf5 ${HDF5_LIBRARIES} ${MPI_CXX_LIBRARIES})
