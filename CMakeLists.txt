cmake_minimum_required(VERSION 3.10.0)  # CMake version check
project(gba)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 17)            # Enable c++17 standard
if(NOT TARGET spdlog)
    find_package(spdlog REQUIRED)
endif()

# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
# SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
# SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")

# Add main.cpp file of project root directory as source file
set(SOURCE_FILES
	src/main.cpp
	src/joypad.cpp
	src/arm7tdmi/arm_opcodes.cpp
	src/arm7tdmi/cpu.cpp
	src/arm7tdmi/ir_io_registers.cpp
	src/arm7tdmi/registers.cpp
	src/arm7tdmi/thumb_opcodes.cpp
	src/debugger.cpp
	src/dma/channel.cpp
	src/dma/controller.cpp
	src/dma/dma_io_registers.cpp
	src/memory/memory.cpp
	src/memory/flash.cpp
	src/memory/io_registers.cpp
	src/platform/sfml/window.cpp
	src/ppu/ppu.cpp
	src/ppu/draw_control.cpp
	src/ppu/draw_util.cpp
	src/ppu/lcd_io_registers.cpp
	src/ppu/objects.cpp
	src/ppu/rotscale_modes.cpp
	src/ppu/text_modes.cpp
	src/timers/timer.cpp
	src/timers/timers.cpp
	src/timers/timers_io_registers.cpp
	)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(gba ${SOURCE_FILES})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
find_package(SFML 2 REQUIRED graphics window system audio)
include_directories(${SFML_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include)

target_compile_options(gba PRIVATE -O3 -Wall -Wextra -pedantic -Werror)
target_link_libraries(gba PRIVATE ${SFML_LIBRARIES} ${SFML_DEPENDENCIES} spdlog::spdlog)