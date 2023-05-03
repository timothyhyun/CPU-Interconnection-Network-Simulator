project(d_coherence)
add_library(d_coherence SHARED dcoherence.c protocol.c stree.c)
target_include_directories(d_coherence PRIVATE ../common)
