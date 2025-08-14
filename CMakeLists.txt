cmake_minimum_required(VERSION 3.10)

project(hyperbolic LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_executable(hyperbolic debug.cpp)

find_package(glfw3 REQUIRED)

#add_subdirectory(../shader_linux/submodules/SDL sdl_bin EXCLUDE_FROM_ALL)
#target_link_libraries(hyperbolic PRIVATE SDL3::SDL3 glfw GLEW)
target_link_libraries(hyperbolic PRIVATE glfw GLEW GLESv2)


