cmake_minimum_required(VERSION 3.19)
project(sdcppmu_march_2021_threading)

option(SANITIZER "Enable Sanitizer" OFF)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 20)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(sdcppmu-march-2021-threading main.cpp)
if (SANITIZER)
    target_compile_options(sdcppmu-march-2021-threading PRIVATE -fsanitize=thread)
    # https://stackoverflow.com/questions/53882681/how-can-i-link-to-thread-sanitizer-with-a-newer-version-of-clang-than-the-system
    set_target_properties(sdcppmu-march-2021-threading PROPERTIES
            LINK_FLAGS -fsanitize=thread)
endif()

target_link_libraries(sdcppmu-march-2021-threading PRIVATE ${CONAN_LIBS_FMT} Threads::Threads)
