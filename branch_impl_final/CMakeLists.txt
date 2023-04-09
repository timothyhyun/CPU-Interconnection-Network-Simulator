project(branch_impl_final)

add_library(branch_impl_final SHARED branch_internal.c branch.c)
target_include_directories(branch_impl_final PRIVATE ../common)
