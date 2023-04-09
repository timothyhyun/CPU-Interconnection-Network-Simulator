project(interconnect_impl_final)
add_library(interconnect_impl_final SHARED interconnect.c)
target_include_directories(interconnect_impl_final PRIVATE ../common)
