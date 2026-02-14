cmake_minimum_required(VERSION 3.16)
project(MusicVisualizer C)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# Create your game executable target as usual
add_executable(MusicVisualizer src/main.c src/visualizer.c src/audio_utils.c)

target_include_directories(MusicVisualizer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link to the actual SDL3 library.
target_link_libraries(MusicVisualizer PRIVATE SDL3::SDL3)
