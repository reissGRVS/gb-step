cmake_minimum_required(VERSION 3.5.1)  # CMake version check
project(gba)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 14)            # Enable c++14 standard
if(NOT TARGET spdlog)
    find_package(spdlog REQUIRED)
endif()

# Add main.cpp file of project root directory as source file
set(SOURCE_FILES
	src/main.cpp
	src/memory.cpp	
	src/arm7tdmi/arm_opcodes.cpp
	src/arm7tdmi/registers.cpp
	src/arm7tdmi/cpu.cpp
	)



# Add executable target with source files listed in SOURCE_FILES variable
add_executable(gba ${SOURCE_FILES})
target_compile_options(gba PRIVATE -Wall -Wextra -pedantic -Werror)
target_link_libraries(gba PRIVATE spdlog::spdlog)