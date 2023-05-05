project(coherence_impl_final)
add_library(coherence_impl_final SHARED coherence.c protocol.c stree.c)
target_include_directories(coherence_impl_final PRIVATE ../common)
