# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file

# Include any dependencies generated for this target.
include CMakeFiles/mappedfile.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/mappedfile.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/mappedfile.dir/flags.make

CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.o: CMakeFiles/mappedfile.dir/flags.make
CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.o: chunk_manager/chunk_manager.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.o   -c /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/chunk_manager/chunk_manager.c

CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/chunk_manager/chunk_manager.c > CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.i

CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/chunk_manager/chunk_manager.c -o CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.s

CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.o.requires:
.PHONY : CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.o.requires

CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.o.provides: CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.o.requires
	$(MAKE) -f CMakeFiles/mappedfile.dir/build.make CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.o.provides.build
.PHONY : CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.o.provides

CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.o.provides.build: CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.o

CMakeFiles/mappedfile.dir/hash_table/hash_table.c.o: CMakeFiles/mappedfile.dir/flags.make
CMakeFiles/mappedfile.dir/hash_table/hash_table.c.o: hash_table/hash_table.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/mappedfile.dir/hash_table/hash_table.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/mappedfile.dir/hash_table/hash_table.c.o   -c /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/hash_table/hash_table.c

CMakeFiles/mappedfile.dir/hash_table/hash_table.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/mappedfile.dir/hash_table/hash_table.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/hash_table/hash_table.c > CMakeFiles/mappedfile.dir/hash_table/hash_table.c.i

CMakeFiles/mappedfile.dir/hash_table/hash_table.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/mappedfile.dir/hash_table/hash_table.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/hash_table/hash_table.c -o CMakeFiles/mappedfile.dir/hash_table/hash_table.c.s

CMakeFiles/mappedfile.dir/hash_table/hash_table.c.o.requires:
.PHONY : CMakeFiles/mappedfile.dir/hash_table/hash_table.c.o.requires

CMakeFiles/mappedfile.dir/hash_table/hash_table.c.o.provides: CMakeFiles/mappedfile.dir/hash_table/hash_table.c.o.requires
	$(MAKE) -f CMakeFiles/mappedfile.dir/build.make CMakeFiles/mappedfile.dir/hash_table/hash_table.c.o.provides.build
.PHONY : CMakeFiles/mappedfile.dir/hash_table/hash_table.c.o.provides

CMakeFiles/mappedfile.dir/hash_table/hash_table.c.o.provides.build: CMakeFiles/mappedfile.dir/hash_table/hash_table.c.o

CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.o: CMakeFiles/mappedfile.dir/flags.make
CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.o: hash_table/hash_funcs.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/CMakeFiles $(CMAKE_PROGRESS_3)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.o   -c /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/hash_table/hash_funcs.c

CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/hash_table/hash_funcs.c > CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.i

CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/hash_table/hash_funcs.c -o CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.s

CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.o.requires:
.PHONY : CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.o.requires

CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.o.provides: CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.o.requires
	$(MAKE) -f CMakeFiles/mappedfile.dir/build.make CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.o.provides.build
.PHONY : CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.o.provides

CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.o.provides.build: CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.o

CMakeFiles/mappedfile.dir/dc_list/dc_list.c.o: CMakeFiles/mappedfile.dir/flags.make
CMakeFiles/mappedfile.dir/dc_list/dc_list.c.o: dc_list/dc_list.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/CMakeFiles $(CMAKE_PROGRESS_4)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/mappedfile.dir/dc_list/dc_list.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/mappedfile.dir/dc_list/dc_list.c.o   -c /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/dc_list/dc_list.c

CMakeFiles/mappedfile.dir/dc_list/dc_list.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/mappedfile.dir/dc_list/dc_list.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/dc_list/dc_list.c > CMakeFiles/mappedfile.dir/dc_list/dc_list.c.i

CMakeFiles/mappedfile.dir/dc_list/dc_list.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/mappedfile.dir/dc_list/dc_list.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/dc_list/dc_list.c -o CMakeFiles/mappedfile.dir/dc_list/dc_list.c.s

CMakeFiles/mappedfile.dir/dc_list/dc_list.c.o.requires:
.PHONY : CMakeFiles/mappedfile.dir/dc_list/dc_list.c.o.requires

CMakeFiles/mappedfile.dir/dc_list/dc_list.c.o.provides: CMakeFiles/mappedfile.dir/dc_list/dc_list.c.o.requires
	$(MAKE) -f CMakeFiles/mappedfile.dir/build.make CMakeFiles/mappedfile.dir/dc_list/dc_list.c.o.provides.build
.PHONY : CMakeFiles/mappedfile.dir/dc_list/dc_list.c.o.provides

CMakeFiles/mappedfile.dir/dc_list/dc_list.c.o.provides.build: CMakeFiles/mappedfile.dir/dc_list/dc_list.c.o

CMakeFiles/mappedfile.dir/logger/logger.c.o: CMakeFiles/mappedfile.dir/flags.make
CMakeFiles/mappedfile.dir/logger/logger.c.o: logger/logger.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/CMakeFiles $(CMAKE_PROGRESS_5)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/mappedfile.dir/logger/logger.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/mappedfile.dir/logger/logger.c.o   -c /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/logger/logger.c

CMakeFiles/mappedfile.dir/logger/logger.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/mappedfile.dir/logger/logger.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/logger/logger.c > CMakeFiles/mappedfile.dir/logger/logger.c.i

CMakeFiles/mappedfile.dir/logger/logger.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/mappedfile.dir/logger/logger.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/logger/logger.c -o CMakeFiles/mappedfile.dir/logger/logger.c.s

CMakeFiles/mappedfile.dir/logger/logger.c.o.requires:
.PHONY : CMakeFiles/mappedfile.dir/logger/logger.c.o.requires

