project(cache_impl)
add_library(cache_impl SHARED cache_internal.c cache.c)
target_include_directories(cache_impl PRIVATE ../common)
