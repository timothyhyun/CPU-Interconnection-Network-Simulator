project(cache_impl_final)
add_library(cache_impl_final SHARED cache_internal.c cache.c)
target_include_directories(cache_impl_final PRIVATE ../common)
