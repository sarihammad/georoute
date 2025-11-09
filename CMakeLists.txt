cmake_minimum_required(VERSION 3.23)

project(georoute
    VERSION 0.1.0
    DESCRIPTION "GeoRoute real-time routing engine"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(GEOROUTE_BUILD_TESTS "Build GeoRoute unit tests" ON)
option(GEOROUTE_BUILD_BENCHMARKS "Build GeoRoute benchmarks" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(FetchContent)

# nlohmann_json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

# cpp-httplib
FetchContent_Declare(
    cpp_httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.15.3
)

# Catch2 (via helper)
include(Catch2)

FetchContent_MakeAvailable(nlohmann_json cpp_httplib Catch2)

set(GEOROUTE_SOURCES
    src/config.cpp
    src/dijkstra.cpp
    src/graph.cpp
    src/http_server.cpp
    src/lambda_handler.cpp
    src/logging.cpp
    src/router.cpp
    src/segment_tree.cpp
)

add_library(georoute_lib STATIC ${GEOROUTE_SOURCES})

target_include_directories(georoute_lib
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(georoute_lib
    PUBLIC
        nlohmann_json::nlohmann_json
        httplib::httplib
)

target_compile_features(georoute_lib PUBLIC cxx_std_20)

target_compile_options(georoute_lib
    PRIVATE
        $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall -Wextra -Wpedantic -Wconversion>
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->)

add_executable(georoute_cli src/main_cli.cpp)

target_link_libraries(georoute_cli
    PRIVATE
        georoute_lib
)

add_executable(georoute_server src/main_server.cpp)

target_link_libraries(georoute_server
    PRIVATE
        georoute_lib
        httplib::httplib
)

if(GEOROUTE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(GEOROUTE_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

install(TARGETS georoute_lib georoute_cli georoute_server
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

