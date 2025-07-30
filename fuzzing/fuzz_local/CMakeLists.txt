cmake_minimum_required(VERSION 3.16)
project(zxlib_fuzz)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include the common fuzzing configuration
include(../FuzzingCommon.cmake)

# Setup fuzzing directories manually to avoid path issues
if(ENABLE_FUZZING)
    # Create directories in the correct location (current source dir is now /fuzzing/fuzz_local)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/corpora")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/logs")
    message(STATUS "Fuzzing directories setup completed for ledger-zxlib")
endif()

# Find required source files
set(ZXLIB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../src")
set(ZXLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../include")

# Find all source files needed for bech32
file(GLOB BECH32_SOURCES
    "${ZXLIB_SRC_DIR}/bech32.c"
    "${ZXLIB_SRC_DIR}/bittools.c"
    "${ZXLIB_SRC_DIR}/segwit_addr.c"
)

# Create a library with the source files
add_library(zxlib_bech32 STATIC ${BECH32_SOURCES})

target_include_directories(zxlib_bech32 PUBLIC 
    ${ZXLIB_INCLUDE_DIR}
    ${ZXLIB_SRC_DIR}
)

# Add fuzz target for bech32
add_fuzz_target(bech32 bech32_fuzzer.cpp zxlib_bech32)