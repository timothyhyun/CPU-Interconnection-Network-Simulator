# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project

# Include any dependencies generated for this target.
include processor_impl/CMakeFiles/processor_impl.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include processor_impl/CMakeFiles/processor_impl.dir/compiler_depend.make

# Include the progress variables for this target.
include processor_impl/CMakeFiles/processor_impl.dir/progress.make

# Include the compile flags for this target's objects.
include processor_impl/CMakeFiles/processor_impl.dir/flags.make

processor_impl/CMakeFiles/processor_impl.dir/processor.c.o: processor_impl/CMakeFiles/processor_impl.dir/flags.make
processor_impl/CMakeFiles/processor_impl.dir/processor.c.o: processor_impl/processor.c
processor_impl/CMakeFiles/processor_impl.dir/processor.c.o: processor_impl/CMakeFiles/processor_impl.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object processor_impl/CMakeFiles/processor_impl.dir/processor.c.o"
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/processor_impl && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT processor_impl/CMakeFiles/processor_impl.dir/processor.c.o -MF CMakeFiles/processor_impl.dir/processor.c.o.d -o CMakeFiles/processor_impl.dir/processor.c.o -c /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/processor_impl/processor.c

processor_impl/CMakeFiles/processor_impl.dir/processor.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/processor_impl.dir/processor.c.i"
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/processor_impl && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/processor_impl/processor.c > CMakeFiles/processor_impl.dir/processor.c.i

processor_impl/CMakeFiles/processor_impl.dir/processor.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/processor_impl.dir/processor.c.s"
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/processor_impl && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/processor_impl/processor.c -o CMakeFiles/processor_impl.dir/processor.c.s

processor_impl/CMakeFiles/processor_impl.dir/processor_internal.c.o: processor_impl/CMakeFiles/processor_impl.dir/flags.make
processor_impl/CMakeFiles/processor_impl.dir/processor_internal.c.o: processor_impl/processor_internal.c
processor_impl/CMakeFiles/processor_impl.dir/processor_internal.c.o: processor_impl/CMakeFiles/processor_impl.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object processor_impl/CMakeFiles/processor_impl.dir/processor_internal.c.o"
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/processor_impl && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT processor_impl/CMakeFiles/processor_impl.dir/processor_internal.c.o -MF CMakeFiles/processor_impl.dir/processor_internal.c.o.d -o CMakeFiles/processor_impl.dir/processor_internal.c.o -c /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/processor_impl/processor_internal.c

processor_impl/CMakeFiles/processor_impl.dir/processor_internal.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/processor_impl.dir/processor_internal.c.i"
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/processor_impl && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/processor_impl/processor_internal.c > CMakeFiles/processor_impl.dir/processor_internal.c.i

processor_impl/CMakeFiles/processor_impl.dir/processor_internal.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/processor_impl.dir/processor_internal.c.s"
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/processor_impl && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/processor_impl/processor_internal.c -o CMakeFiles/processor_impl.dir/processor_internal.c.s

# Object files for target processor_impl
processor_impl_OBJECTS = \
"CMakeFiles/processor_impl.dir/processor.c.o" \
"CMakeFiles/processor_impl.dir/processor_internal.c.o"

# External object files for target processor_impl
processor_impl_EXTERNAL_OBJECTS =

processor_impl/libprocessor_impl.so: processor_impl/CMakeFiles/processor_impl.dir/processor.c.o
processor_impl/libprocessor_impl.so: processor_impl/CMakeFiles/processor_impl.dir/processor_internal.c.o
processor_impl/libprocessor_impl.so: processor_impl/CMakeFiles/processor_impl.dir/build.make
processor_impl/libprocessor_impl.so: processor_impl/CMakeFiles/processor_impl.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C shared library libprocessor_impl.so"
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/processor_impl && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/processor_impl.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
processor_impl/CMakeFiles/processor_impl.dir/build: processor_impl/libprocessor_impl.so
.PHONY : processor_impl/CMakeFiles/processor_impl.dir/build

processor_impl/CMakeFiles/processor_impl.dir/clean:
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/processor_impl && $(CMAKE_COMMAND) -P CMakeFiles/processor_impl.dir/cmake_clean.cmake
.PHONY : processor_impl/CMakeFiles/processor_impl.dir/clean

processor_impl/CMakeFiles/processor_impl.dir/depend:
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/processor_impl /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/processor_impl /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/processor_impl/CMakeFiles/processor_impl.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : processor_impl/CMakeFiles/processor_impl.dir/depend