CMakeFiles/mappedfile.dir/logger/logger.c.o.provides: CMakeFiles/mappedfile.dir/logger/logger.c.o.requires
	$(MAKE) -f CMakeFiles/mappedfile.dir/build.make CMakeFiles/mappedfile.dir/logger/logger.c.o.provides.build
.PHONY : CMakeFiles/mappedfile.dir/logger/logger.c.o.provides

CMakeFiles/mappedfile.dir/logger/logger.c.o.provides.build: CMakeFiles/mappedfile.dir/logger/logger.c.o

CMakeFiles/mappedfile.dir/mapped_file.c.o: CMakeFiles/mappedfile.dir/flags.make
CMakeFiles/mappedfile.dir/mapped_file.c.o: mapped_file.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/CMakeFiles $(CMAKE_PROGRESS_6)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/mappedfile.dir/mapped_file.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/mappedfile.dir/mapped_file.c.o   -c /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/mapped_file.c

CMakeFiles/mappedfile.dir/mapped_file.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/mappedfile.dir/mapped_file.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/mapped_file.c > CMakeFiles/mappedfile.dir/mapped_file.c.i

CMakeFiles/mappedfile.dir/mapped_file.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/mappedfile.dir/mapped_file.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/mapped_file.c -o CMakeFiles/mappedfile.dir/mapped_file.c.s

CMakeFiles/mappedfile.dir/mapped_file.c.o.requires:
.PHONY : CMakeFiles/mappedfile.dir/mapped_file.c.o.requires

CMakeFiles/mappedfile.dir/mapped_file.c.o.provides: CMakeFiles/mappedfile.dir/mapped_file.c.o.requires
	$(MAKE) -f CMakeFiles/mappedfile.dir/build.make CMakeFiles/mappedfile.dir/mapped_file.c.o.provides.build
.PHONY : CMakeFiles/mappedfile.dir/mapped_file.c.o.provides

CMakeFiles/mappedfile.dir/mapped_file.c.o.provides.build: CMakeFiles/mappedfile.dir/mapped_file.c.o

CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.o: CMakeFiles/mappedfile.dir/flags.make
CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.o: sorted_set/sorted_set.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/CMakeFiles $(CMAKE_PROGRESS_7)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.o"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.o   -c /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/sorted_set/sorted_set.c

CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.i"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/sorted_set/sorted_set.c > CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.i

CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.s"
	/usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/sorted_set/sorted_set.c -o CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.s

CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.o.requires:
.PHONY : CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.o.requires

CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.o.provides: CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.o.requires
	$(MAKE) -f CMakeFiles/mappedfile.dir/build.make CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.o.provides.build
.PHONY : CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.o.provides

CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.o.provides.build: CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.o

# Object files for target mappedfile
mappedfile_OBJECTS = \
"CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.o" \
"CMakeFiles/mappedfile.dir/hash_table/hash_table.c.o" \
"CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.o" \
"CMakeFiles/mappedfile.dir/dc_list/dc_list.c.o" \
"CMakeFiles/mappedfile.dir/logger/logger.c.o" \
"CMakeFiles/mappedfile.dir/mapped_file.c.o" \
"CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.o"

# External object files for target mappedfile
mappedfile_EXTERNAL_OBJECTS =

out/libmappedfile.a: CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.o
out/libmappedfile.a: CMakeFiles/mappedfile.dir/hash_table/hash_table.c.o
out/libmappedfile.a: CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.o
out/libmappedfile.a: CMakeFiles/mappedfile.dir/dc_list/dc_list.c.o
out/libmappedfile.a: CMakeFiles/mappedfile.dir/logger/logger.c.o
out/libmappedfile.a: CMakeFiles/mappedfile.dir/mapped_file.c.o
out/libmappedfile.a: CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.o
out/libmappedfile.a: CMakeFiles/mappedfile.dir/build.make
out/libmappedfile.a: CMakeFiles/mappedfile.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C static library out/libmappedfile.a"
	$(CMAKE_COMMAND) -P CMakeFiles/mappedfile.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/mappedfile.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/mappedfile.dir/build: out/libmappedfile.a
.PHONY : CMakeFiles/mappedfile.dir/build

CMakeFiles/mappedfile.dir/requires: CMakeFiles/mappedfile.dir/chunk_manager/chunk_manager.c.o.requires
CMakeFiles/mappedfile.dir/requires: CMakeFiles/mappedfile.dir/hash_table/hash_table.c.o.requires
CMakeFiles/mappedfile.dir/requires: CMakeFiles/mappedfile.dir/hash_table/hash_funcs.c.o.requires
CMakeFiles/mappedfile.dir/requires: CMakeFiles/mappedfile.dir/dc_list/dc_list.c.o.requires
CMakeFiles/mappedfile.dir/requires: CMakeFiles/mappedfile.dir/logger/logger.c.o.requires
CMakeFiles/mappedfile.dir/requires: CMakeFiles/mappedfile.dir/mapped_file.c.o.requires
CMakeFiles/mappedfile.dir/requires: CMakeFiles/mappedfile.dir/sorted_set/sorted_set.c.o.requires
.PHONY : CMakeFiles/mappedfile.dir/requires

CMakeFiles/mappedfile.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/mappedfile.dir/cmake_clean.cmake
.PHONY : CMakeFiles/mappedfile.dir/clean

CMakeFiles/mappedfile.dir/depend:
	cd /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file /home/parapapapam/Developer/BeautyCode/mapped_file/mapped_file/CMakeFiles/mappedfile.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/mappedfile.dir/depend

