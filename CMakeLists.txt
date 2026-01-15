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
option(GEOROUTE_FETCH_DEPS "Fetch third-party dependencies with FetchContent (requires network)" OFF)
option(GEOROUTE_REQUIRE_SYSTEM_DEPS "Fail configure if system deps are missing" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#
# Third-party deps
# Prefer system packages for reproducible/offline builds.
# If you want CMake to fetch deps, enable: -DGEOROUTE_FETCH_DEPS=ON (requires network).
#

set(GEOROUTE_HAVE_NLOHMANN_JSON OFF)
set(GEOROUTE_HAVE_HTTPLIB OFF)

# Try system packages first
find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
    set(GEOROUTE_HAVE_NLOHMANN_JSON ON)
endif()

find_package(httplib QUIET)
if(httplib_FOUND)
    set(GEOROUTE_HAVE_HTTPLIB ON)
endif()

# Vendored header-only fallback
if(NOT GEOROUTE_HAVE_NLOHMANN_JSON AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann/json.hpp")
    add_library(nlohmann_json_vendor INTERFACE)
    target_include_directories(nlohmann_json_vendor INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
    add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json_vendor)
    set(GEOROUTE_HAVE_NLOHMANN_JSON ON)
    message(STATUS "Using vendored nlohmann/json")
endif()

if(NOT GEOROUTE_HAVE_HTTPLIB AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/httplib/httplib.h")
    add_library(httplib_vendor INTERFACE)
    target_include_directories(httplib_vendor INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
    add_library(httplib::httplib ALIAS httplib_vendor)
    set(GEOROUTE_HAVE_HTTPLIB ON)
    message(STATUS "Using vendored cpp-httplib")
endif()

# Optional FetchContent (requires network)
if(GEOROUTE_FETCH_DEPS)
    include(FetchContent)
    if(NOT GEOROUTE_HAVE_NLOHMANN_JSON)
        FetchContent_Declare(
            nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG v3.11.3
        )
    endif()
    if(NOT GEOROUTE_HAVE_HTTPLIB)
        FetchContent_Declare(
            cpp_httplib
            GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
            GIT_TAG v0.15.3
        )
    endif()
    # Catch2 (via helper)
    include(Catch2)
    FetchContent_MakeAvailable(nlohmann_json cpp_httplib Catch2)
    if(nlohmann_json_FOUND)
        set(GEOROUTE_HAVE_NLOHMANN_JSON ON)
    endif()
    if(httplib_FOUND)
        set(GEOROUTE_HAVE_HTTPLIB ON)
    endif()
endif()

# Validate dependencies
if(GEOROUTE_REQUIRE_SYSTEM_DEPS)
    if(NOT GEOROUTE_HAVE_NLOHMANN_JSON)
        message(FATAL_ERROR
            "nlohmann_json not found. Options:\n"
            "  1. Install system package (e.g., apt-get install nlohmann-json3-dev)\n"
            "  2. Use vendored header: ensure third_party/nlohmann/json.hpp exists\n"
            "  3. Enable fetch: -DGEOROUTE_FETCH_DEPS=ON (requires network)")
    endif()
    if(NOT GEOROUTE_HAVE_HTTPLIB)
        message(FATAL_ERROR
            "cpp-httplib not found. Options:\n"
            "  1. Install system package\n"
            "  2. Use vendored header: ensure third_party/httplib/httplib.h exists\n"
            "  3. Enable fetch: -DGEOROUTE_FETCH_DEPS=ON (requires network)")
    endif()
endif()

set(GEOROUTE_SOURCES
    src/config.cpp
    src/dijkstra.cpp
    src/engine.cpp
    src/graph.cpp
    src/http_server.cpp
    src/lambda_handler.cpp
    src/logging.cpp
    src/router.cpp
    src/segment_tree.cpp
    src/app.cpp
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

add_executable(georoute_server apps/server/main.cpp)

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

