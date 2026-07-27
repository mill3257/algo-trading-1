cmake_minimum_required(VERSION 3.20)
project(dsp_trader VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g -fsanitize=address,undefined")

add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)

# ── Optional dependencies (enable as you reach each phase) ────────────────────
find_package(PkgConfig REQUIRED)

# Phase 2: FFTW3 for sliding DFT
# pkg_check_modules(FFTW3 REQUIRED fftw3)

# Phase 4: ZeroMQ for C++/Python IPC
# pkg_check_modules(ZMQ REQUIRED libzmq)

find_package(Threads REQUIRED)

# ── Subdirectories ─────────────────────────────────────────────────────────────
include_directories(include)
add_subdirectory(src)
enable_testing()
add_subdirectory(tests)

# ── Main executable ────────────────────────────────────────────────────────────
add_executable(dsp_trader src/main.cpp)
target_link_libraries(dsp_trader
    PRIVATE
    core_lib
    dsp_lib
    alpha_lib
    risk_lib
    execution_lib
    Threads::Threads
)
