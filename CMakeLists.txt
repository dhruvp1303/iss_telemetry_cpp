cmake_minimum_required(VERSION 3.14)
project(iss_telemetry_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# --- Core library (everything except main, so tests can link against it) -----
add_library(telemetry_core
  src/TelemetryParser.cpp
  src/MetricsEngine.cpp
  src/AnomalyDetector.cpp
)
target_include_directories(telemetry_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_options(telemetry_core PRIVATE -Wall -Wextra)

# --- Application -------------------------------------------------------------
add_executable(telemetry src/main.cpp)
target_link_libraries(telemetry PRIVATE telemetry_core)
target_compile_options(telemetry PRIVATE -Wall -Wextra)

# --- Tests -------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
# Don't override the parent project's compiler/linker settings on Windows.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(telemetry_tests tests/test_telemetry.cpp)
target_link_libraries(telemetry_tests PRIVATE telemetry_core GTest::gtest_main)
target_compile_options(telemetry_tests PRIVATE -Wall -Wextra)

include(GoogleTest)
gtest_discover_tests(telemetry_tests)
