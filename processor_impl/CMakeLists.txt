project(processor_impl)

add_library(processor_impl SHARED processor.c processor_internal.c)
target_include_directories(processor_impl PRIVATE ../common)
