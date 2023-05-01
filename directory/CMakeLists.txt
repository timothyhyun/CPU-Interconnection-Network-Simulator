project(directory)
add_library(directory SHARED directory.c stree.c)
target_include_directories(directory PRIVATE ../common)

