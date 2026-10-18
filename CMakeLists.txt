cmake_minimum_required(VERSION 3.24)

project(automata_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Add the CrossWindow library

include(FetchContent)

# Disable GLFW components we don't need.
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

FetchContent_MakeAvailable(glfw)


# Create the executable

add_executable(automata_engine
    main.cpp
    renderer.cpp
    StateMachine.cpp
)


# Create your executable using CrossWindow's helper
#xwin_add_executable(
 #   automata_engine
 #   engine_setup.cpp
 #   renderer.cpp
 #   StateMachine.cpp
#)


# Tell Visual Studio where to find headers
target_include_directories(automata_engine PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Link CrossWindow
target_link_libraries(automata_engine PRIVATE
    glfw
    d3d12
    dxgi
    dxguid
)

# Use C++20
target_compile_features(automata_engine PRIVATE cxx_std_20)

target_compile_definitions(automata_engine PRIVATE
    GLFW_INCLUDE_NONE
    GLFW_EXPOSE_NATIVE_WIN32
)