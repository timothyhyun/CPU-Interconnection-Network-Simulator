project(branch_impl)

add_library(branch_impl SHARED branch_internal.c branch.c)
target_include_directories(branch_impl PRIVATE ../common)
