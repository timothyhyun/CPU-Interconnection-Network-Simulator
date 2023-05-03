cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS "-O2 -g -DNDEBUG")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

add_subdirectory(branch)
add_subdirectory(branch_impl)
# add_subdirectory(branch_impl_final)
add_subdirectory(cache_impl)
add_subdirectory(engine)
add_subdirectory(trace)
add_subdirectory(processor)
add_subdirectory(processor_impl)
add_subdirectory(coherence)
add_subdirectory(interconnect)
add_subdirectory(simpleCache)

add_subdirectory(directory)
add_subdirectory(d_coherence)
add_subdirectory(cache_impl_final)
# add_subdirectory(processor_impl_final)
# add_subdirectory(coherence_impl_final)
# add_subdirectory(interconnect_impl_final)


project(cadss C)

