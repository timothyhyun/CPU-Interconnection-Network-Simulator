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
include coherence_impl_final/CMakeFiles/coherence_impl_final.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include coherence_impl_final/CMakeFiles/coherence_impl_final.dir/compiler_depend.make

# Include the progress variables for this target.
include coherence_impl_final/CMakeFiles/coherence_impl_final.dir/progress.make

# Include the compile flags for this target's objects.
include coherence_impl_final/CMakeFiles/coherence_impl_final.dir/flags.make

coherence_impl_final/CMakeFiles/coherence_impl_final.dir/coherence.c.o: coherence_impl_final/CMakeFiles/coherence_impl_final.dir/flags.make
coherence_impl_final/CMakeFiles/coherence_impl_final.dir/coherence.c.o: coherence_impl_final/coherence.c
coherence_impl_final/CMakeFiles/coherence_impl_final.dir/coherence.c.o: coherence_impl_final/CMakeFiles/coherence_impl_final.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object coherence_impl_final/CMakeFiles/coherence_impl_final.dir/coherence.c.o"
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT coherence_impl_final/CMakeFiles/coherence_impl_final.dir/coherence.c.o -MF CMakeFiles/coherence_impl_final.dir/coherence.c.o.d -o CMakeFiles/coherence_impl_final.dir/coherence.c.o -c /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final/coherence.c

coherence_impl_final/CMakeFiles/coherence_impl_final.dir/coherence.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/coherence_impl_final.dir/coherence.c.i"
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final/coherence.c > CMakeFiles/coherence_impl_final.dir/coherence.c.i

coherence_impl_final/CMakeFiles/coherence_impl_final.dir/coherence.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/coherence_impl_final.dir/coherence.c.s"
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final/coherence.c -o CMakeFiles/coherence_impl_final.dir/coherence.c.s

coherence_impl_final/CMakeFiles/coherence_impl_final.dir/protocol.c.o: coherence_impl_final/CMakeFiles/coherence_impl_final.dir/flags.make
coherence_impl_final/CMakeFiles/coherence_impl_final.dir/protocol.c.o: coherence_impl_final/protocol.c
coherence_impl_final/CMakeFiles/coherence_impl_final.dir/protocol.c.o: coherence_impl_final/CMakeFiles/coherence_impl_final.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object coherence_impl_final/CMakeFiles/coherence_impl_final.dir/protocol.c.o"
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT coherence_impl_final/CMakeFiles/coherence_impl_final.dir/protocol.c.o -MF CMakeFiles/coherence_impl_final.dir/protocol.c.o.d -o CMakeFiles/coherence_impl_final.dir/protocol.c.o -c /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final/protocol.c

coherence_impl_final/CMakeFiles/coherence_impl_final.dir/protocol.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/coherence_impl_final.dir/protocol.c.i"
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final/protocol.c > CMakeFiles/coherence_impl_final.dir/protocol.c.i

coherence_impl_final/CMakeFiles/coherence_impl_final.dir/protocol.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/coherence_impl_final.dir/protocol.c.s"
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final/protocol.c -o CMakeFiles/coherence_impl_final.dir/protocol.c.s

coherence_impl_final/CMakeFiles/coherence_impl_final.dir/stree.c.o: coherence_impl_final/CMakeFiles/coherence_impl_final.dir/flags.make
coherence_impl_final/CMakeFiles/coherence_impl_final.dir/stree.c.o: coherence_impl_final/stree.c
coherence_impl_final/CMakeFiles/coherence_impl_final.dir/stree.c.o: coherence_impl_final/CMakeFiles/coherence_impl_final.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object coherence_impl_final/CMakeFiles/coherence_impl_final.dir/stree.c.o"
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT coherence_impl_final/CMakeFiles/coherence_impl_final.dir/stree.c.o -MF CMakeFiles/coherence_impl_final.dir/stree.c.o.d -o CMakeFiles/coherence_impl_final.dir/stree.c.o -c /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final/stree.c

coherence_impl_final/CMakeFiles/coherence_impl_final.dir/stree.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/coherence_impl_final.dir/stree.c.i"
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final/stree.c > CMakeFiles/coherence_impl_final.dir/stree.c.i

coherence_impl_final/CMakeFiles/coherence_impl_final.dir/stree.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/coherence_impl_final.dir/stree.c.s"
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final && /usr/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final/stree.c -o CMakeFiles/coherence_impl_final.dir/stree.c.s

# Object files for target coherence_impl_final
coherence_impl_final_OBJECTS = \
"CMakeFiles/coherence_impl_final.dir/coherence.c.o" \
"CMakeFiles/coherence_impl_final.dir/protocol.c.o" \
"CMakeFiles/coherence_impl_final.dir/stree.c.o"

# External object files for target coherence_impl_final
coherence_impl_final_EXTERNAL_OBJECTS =

coherence_impl_final/libcoherence_impl_final.so: coherence_impl_final/CMakeFiles/coherence_impl_final.dir/coherence.c.o
coherence_impl_final/libcoherence_impl_final.so: coherence_impl_final/CMakeFiles/coherence_impl_final.dir/protocol.c.o
coherence_impl_final/libcoherence_impl_final.so: coherence_impl_final/CMakeFiles/coherence_impl_final.dir/stree.c.o
coherence_impl_final/libcoherence_impl_final.so: coherence_impl_final/CMakeFiles/coherence_impl_final.dir/build.make
coherence_impl_final/libcoherence_impl_final.so: coherence_impl_final/CMakeFiles/coherence_impl_final.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking C shared library libcoherence_impl_final.so"
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/coherence_impl_final.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
coherence_impl_final/CMakeFiles/coherence_impl_final.dir/build: coherence_impl_final/libcoherence_impl_final.so
.PHONY : coherence_impl_final/CMakeFiles/coherence_impl_final.dir/build

coherence_impl_final/CMakeFiles/coherence_impl_final.dir/clean:
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final && $(CMAKE_COMMAND) -P CMakeFiles/coherence_impl_final.dir/cmake_clean.cmake
.PHONY : coherence_impl_final/CMakeFiles/coherence_impl_final.dir/clean

coherence_impl_final/CMakeFiles/coherence_impl_final.dir/depend:
	cd /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final /afs/andrew.cmu.edu/usr17/tonyy/private/15418/15418-project/coherence_impl_final/CMakeFiles/coherence_impl_final.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : coherence_impl_final/CMakeFiles/coherence_impl_final.dir/depend

