# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.14

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/dzhiblavi/Soft/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/dzhiblavi/Soft/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/dzhiblavi/Documents/prog/cpp/code/qtgrep

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/dzhiblavi/Documents/prog/cpp/code/qtgrep/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/test_threadpool.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/test_threadpool.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test_threadpool.dir/flags.make

CMakeFiles/test_threadpool.dir/test_threadpool.cpp.o: CMakeFiles/test_threadpool.dir/flags.make
CMakeFiles/test_threadpool.dir/test_threadpool.cpp.o: ../test_threadpool.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dzhiblavi/Documents/prog/cpp/code/qtgrep/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/test_threadpool.dir/test_threadpool.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test_threadpool.dir/test_threadpool.cpp.o -c /home/dzhiblavi/Documents/prog/cpp/code/qtgrep/test_threadpool.cpp

CMakeFiles/test_threadpool.dir/test_threadpool.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test_threadpool.dir/test_threadpool.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/dzhiblavi/Documents/prog/cpp/code/qtgrep/test_threadpool.cpp > CMakeFiles/test_threadpool.dir/test_threadpool.cpp.i

CMakeFiles/test_threadpool.dir/test_threadpool.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test_threadpool.dir/test_threadpool.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/dzhiblavi/Documents/prog/cpp/code/qtgrep/test_threadpool.cpp -o CMakeFiles/test_threadpool.dir/test_threadpool.cpp.s

# Object files for target test_threadpool
test_threadpool_OBJECTS = \
"CMakeFiles/test_threadpool.dir/test_threadpool.cpp.o"

# External object files for target test_threadpool
test_threadpool_EXTERNAL_OBJECTS =

test_threadpool: CMakeFiles/test_threadpool.dir/test_threadpool.cpp.o
test_threadpool: CMakeFiles/test_threadpool.dir/build.make
test_threadpool: CMakeFiles/test_threadpool.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/dzhiblavi/Documents/prog/cpp/code/qtgrep/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable test_threadpool"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_threadpool.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test_threadpool.dir/build: test_threadpool

.PHONY : CMakeFiles/test_threadpool.dir/build

CMakeFiles/test_threadpool.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test_threadpool.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test_threadpool.dir/clean

CMakeFiles/test_threadpool.dir/depend:
	cd /home/dzhiblavi/Documents/prog/cpp/code/qtgrep/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/dzhiblavi/Documents/prog/cpp/code/qtgrep /home/dzhiblavi/Documents/prog/cpp/code/qtgrep /home/dzhiblavi/Documents/prog/cpp/code/qtgrep/cmake-build-debug /home/dzhiblavi/Documents/prog/cpp/code/qtgrep/cmake-build-debug /home/dzhiblavi/Documents/prog/cpp/code/qtgrep/cmake-build-debug/CMakeFiles/test_threadpool.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/test_threadpool.dir/depend

