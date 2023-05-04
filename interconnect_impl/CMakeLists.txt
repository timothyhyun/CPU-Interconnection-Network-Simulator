project(interconnect_impl)
add_library(interconnect_impl SHARED interconnect.c interconnect_internal.c queues.c)
target_include_directories(interconnect_impl PRIVATE ../common)
