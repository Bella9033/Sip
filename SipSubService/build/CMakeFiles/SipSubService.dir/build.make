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
CMAKE_SOURCE_DIR = /mnt/hgfs/share/SubServer/SipSubService/cmake

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/hgfs/share/SubServer/SipSubService/build

# Include any dependencies generated for this target.
include CMakeFiles/SipSubService.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/SipSubService.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/SipSubService.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/SipSubService.dir/flags.make

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/conf_reader.cpp.o: CMakeFiles/SipSubService.dir/flags.make
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/conf_reader.cpp.o: /mnt/hgfs/share/SubServer/SipSubService/src/conf_reader.cpp
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/conf_reader.cpp.o: CMakeFiles/SipSubService.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/share/SubServer/SipSubService/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/conf_reader.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/conf_reader.cpp.o -MF CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/conf_reader.cpp.o.d -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/conf_reader.cpp.o -c /mnt/hgfs/share/SubServer/SipSubService/src/conf_reader.cpp

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/conf_reader.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/conf_reader.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/hgfs/share/SubServer/SipSubService/src/conf_reader.cpp > CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/conf_reader.cpp.i

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/conf_reader.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/conf_reader.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/hgfs/share/SubServer/SipSubService/src/conf_reader.cpp -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/conf_reader.cpp.s

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread.cpp.o: CMakeFiles/SipSubService.dir/flags.make
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread.cpp.o: /mnt/hgfs/share/SubServer/SipSubService/src/ev_thread.cpp
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread.cpp.o: CMakeFiles/SipSubService.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/share/SubServer/SipSubService/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread.cpp.o -MF CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread.cpp.o.d -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread.cpp.o -c /mnt/hgfs/share/SubServer/SipSubService/src/ev_thread.cpp

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/hgfs/share/SubServer/SipSubService/src/ev_thread.cpp > CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread.cpp.i

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/hgfs/share/SubServer/SipSubService/src/ev_thread.cpp -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread.cpp.s

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread_pool.cpp.o: CMakeFiles/SipSubService.dir/flags.make
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread_pool.cpp.o: /mnt/hgfs/share/SubServer/SipSubService/src/ev_thread_pool.cpp
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread_pool.cpp.o: CMakeFiles/SipSubService.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/share/SubServer/SipSubService/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread_pool.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread_pool.cpp.o -MF CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread_pool.cpp.o.d -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread_pool.cpp.o -c /mnt/hgfs/share/SubServer/SipSubService/src/ev_thread_pool.cpp

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread_pool.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread_pool.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/hgfs/share/SubServer/SipSubService/src/ev_thread_pool.cpp > CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread_pool.cpp.i

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread_pool.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread_pool.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/hgfs/share/SubServer/SipSubService/src/ev_thread_pool.cpp -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread_pool.cpp.s

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/global_ctl.cpp.o: CMakeFiles/SipSubService.dir/flags.make
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/global_ctl.cpp.o: /mnt/hgfs/share/SubServer/SipSubService/src/global_ctl.cpp
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/global_ctl.cpp.o: CMakeFiles/SipSubService.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/share/SubServer/SipSubService/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/global_ctl.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/global_ctl.cpp.o -MF CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/global_ctl.cpp.o.d -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/global_ctl.cpp.o -c /mnt/hgfs/share/SubServer/SipSubService/src/global_ctl.cpp

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/global_ctl.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/global_ctl.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/hgfs/share/SubServer/SipSubService/src/global_ctl.cpp > CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/global_ctl.cpp.i

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/global_ctl.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/global_ctl.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/hgfs/share/SubServer/SipSubService/src/global_ctl.cpp -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/global_ctl.cpp.s

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/log_level.cpp.o: CMakeFiles/SipSubService.dir/flags.make
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/log_level.cpp.o: /mnt/hgfs/share/SubServer/SipSubService/src/log_level.cpp
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/log_level.cpp.o: CMakeFiles/SipSubService.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/share/SubServer/SipSubService/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/log_level.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/log_level.cpp.o -MF CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/log_level.cpp.o.d -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/log_level.cpp.o -c /mnt/hgfs/share/SubServer/SipSubService/src/log_level.cpp

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/log_level.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/log_level.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/hgfs/share/SubServer/SipSubService/src/log_level.cpp > CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/log_level.cpp.i

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/log_level.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/log_level.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/hgfs/share/SubServer/SipSubService/src/log_level.cpp -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/log_level.cpp.s

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/main.cpp.o: CMakeFiles/SipSubService.dir/flags.make
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/main.cpp.o: /mnt/hgfs/share/SubServer/SipSubService/src/main.cpp
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/main.cpp.o: CMakeFiles/SipSubService.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/share/SubServer/SipSubService/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/main.cpp.o -MF CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/main.cpp.o.d -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/main.cpp.o -c /mnt/hgfs/share/SubServer/SipSubService/src/main.cpp

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/hgfs/share/SubServer/SipSubService/src/main.cpp > CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/main.cpp.i

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/hgfs/share/SubServer/SipSubService/src/main.cpp -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/main.cpp.s

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/pjsip_utils.cpp.o: CMakeFiles/SipSubService.dir/flags.make
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/pjsip_utils.cpp.o: /mnt/hgfs/share/SubServer/SipSubService/src/pjsip_utils.cpp
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/pjsip_utils.cpp.o: CMakeFiles/SipSubService.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/share/SubServer/SipSubService/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/pjsip_utils.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/pjsip_utils.cpp.o -MF CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/pjsip_utils.cpp.o.d -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/pjsip_utils.cpp.o -c /mnt/hgfs/share/SubServer/SipSubService/src/pjsip_utils.cpp

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/pjsip_utils.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/pjsip_utils.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/hgfs/share/SubServer/SipSubService/src/pjsip_utils.cpp > CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/pjsip_utils.cpp.i

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/pjsip_utils.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/pjsip_utils.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/hgfs/share/SubServer/SipSubService/src/pjsip_utils.cpp -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/pjsip_utils.cpp.s

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_core.cpp.o: CMakeFiles/SipSubService.dir/flags.make
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_core.cpp.o: /mnt/hgfs/share/SubServer/SipSubService/src/sip_core.cpp
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_core.cpp.o: CMakeFiles/SipSubService.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/share/SubServer/SipSubService/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_core.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_core.cpp.o -MF CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_core.cpp.o.d -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_core.cpp.o -c /mnt/hgfs/share/SubServer/SipSubService/src/sip_core.cpp

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_core.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_core.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/hgfs/share/SubServer/SipSubService/src/sip_core.cpp > CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_core.cpp.i

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_core.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_core.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/hgfs/share/SubServer/SipSubService/src/sip_core.cpp -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_core.cpp.s

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_local_config.cpp.o: CMakeFiles/SipSubService.dir/flags.make
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_local_config.cpp.o: /mnt/hgfs/share/SubServer/SipSubService/src/sip_local_config.cpp
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_local_config.cpp.o: CMakeFiles/SipSubService.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/share/SubServer/SipSubService/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_local_config.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_local_config.cpp.o -MF CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_local_config.cpp.o.d -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_local_config.cpp.o -c /mnt/hgfs/share/SubServer/SipSubService/src/sip_local_config.cpp

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_local_config.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_local_config.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/hgfs/share/SubServer/SipSubService/src/sip_local_config.cpp > CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_local_config.cpp.i

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_local_config.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_local_config.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/hgfs/share/SubServer/SipSubService/src/sip_local_config.cpp -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_local_config.cpp.s

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_register.cpp.o: CMakeFiles/SipSubService.dir/flags.make
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_register.cpp.o: /mnt/hgfs/share/SubServer/SipSubService/src/sip_register.cpp
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_register.cpp.o: CMakeFiles/SipSubService.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/share/SubServer/SipSubService/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_register.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_register.cpp.o -MF CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_register.cpp.o.d -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_register.cpp.o -c /mnt/hgfs/share/SubServer/SipSubService/src/sip_register.cpp

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_register.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_register.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/hgfs/share/SubServer/SipSubService/src/sip_register.cpp > CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_register.cpp.i

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_register.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_register.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/hgfs/share/SubServer/SipSubService/src/sip_register.cpp -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_register.cpp.s

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/task_timer.cpp.o: CMakeFiles/SipSubService.dir/flags.make
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/task_timer.cpp.o: /mnt/hgfs/share/SubServer/SipSubService/src/task_timer.cpp
CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/task_timer.cpp.o: CMakeFiles/SipSubService.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/hgfs/share/SubServer/SipSubService/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building CXX object CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/task_timer.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/task_timer.cpp.o -MF CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/task_timer.cpp.o.d -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/task_timer.cpp.o -c /mnt/hgfs/share/SubServer/SipSubService/src/task_timer.cpp

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/task_timer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/task_timer.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/hgfs/share/SubServer/SipSubService/src/task_timer.cpp > CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/task_timer.cpp.i

CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/task_timer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/task_timer.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/hgfs/share/SubServer/SipSubService/src/task_timer.cpp -o CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/task_timer.cpp.s

# Object files for target SipSubService
SipSubService_OBJECTS = \
"CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/conf_reader.cpp.o" \
"CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread.cpp.o" \
"CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread_pool.cpp.o" \
"CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/global_ctl.cpp.o" \
"CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/log_level.cpp.o" \
"CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/main.cpp.o" \
"CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/pjsip_utils.cpp.o" \
"CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_core.cpp.o" \
"CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_local_config.cpp.o" \
"CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_register.cpp.o" \
"CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/task_timer.cpp.o"

# External object files for target SipSubService
SipSubService_EXTERNAL_OBJECTS =

SipSubService: CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/conf_reader.cpp.o
SipSubService: CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread.cpp.o
SipSubService: CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/ev_thread_pool.cpp.o
SipSubService: CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/global_ctl.cpp.o
SipSubService: CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/log_level.cpp.o
SipSubService: CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/main.cpp.o
SipSubService: CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/pjsip_utils.cpp.o
SipSubService: CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_core.cpp.o
SipSubService: CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_local_config.cpp.o
SipSubService: CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/sip_register.cpp.o
SipSubService: CMakeFiles/SipSubService.dir/mnt/hgfs/share/SubServer/SipSubService/src/task_timer.cpp.o
SipSubService: CMakeFiles/SipSubService.dir/build.make
SipSubService: /usr/lib/x86_64-linux-gnu/libfmt.so.8.1.1
SipSubService: CMakeFiles/SipSubService.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/hgfs/share/SubServer/SipSubService/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Linking CXX executable SipSubService"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/SipSubService.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/SipSubService.dir/build: SipSubService
.PHONY : CMakeFiles/SipSubService.dir/build

CMakeFiles/SipSubService.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/SipSubService.dir/cmake_clean.cmake
.PHONY : CMakeFiles/SipSubService.dir/clean

CMakeFiles/SipSubService.dir/depend:
	cd /mnt/hgfs/share/SubServer/SipSubService/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/hgfs/share/SubServer/SipSubService/cmake /mnt/hgfs/share/SubServer/SipSubService/cmake /mnt/hgfs/share/SubServer/SipSubService/build /mnt/hgfs/share/SubServer/SipSubService/build /mnt/hgfs/share/SubServer/SipSubService/build/CMakeFiles/SipSubService.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/SipSubService.dir/depend

