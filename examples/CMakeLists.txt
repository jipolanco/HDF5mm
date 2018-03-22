add_executable(test_serial test_serial.cpp)
target_link_libraries(test_serial ${HDF5_LIBRARIES})

if(${REQUIRE_PARALLEL_HDF5})
    add_executable(test_parallel test_parallel.cpp)
    target_link_libraries(test_parallel ${HDF5_LIBRARIES} ${MPI_CXX_LIBRARIES})
endif()
