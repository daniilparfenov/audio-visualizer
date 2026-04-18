cmake_minimum_required(VERSION 4.0.0)
project(MusicVisualizer C)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# SDL_mixer
add_subdirectory(vendored/SDL_mixer EXCLUDE_FROM_ALL)

# Create your game executable target as usual
add_executable(MusicVisualizer
    src/main.c
    src/visualizer.c
    src/audio_utils.c
    src/window_utils.c
    src/player.c
    src/gui.c
    src/app_state.c
)

target_include_directories(MusicVisualizer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendored/Nuklear
    ${CMAKE_CURRENT_SOURCE_DIR}/vendored/Nuklear/demo/sdl3_renderer
)

# Link to the actual SDL3 library.
target_link_libraries(MusicVisualizer PRIVATE SDL3::SDL3 SDL3_mixer::SDL3_mixer)

add_subdirectory(fft)
target_link_libraries(MusicVisualizer PRIVATE fftLib)
