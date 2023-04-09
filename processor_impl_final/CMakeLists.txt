project(processor_impl_final)

add_library(processor_impl_final SHARED processor.c processor_internal.c)
target_include_directories(processor_impl_final PRIVATE ../common)
