# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.24

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CMake.app/Contents/bin/cmake

# The command to remove a file.
RM = /Applications/CMake.app/Contents/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/tony/Documents/GitHub/cadss-f22-group3

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/tony/Documents/GitHub/cadss-f22-group3

# Include any dependencies generated for this target.
include cache/CMakeFiles/cache.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include cache/CMakeFiles/cache.dir/compiler_depend.make

# Include the progress variables for this target.
include cache/CMakeFiles/cache.dir/progress.make

# Include the compile flags for this target's objects.
include cache/CMakeFiles/cache.dir/flags.make

cache/CMakeFiles/cache.dir/cache_internal.c.o: cache/CMakeFiles/cache.dir/flags.make
cache/CMakeFiles/cache.dir/cache_internal.c.o: cache/cache_internal.c
cache/CMakeFiles/cache.dir/cache_internal.c.o: cache/CMakeFiles/cache.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/tony/Documents/GitHub/cadss-f22-group3/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object cache/CMakeFiles/cache.dir/cache_internal.c.o"
	cd /Users/tony/Documents/GitHub/cadss-f22-group3/cache && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT cache/CMakeFiles/cache.dir/cache_internal.c.o -MF CMakeFiles/cache.dir/cache_internal.c.o.d -o CMakeFiles/cache.dir/cache_internal.c.o -c /Users/tony/Documents/GitHub/cadss-f22-group3/cache/cache_internal.c

cache/CMakeFiles/cache.dir/cache_internal.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cache.dir/cache_internal.c.i"
	cd /Users/tony/Documents/GitHub/cadss-f22-group3/cache && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/tony/Documents/GitHub/cadss-f22-group3/cache/cache_internal.c > CMakeFiles/cache.dir/cache_internal.c.i

cache/CMakeFiles/cache.dir/cache_internal.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cache.dir/cache_internal.c.s"
	cd /Users/tony/Documents/GitHub/cadss-f22-group3/cache && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/tony/Documents/GitHub/cadss-f22-group3/cache/cache_internal.c -o CMakeFiles/cache.dir/cache_internal.c.s

cache/CMakeFiles/cache.dir/cache.c.o: cache/CMakeFiles/cache.dir/flags.make
cache/CMakeFiles/cache.dir/cache.c.o: cache/cache.c
cache/CMakeFiles/cache.dir/cache.c.o: cache/CMakeFiles/cache.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/tony/Documents/GitHub/cadss-f22-group3/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object cache/CMakeFiles/cache.dir/cache.c.o"
	cd /Users/tony/Documents/GitHub/cadss-f22-group3/cache && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT cache/CMakeFiles/cache.dir/cache.c.o -MF CMakeFiles/cache.dir/cache.c.o.d -o CMakeFiles/cache.dir/cache.c.o -c /Users/tony/Documents/GitHub/cadss-f22-group3/cache/cache.c

cache/CMakeFiles/cache.dir/cache.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/cache.dir/cache.c.i"
	cd /Users/tony/Documents/GitHub/cadss-f22-group3/cache && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/tony/Documents/GitHub/cadss-f22-group3/cache/cache.c > CMakeFiles/cache.dir/cache.c.i

cache/CMakeFiles/cache.dir/cache.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/cache.dir/cache.c.s"
	cd /Users/tony/Documents/GitHub/cadss-f22-group3/cache && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/tony/Documents/GitHub/cadss-f22-group3/cache/cache.c -o CMakeFiles/cache.dir/cache.c.s

# Object files for target cache
cache_OBJECTS = \
"CMakeFiles/cache.dir/cache_internal.c.o" \
"CMakeFiles/cache.dir/cache.c.o"

# External object files for target cache
cache_EXTERNAL_OBJECTS =

cache/libcache.dylib: cache/CMakeFiles/cache.dir/cache_internal.c.o
cache/libcache.dylib: cache/CMakeFiles/cache.dir/cache.c.o
cache/libcache.dylib: cache/CMakeFiles/cache.dir/build.make
cache/libcache.dylib: cache/CMakeFiles/cache.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/tony/Documents/GitHub/cadss-f22-group3/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C shared library libcache.dylib"
	cd /Users/tony/Documents/GitHub/cadss-f22-group3/cache && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/cache.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
cache/CMakeFiles/cache.dir/build: cache/libcache.dylib
.PHONY : cache/CMakeFiles/cache.dir/build

cache/CMakeFiles/cache.dir/clean:
	cd /Users/tony/Documents/GitHub/cadss-f22-group3/cache && $(CMAKE_COMMAND) -P CMakeFiles/cache.dir/cmake_clean.cmake
.PHONY : cache/CMakeFiles/cache.dir/clean

cache/CMakeFiles/cache.dir/depend:
	cd /Users/tony/Documents/GitHub/cadss-f22-group3 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/tony/Documents/GitHub/cadss-f22-group3 /Users/tony/Documents/GitHub/cadss-f22-group3/cache /Users/tony/Documents/GitHub/cadss-f22-group3 /Users/tony/Documents/GitHub/cadss-f22-group3/cache /Users/tony/Documents/GitHub/cadss-f22-group3/cache/CMakeFiles/cache.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : cache/CMakeFiles/cache.dir/depend
