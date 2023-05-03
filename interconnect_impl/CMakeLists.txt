project(interconnect_impl)
add_library(interconnect_impl SHARED interconnect.c interconnect_internal.c)
target_include_directories(interconnect_imple PRIVATE ../common)
