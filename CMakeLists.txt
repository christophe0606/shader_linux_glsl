cmake_minimum_required(VERSION 3.10)

project(hyperbolic LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_executable(hyperbolic main.cpp video.cpp hyperbolic.cpp tools.cpp)
add_executable(debug debug.cpp tools.cpp)

add_subdirectory(c_mcp c_mpc_build)

target_link_libraries(hyperbolic PUBLIC CMCP)
target_link_libraries(debug PUBLIC CMCP)

target_compile_options(hyperbolic PRIVATE
  $<$<CONFIG:RelWithDebInfo>:
    -O3
    -fvectorize         # loop vectorizer (on by default at O2+, explicit here)
    -fslp-vectorize     # SLP vectorizer (on by default at O2+, explicit here)
    -march=native       # use CPU SIMD features (ok if building on target)
  >
)

# (Optional) ThinLTO for extra speed in RelWithDebInfo
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)

find_package(glfw3 REQUIRED)

#add_subdirectory(../shader_linux/submodules/SDL sdl_bin EXCLUDE_FROM_ALL)
#target_link_libraries(hyperbolic PRIVATE SDL3::SDL3 glfw GLEW)
target_link_libraries(hyperbolic PRIVATE glfw GLEW GLESv2)

add_executable(video video_main.cpp video.cpp
)
