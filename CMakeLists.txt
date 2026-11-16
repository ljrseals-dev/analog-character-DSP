cmake_minimum_required(VERSION 3.16)
project(AnalogCharacterDSP VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Header-only library target
add_library(analog_character_dsp INTERFACE)
target_include_directories(analog_character_dsp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Optional: simple test executable
option(BUILD_EXAMPLES "Build example programs" ON)
if(BUILD_EXAMPLES)
    add_executable(fir_test examples/fir_test.cpp)
    target_link_libraries(fir_test PRIVATE analog_character_dsp)
endif()
