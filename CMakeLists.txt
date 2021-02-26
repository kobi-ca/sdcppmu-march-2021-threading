cmake_minimum_required(VERSION 3.19)
project(sdcppmu_march_2021_threading)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 20)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(sdcppmu-march-2021-threading main.cpp)
target_link_libraries(sdcppmu-march-2021-threading PRIVATE ${CONAN_LIBS_FMT} Threads::Threads)
