# setup project
#set Name of the project


cmake_minimum_required(VERSION 3.9)
project(MemflowBase VERSION 1.0 DESCRIPTION "Memflow Base")

add_compile_options(-Wno-multichar -std=c++17 -Wall -pg -flto -Wextra -lm -Wl,--no-as-needed -ldl -lpthread -D_GLIBCXX_USE_NANOSLEEP -g3)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

# set(MEMFLOW_VERSION 0.2.0-beta5)
set(MEMFLOW_VERSION 0.2.0-beta7)

# 0.2.0-beta7

# load memflow project
include(ExternalProject)
set_directory_properties(PROPERTIES EP_PREFIX ${CMAKE_BINARY_DIR}/memflow)
ExternalProject_Add(
    memflow
    GIT_REPOSITORY "https://github.com/memflow/memflow"
    GIT_SHALLOW true
    GIT_PROGRESS true
    GIT_TAG ${MEMFLOW_VERSION}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND cargo build --all-features --workspace
    COMMAND cargo build --release --all-features --workspace
    BINARY_DIR "${CMAKE_BINARY_DIR}/memflow/src/memflow"
    INSTALL_COMMAND ""
    LOG_BUILD ON)



# https://github.com/memflow/memflow-kvm
find_package(Threads)



# create binary from /src
file(GLOB_RECURSE sources "src/*.cpp")
file(GLOB_RECURSE moreSources "src/*.c")
file(GLOB_RECURSE headers "src/*.h")



file(GLOB_RECURSE memflowHpp "memflow/*")

add_executable(
    MemflowBase
    ${sources}
    ${moreSources}
    ${headers}
)

add_dependencies(
    MemflowBase
    memflow
)

target_link_libraries(
    MemflowBase
    ${OPENGL_LIBRARIES}
    optimized "${CMAKE_BINARY_DIR}/memflow/src/memflow/target/release/libmemflow_ffi.a"
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_DL_LIBS}
    m)

target_include_directories(
    MemflowBase PUBLIC
    "${CMAKE_BINARY_DIR}/memflow/src/memflow/memflow-ffi"
)
